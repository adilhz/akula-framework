/***************************************************************************
 *   Copyright (C) 2006 by Krasimir Marinov   *
 *   krasimir.vanev@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef OS_EVENT_DEMULTIPLEXER_POLL_H
#define OS_EVENT_DEMULTIPLEXER_POLL_H

#include "reactor_utils.h"
#include <cstring>
#include <sys/poll.h>
#include <unistd.h> /*fdatasync(), read()*/


namespace reactor
{
    class OsEventDemultiplexerPollImpl
    {
    public:
        OsEventDemultiplexerPollImpl()
        {
            m_pPollDataTable = new struct pollfd [CReactorUtils::maxOpenFilesPerProcess()];
            m_pIndexTable = new int [CReactorUtils::maxOpenFilesPerProcess()];
            for(int i = 0; i < CReactorUtils::maxOpenFilesPerProcess(); i++)
            {
                m_pPollDataTable[i].fd = m_pIndexTable[i] = -1;
            }
     
            m_iNumberOfPolledFds = 0;
     
            ::pipe(m_pipe);
            add_fd(m_pipe[0], CReactorUtils::READ_EVENT);// register the pipe to be able to read the commands
     
            m_bStop = false;
        }
     
        ~OsEventDemultiplexerPollImpl()
        {
            delete [] m_pPollDataTable;
            delete [] m_pIndexTable;
        }
     
        // registering sockets in the correct fd_set
        void add_fd(int iSocketFD, unsigned long ulFlag)
        {
            int iIndex = m_pIndexTable[iSocketFD];
         
            if(iIndex == -1) //not being added for event currently
            {
                int iCurrPollDataTableIndex = iIndex = m_iNumberOfPolledFds++;
         
                /* ***IMPORTANT***
                 * Reset the events when adding, they are not automatically resetted!
                 * If this fd has been used and after removing its events are not cleared subsequent adding to poll
                 * will result with the non-cleared events available - this is shit when accepting connection and want the
                 * fd to be registered for READ only and WRITE event suddenly appears! :-)
                 */
                 ::memset(&m_pPollDataTable[iCurrPollDataTableIndex], 0, sizeof(struct pollfd));
         
                m_pPollDataTable[iCurrPollDataTableIndex].fd = iSocketFD;
                 
                m_pIndexTable[iSocketFD] = iCurrPollDataTableIndex;
            }
         
            if(ulFlag & CReactorUtils::READ_EVENT)
            {
                m_pPollDataTable[iIndex].events |= POLLRDNORM;
            }
         
            if(ulFlag & CReactorUtils::WRITE_EVENT)
            {
                m_pPollDataTable[iIndex].events |= POLLWRNORM;
            }
         
            trigger_internal_command('a');
        }
     
         // "unregistering" from fd_sets
        void remove_fd(int iSocketFD, unsigned long ulFlag)
        {
            int iIndex = m_pIndexTable[iSocketFD];
        
            if(ulFlag & CReactorUtils::READ_EVENT)
            {
                m_pPollDataTable[iIndex].events &= (~POLLRDNORM);
            }
            
            if(ulFlag & CReactorUtils::WRITE_EVENT)
            {
                m_pPollDataTable[iIndex].events &= (~POLLWRNORM);
            }
        
            if(m_pPollDataTable[iIndex].events == 0)
            {
                int iLastPollDataTableIndex = --m_iNumberOfPolledFds;
        
                ::memset(&m_pPollDataTable[iIndex], 0, sizeof(struct pollfd));
                m_pPollDataTable[iIndex].fd = -1;
                m_pIndexTable[iSocketFD] = -1;
                if(iIndex != iLastPollDataTableIndex)
                {
                    ::memcpy(&m_pPollDataTable[iIndex], &m_pPollDataTable[iLastPollDataTableIndex], sizeof(struct pollfd));
                    //lets keep it clean
                    ::memset(&m_pPollDataTable[iLastPollDataTableIndex], 0, sizeof(struct pollfd));
                    m_pPollDataTable[iLastPollDataTableIndex].fd = -1;
                    int fd = m_pPollDataTable[iIndex].fd;
         
                    //TODO: Try to optimize this search, maybe use list of deactivated descriptors and iterate only them
                    if(fd == -1) //deactivated - find the fd using the index
                    {
                        for(int i = 0; i < CReactorUtils::maxOpenFilesPerProcess(); i++)
                        {
                            if(iLastPollDataTableIndex == m_pIndexTable[i])
                            {
                                fd = i;
                                break;
                            }
                        }
                    }
         
                    m_pIndexTable[fd] = iIndex;
                }
            }
         
            trigger_internal_command('r');
        }
     
        int watch_fds()
        {
            int iCountOfReadyFDs = 0;
        
            do
            {
                if(isStopped())
                    return 0;
                
                iCountOfReadyFDs = ::poll(m_pPollDataTable, m_iNumberOfPolledFds, -1);
            } while(check_fd(m_pipe[0], CReactorUtils::READ_EVENT) && process_internal_command());
        
            return iCountOfReadyFDs;
        }
     
        // this function is used for determining if given FD is "ready" for the event, marked with ulFlag
        bool check_fd(int iSocketFD, unsigned long ulFlag)
        {
            bool bIsReady = false;
            int iIndex = m_pIndexTable[iSocketFD];
         
            if(m_pPollDataTable[iIndex].fd == -1)
            {
                //ERROR???!!!
                m_pPollDataTable[iIndex].revents = 0;
                return false;
            }
         
            if((ulFlag & CReactorUtils::READ_EVENT) &&
                (m_pPollDataTable[iIndex].revents & (POLLRDNORM | POLLHUP | POLLNVAL | POLLERR)))
            {
                bIsReady = true;
            }
         
            if((ulFlag & CReactorUtils::WRITE_EVENT) &&
                (m_pPollDataTable[iIndex].revents & (POLLWRNORM | POLLHUP | POLLNVAL | POLLERR)))
            {
                bIsReady = true;
            }
         
            return bIsReady;
        }
     
        /**Temporary deactivating*/
        void deactivate_fd(int iFd)
        {
            int iIndex = m_pIndexTable[iFd];
            if(iIndex != -1)
            {
                m_pPollDataTable[iIndex].fd = -1; //do not monitor this file descriptor
                m_pPollDataTable[iIndex].revents = 0;
                trigger_internal_command('r');
            }
        }
     
        /** Return this file descriptor from deactivation*/
        void reactivate_fd(int iFd)
        {
            int iIndex = m_pIndexTable[iFd];
            if(iIndex != -1)
            {
                m_pPollDataTable[iIndex].fd = iFd;
                trigger_internal_command('a');
            }
        }

        void stop(void)
        {
            trigger_internal_command('e');
        }

        bool isStopped(void) const
        {
            return m_bStop;
        }

    private:
        void trigger_internal_command(char command)
        {
            // send command to the reactor (writing to the pipe)
            ::write(m_pipe[1], &command, 1);
            ::fdatasync(m_pipe[1]);
        }
         
        bool process_internal_command()
        {
            char command;
            ::read(m_pipe[0], &command, 1);
            
            // do something
            switch(command)
            {
                case 'a': // event added
                    return true;
                break;
                case 'r': // event removed
                    return true;
                break;
                case 'e': // dispatch loop exit
                    return m_bStop = true;
                break;
                default:
                    return false;
                break;
            }
        }

    private:
        int m_pipe[2];
        struct pollfd* m_pPollDataTable;
        int* m_pIndexTable;

        //max used index into struct pollfd array
        int m_iNumberOfPolledFds;

        bool m_bStop;
    };
}//namespace reactor

#endif /*OS_EVENT_DEMULTIPLEXER_POLL_H*/

