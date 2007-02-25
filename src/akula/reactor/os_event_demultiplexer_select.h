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

#ifndef OS_EVENT_DEMULTIPLEXER_SELECT_H
#define OS_EVENT_DEMULTIPLEXER_SELECT_H

#include "reactor_utils.h"
#include <sys/select.h>
#include <exception>
#include <string>

/*!!! Not working. To be implemented!!!*/

namespace reactor
{
    class OsEventDemultiplexerSelectImpl
    {
    private:
        void trigger_internal_command(char command)
        {
            // send command to the reactor (writing to the pipe)
            ::write(m_pipe[1], &command, 1);
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
                    //return m_bStopRequest = true;
                break;
                default:
                    return false;
                break;
            }
        }

        // finding the Max value for registered FDs, this value is needed for select() invokation
        int get_max_fd(void)
        {
        //TODO: keep member with the max value fd!
        // if(new_fd > last_max_fd) {last_max_fd = new_fd};
        
        /*
            //it is not needed to iterate through the set for finding the max value, 'cos "...since set and multiset
            //are Sorted Associative Containers, their elements are always sorted in ascending order..."
            //so it is enough to take the value for the last element in the set :-)
            //
            EventHandlersTableIterator_t IT = m_EventHandlersTable.end();
            
            return (std::max(m_pipefd[0], (--IT)->first));//(--IT), 'cos IT=m_CallbacksMap.end() points to one after the last element
        */
        }

    public:
        OsEventDemultiplexerSelectImpl()
        {
            // initialisation of the sets used by select()
            FD_ZERO(&m_master_rfdset);
            FD_ZERO(&m_master_wfdset);
        
            ::pipe(m_pipe);
            add_fd(m_pipe[0], CReactorUtils::READ_EVENT);// register the pipe to be able to read the commands
        }
    
        // registering sockets in the correct fd_set
        void add_fd(int iSocketFD, unsigned long ulFlag)
        {
            if(iSocketFD >= MAX_ALLOWED_FD)
            {
                throw x_select_max_fd_exceeded();
            }
        
            // "registering" the FD in the correct select()'s set
            if(ulFlag & CReactorUtils::READ_EVENT)  FD_SET(iSocketFD, &m_master_rfdset); 
        
            if(ulFlag & CReactorUtils::WRITE_EVENT) FD_SET(iSocketFD, &m_master_wfdset);
        }
    
        // "unregistering" from fd_sets
        void remove_fd(int iSocketFD, unsigned long ulFlag)
        {
            // clear the sets for select()
            if(ulFlag & CReactorUtils::READ_EVENT)  FD_CLR(iSocketFD, &m_master_rfdset); 
        
            if(ulFlag & CReactorUtils::WRITE_EVENT) FD_CLR(iSocketFD, &m_master_wfdset);
        }
    
        // here for returned from select() "ready" descriptors callbacks are invoked
        int watch_fds()
        {
            //ONLY ONE THREAD AT A TIME SHOULD WAIT ON SELECT() - LEADERS/FOLLOWERS STRATEGY BY Douglas Schmidt (see lf.pdf)!!!
        
            /* select() modifies the descriptor sets pointed...
             * We use the FD_ISSET macro on return to test a specific descriptor in an fd_set structure.
             * Any descriptor that is not ready on retun will have its corresponding bit cleared in the descriptor set.
             * This is the reason for using copies of the "master" sets.
             * For more info see: UNP, part 1, page 152 
             */
             int iCountOfReadyFDs;
            do
            {
                m_working_rfdset = m_master_rfdset;
                m_working_wfdset = m_master_wfdset;
        
                int iMaxFD = get_max_fd(); // determine the FD with max value( the value is written in m_iMaxFD )
        
                iCountOfReadyFDs = ::select(iMaxFD+1, &m_working_rfdset, &m_working_wfdset, NULL, NULL);
            } while(check_fd(m_pipe[0], CReactorUtils::READ_EVENT) && process_internal_command());
        
            return iCountOfReadyFDs;
        }
    
        // this function is used for determining if given FD is "ready" for the event, marked with ulFlag
        bool check_fd(int iSocketFD, unsigned long ulFlag)
        {
            bool bIsReady = false;
        
            if( (ulFlag & CReactorUtils::READ_EVENT) && FD_ISSET(iSocketFD, &m_working_rfdset) )    bIsReady = true;
        
            if( (ulFlag & CReactorUtils::WRITE_EVENT) && FD_ISSET(iSocketFD, &m_working_wfdset) )   bIsReady = true;
        
            return bIsReady;
        }

    private:
        static const int MAX_ALLOWED_FD = FD_SETSIZE;

        fd_set m_master_rfdset;
        fd_set m_master_wfdset;
        fd_set m_working_rfdset;
        fd_set m_working_wfdset;

        int m_pipe[2];

    public:
        class x_select : public std::exception
        {
         public:
            x_select(const std::string& sMessage) throw()
                : m_sMessage(sMessage)
            {}

            virtual ~x_select() throw()
            {}

            virtual const char* what() const throw()
            {
                return m_sMessage.c_str();
            }

         protected:
            std::string m_sMessage;
        };

        class x_select_max_fd_exceeded : public x_select
        {
         public:
            x_select_max_fd_exceeded() throw()
                : x_select("select(): trying to add fd greater than FD_SETSIZE.")
            {}
            virtual ~x_select_max_fd_exceeded() throw()
            {}
        };
    };
}//namespace reactor

#endif /*OS_EVENT_DEMULTIPLEXER_SELECT_H*/

