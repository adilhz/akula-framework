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
#include "os_event_demultiplexer_poll.h"
#include "reactor_utils.h"
#include <akula/dbg/dbg.h>
#include <cstring>
#include <sys/poll.h>

reactor::OsEventDemultiplexerPollImpl::OsEventDemultiplexerPollImpl()
{
    dbg::info() << "Initializing reactor with " << CReactorUtils::MAX_OPEN_FILES_PER_PROCESS << "slots\n";

    m_pPollDataTable = new struct pollfd [CReactorUtils::MAX_OPEN_FILES_PER_PROCESS];
    m_pIndexTable = new int [CReactorUtils::MAX_OPEN_FILES_PER_PROCESS];
    for(int i = 0; i < CReactorUtils::MAX_OPEN_FILES_PER_PROCESS; i++)
    {
        m_pPollDataTable[i].fd = m_pIndexTable[i] = -1;
    }

    m_iNumberOfPolledFds = 0;

    ::pipe(m_pipe);
    add_fd(m_pipe[0], CReactorUtils::READ_MASK);// register the pipe to be able to read the commands
}

reactor::OsEventDemultiplexerPollImpl::~OsEventDemultiplexerPollImpl()
{
    delete [] m_pPollDataTable;
    delete [] m_pIndexTable;
}

// registering sockets in the correct fd_set
void
reactor::OsEventDemultiplexerPollImpl::add_fd(int iSocketFD, unsigned long ulFlag)
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

    if(ulFlag & CReactorUtils::READ_MASK)
    {
        m_pPollDataTable[iIndex].events |= POLLRDNORM;
    }

    if(ulFlag & CReactorUtils::WRITE_MASK)
    {
        m_pPollDataTable[iIndex].events |= POLLWRNORM;
    }

trigger_internal_command('a');
}

// "unregistering" from fd_sets
void
reactor::OsEventDemultiplexerPollImpl::remove_fd(int iSocketFD, unsigned long ulFlag)
{
    int iIndex = m_pIndexTable[iSocketFD];

    if(ulFlag & CReactorUtils::READ_MASK)
    {
        m_pPollDataTable[iIndex].events &= (~POLLRDNORM);
    }
    
    if(ulFlag & CReactorUtils::WRITE_MASK)
    {
        m_pPollDataTable[iIndex].events &= (~POLLWRNORM);
    }

    // Removing both events means the connection is closed (removed)
    // Removing when events is 0 is not good since it is possible to be temp. deactivating
    if(ulFlag & CReactorUtils::REMOVE_MASK)
    {
        int iLastPollDataTableIndex = --m_iNumberOfPolledFds;
        //<TEST
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
                for(int i = 0; i < CReactorUtils::MAX_OPEN_FILES_PER_PROCESS; i++)
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
        //TEST/>
#if 0
        ::memset(&m_pPollDataTable[iIndex], 0, sizeof(struct pollfd));
        ::memcpy(&m_pPollDataTable[iIndex], &m_pPollDataTable[iLastPollDataTableIndex], sizeof(struct pollfd));
        ::memset(&m_pPollDataTable[iLastPollDataTableIndex], 0, sizeof(struct pollfd));
        m_pPollDataTable[iLastPollDataTableIndex].fd = -1;

        int fd = m_pPollDataTable[iIndex].fd;

        //TODO: Try to optimize this search, maybe use list of deactivated descriptors and iterate only them
        if(fd == -1) //deactivated - find the fd using the index
        {
            for(int i = 0; i < ReactorUtils::MAX_OPEN_FILES_PER_PROCESS; i++)
            {
                if(iLastPollDataTableIndex == m_pIndexTable[i])
                {
                    fd = i;
                    MSG(AIM_ERROR, "FOUND fd = %d", fd)
                    break;
                }
            }
        }

        m_pIndexTable[fd] = iIndex;
        m_pIndexTable[iSocketFD] = -1;
#endif
    }

trigger_internal_command('r');
}

int
reactor::OsEventDemultiplexerPollImpl::watch_fds()
{
    //ONLY ONE THREAD AT A TIME SHOULD WAIT ON SELECT() - LEADERS/FOLLOWERS STRATEGY BY Douglas Schmidt (see lf.pdf)!!!

    int iCountOfReadyFDs = 0;

    do
    {
        iCountOfReadyFDs = ::poll(m_pPollDataTable, m_iNumberOfPolledFds, -1);
    } while(check_fd(m_pipe[0], CReactorUtils::READ_MASK) && process_internal_command());

    return iCountOfReadyFDs;
}

// this function is used for determining if given FD is "ready" for the event, marked with ulFlag
bool
reactor::OsEventDemultiplexerPollImpl::check_fd(int iSocketFD, unsigned long ulFlag)
{
    bool bIsReady = false;
    int iIndex = m_pIndexTable[iSocketFD];

    if(m_pPollDataTable[iIndex].fd == -1)
    {
        //ERROR???!!!
        m_pPollDataTable[iIndex].revents = 0;
        return false;
    }

    if((ulFlag & CReactorUtils::READ_MASK) &&
        (m_pPollDataTable[iIndex].revents & (POLLRDNORM | POLLHUP | POLLNVAL | POLLERR)))
    {
        bIsReady = true;
    }

    if((ulFlag & CReactorUtils::WRITE_MASK) &&
        (m_pPollDataTable[iIndex].revents & (POLLWRNORM | POLLHUP | POLLNVAL | POLLERR)))
    {
        bIsReady = true;
    }

    return bIsReady;
}

/**Temporary deactivating*/
void
reactor::OsEventDemultiplexerPollImpl::deactivate_fd(int iFd)
{
    int iIndex = m_pIndexTable[iFd];
    if(iIndex != -1)
    {
        m_pPollDataTable[iIndex].fd = -1; //do not monitor this file descriptor
        m_pPollDataTable[iIndex].revents = 0;
        trigger_internal_command('r');
    }
    else
    {
        dbg::warning() << "Trying to deactivate socket " << iFd << ". Not found.\n";
    }
}

/** Return this file descriptor from deactivation*/
void
reactor::OsEventDemultiplexerPollImpl::reactivate_fd(int iFd)
{
    int iIndex = m_pIndexTable[iFd];
    if(iIndex != -1)
    {
        m_pPollDataTable[iIndex].fd = iFd;
        trigger_internal_command('a');
    }
    else
    {
        dbg::warning() << "Trying to reactivate socket " << iFd << ". Not found.\n";
    }
}
