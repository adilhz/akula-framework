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

#ifndef APP_EVENT_DEMULTIPLXER_ARRAY_H
#define APP_EVENT_DEMULTIPLXER_ARRAY_H

#include "reactor_utils.h"
#include <akula/net/socket.h>
#include <akula/utils/guard.h>
#include <akula/utils/thread_mutex.h>
#include <vector>
#include <cerrno>
#include <cassert>

namespace reactor
{

template<class T_osLevelDemultiplexer>
class AppEventDemultiplexerArrayImpl
{
 private:
    typedef std::vector<CReactorUtils::SHandlerTriple*> HandlersContainer_t;
    typedef HandlersContainer_t::iterator HandlersContainerIt_t;

    typedef std::vector<int> IndexContainer_t;
    typedef IndexContainer_t::iterator IndexContainerIt_t;

    HandlersContainer_t m_handlers;
    IndexContainer_t m_indexes;

    utils::Thread_Mutex m_mutex;

    int m_iNumberOfRegisteredSockets;

    /**the os-level demux mechanism*/
    T_osLevelDemultiplexer m_OsLevelDemultiplexer;

    bool m_bStop;

 public:
    AppEventDemultiplexerArrayImpl()
        :m_handlers(CReactorUtils::maxOpenFilesPerProcess()) //all the elements should be NULL
        , m_indexes(CReactorUtils::maxOpenFilesPerProcess(), -1) //fill in the index table with -1
        , m_iNumberOfRegisteredSockets(0)
        , m_bStop(false)
    {
    }
    
    ~AppEventDemultiplexerArrayImpl()
    {
        HandlersContainerIt_t IT;
        int i;
        for(IT = m_handlers.begin(), i = 0; IT != m_handlers.end() && i < m_iNumberOfRegisteredSockets; IT++, i++)
            delete *IT;
    }

    void stop(void)
    {
        m_bStop = true;
        m_OsLevelDemultiplexer.stop();
    }

    bool isStopped(void) const
    {
        return m_bStop;
    }

    void
    register_socket(net::CSocket* pSocket, CReactorUtils::EventType_t events, CReactorUtils::IEventHandler* pHandler)
    {
        utils::Guard<utils::Thread_Mutex> guard(m_mutex);
        
        int iIndex = m_indexes[pSocket->getSocketHandle()];
        if(iIndex == -1) //not registered for other events
        {
            int iCurrIndex = m_iNumberOfRegisteredSockets++;
    
            CReactorUtils::SHandlerTriple* pHandlers = 
                new CReactorUtils::SHandlerTriple(pHandler, pSocket, events);
    
            m_handlers[iCurrIndex] = pHandlers;
            m_indexes[pSocket->getSocketHandle()] = iCurrIndex;
        }
        else //already registered
        {
            assert(m_handlers[iIndex] != NULL);

            m_handlers[iIndex]->m_events |= events;
        }

        m_OsLevelDemultiplexer.add_fd(pSocket->getSocketHandle(), events);
    }
    
    void
    unregister_socket(net::CSocket *pSocket, CReactorUtils::EventType_t events)
    {
        utils::Guard<utils::Thread_Mutex> guard(m_mutex);
        
        //unregister from low-level event demultiplexor
        m_OsLevelDemultiplexer.remove_fd(pSocket->getSocketHandle(), events);
    
        int iIndex = m_indexes[pSocket->getSocketHandle()];
        if(iIndex != -1)
        {
            assert(m_handlers[iIndex] != NULL);
            
            m_handlers[iIndex]->m_events &= (~events);
            if(m_handlers[iIndex]->m_events == 0)
            {
                delete m_handlers[iIndex];
                m_handlers[iIndex] = NULL;
                m_indexes[pSocket->getSocketHandle()] = -1;

                int iLastHandlersIndex = --m_iNumberOfRegisteredSockets;
                if(iIndex != iLastHandlersIndex)
                {
                    m_handlers[iIndex] = m_handlers[iLastHandlersIndex];
                    m_handlers[iLastHandlersIndex] = NULL;
                    m_indexes[m_handlers[iIndex]->m_psocket->getSocketHandle()] = iIndex;
                }
            }
        }
    }
    
    void
    deactivate_socket(net::CSocket *pSocket)
    {
        /*
        During deactivation we unregister the socket descriptor for all the regiestered events, because if we unregister only for
        a given event problems are possible. For example if we have a socket registered for read and write and deactivate 
        only for read, i.e. write remains, and have received EOF during reading we are deleting the corresponding connection
        (CConnection). If at the same moment we are ready for write event, the deletion will be blocked for the period of 
        invoking the write callback for this connection and right after calback invocation the connection is deleted, so the
        only thing that can happen is segmantation fault!
        */

        utils::Guard<utils::Thread_Mutex> guard(m_mutex);

        // The deactivation takes part in the lower level demultiplexer, we are adding new events to the file descriptor, 
        // but we are telling the OS to not watch this descriptor, i.e. it stays in our structures and only excluded from the OS monitoring.
        m_OsLevelDemultiplexer.deactivate_fd(pSocket->getSocketHandle());
    }
    
    void
    reactivate_socket(net::CSocket *pSocket)
    {
        utils::Guard<utils::Thread_Mutex> guard(m_mutex);
        m_OsLevelDemultiplexer.reactivate_fd(pSocket->getSocketHandle());
    }

    bool getReadyEventHandler(CReactorUtils::SHandlerTriple& ready)
    {
        int iResult = m_OsLevelDemultiplexer.watch_fds();

        if(isStopped() || iResult < 0)
            return false;
        
        if(iResult == 0) // timer
        {
            //TODO: fill in ready timer handlers
            return true;
        }
    
        // find the ready event handlers
        int i;
        bool bFound = false;
        HandlersContainerIt_t IT;
        utils::Guard<utils::Thread_Mutex> guard(m_mutex);
        for(IT = m_handlers.begin(), i = 0;
                IT != m_handlers.end() && i < m_iNumberOfRegisteredSockets;
                IT++, i++)
        {
            assert((*IT) != NULL);
    
            if(((*IT)->m_events & CReactorUtils::READ_EVENT) &&
                m_OsLevelDemultiplexer.check_fd((*IT)->m_psocket->getSocketHandle(), CReactorUtils::READ_EVENT))
            {
                ready.m_events |= CReactorUtils::READ_EVENT;
                bFound = true;
            }
        
            if(((*IT)->m_events & CReactorUtils::WRITE_EVENT) &&
                m_OsLevelDemultiplexer.check_fd((*IT)->m_psocket->getSocketHandle(), CReactorUtils::WRITE_EVENT))
            {
                ready.m_events |= CReactorUtils::WRITE_EVENT;
                bFound = true;
            }
    
            if(bFound)
            {
                ready.m_phandler = (*IT)->m_phandler;
                ready.m_psocket = (*IT)->m_psocket;
                break;
            }
        }
    
        return bFound;
    }
};

}//namespace reactor

#endif /*APP_EVENT_DEMULTIPLXER_ARRAY_H*/

