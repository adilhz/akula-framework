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

#ifndef APP_EVENT_DEMULTIPLXER_MAP_H
#define APP_EVENT_DEMULTIPLXER_MAP_H

#include "reactor_utils.h"
#include "os_event_demultiplexer.h"
#include <akula/dbg/dbg.h>
#include <akula/utils/guard.h>
#include <akula/net/socket.h>
#include <cerrno>
#include <akula/utils/thread_mutex.h>
#include <map>
#include <iostream>

namespace reactor
{

template<class T_osLevelDemultiplexer>
class AppEventDemultiplexerMapImpl
{
 private:
    typedef std::map<int, CReactorUtils::CEventHandlers*>  EventHandlersTable_t;
    typedef EventHandlersTable_t::iterator      EventHandlersTableIterator_t;
 
    EventHandlersTable_t m_EventHandlersTable;
    utils::Thread_Mutex m_mutex;

    /**os-level demux mechanism*/
    OsEventDemultiplexer<T_osLevelDemultiplexer> m_OsLevelDemultiplexer;

 public:
    inline
    unsigned long
    get_events(const CReactorUtils::CEventHandlers& handlers) const
    {
        unsigned long ulRetVal = CReactorUtils::NO_REGISTERED_EVENTS;
        if(handlers.pReadEventHandler != NULL) ulRetVal |= CReactorUtils::READ_MASK;
        if(handlers.pWriteEventHandler != NULL) ulRetVal |= CReactorUtils::WRITE_MASK;

        return ulRetVal;
    }
    
    AppEventDemultiplexerMapImpl()
    {
    }
    
    void
    register_socket(net::CSocket *pSocket, const unsigned long ulEvents, CReactorUtils::IEventHandler* pEventHandler)
    {
        utils::Guard<utils::Thread_Mutex> guard(m_mutex);
    
        EventHandlersTableIterator_t IT = m_EventHandlersTable.find(pSocket->getSocketHandle());
            
        if(IT == m_EventHandlersTable.end() ) //the socket is not registered
        {
            CReactorUtils::CEventHandlers* pHandlers = new CReactorUtils::CEventHandlers(
                                                    (ulEvents & CReactorUtils::READ_MASK) ? pEventHandler : NULL,
                                                    (ulEvents & CReactorUtils::WRITE_MASK) ? pEventHandler : NULL,
                                                    pSocket);

            m_EventHandlersTable.insert(EventHandlersTable_t::value_type(pSocket->getSocketHandle(), pHandlers));
        }
        else //the socket is already registered - register it for new event
        {
            if(ulEvents & CReactorUtils::READ_MASK)    IT->second->pReadEventHandler = pEventHandler;
            if(ulEvents & CReactorUtils::WRITE_MASK)   IT->second->pWriteEventHandler = pEventHandler;
        }

        try
        {
            // register in the os-level reactor
            m_OsLevelDemultiplexer.add_fd(pSocket->getSocketHandle(), ulEvents);
        }
        catch(std::exception& e)
        {
            dbg::error() << "Error registering descriptor in the os " << e.what() << std::endl;
        }

        dbg::debug() << "Registered socket " << pSocket->getSocketHandle() 
                                    << "for event " << ulEvents << std::endl;
    }
    
    void
    unregister_socket(net::CSocket *pSocket, unsigned long ulEvents)
    {
        utils::Guard<utils::Thread_Mutex> guard(m_mutex);
    
        m_OsLevelDemultiplexer.remove_fd(pSocket->getSocketHandle(), ulEvents);
    
        EventHandlersTableIterator_t IT = m_EventHandlersTable.find(pSocket->getSocketHandle());
        
        if(IT != m_EventHandlersTable.end())
        {
            if(ulEvents & CReactorUtils::READ_MASK)    IT->second->pReadEventHandler = NULL;
            if(ulEvents & CReactorUtils::WRITE_MASK)       IT->second->pWriteEventHandler = NULL;
            if(ulEvents & CReactorUtils::REMOVE_MASK)
            {
                delete IT->second;
                IT->second = NULL;
                m_EventHandlersTable.erase(IT);
            }

            dbg::debug() << "Unregistered socket " << pSocket->getSocketHandle()
                                        << "for event " << ulEvents
                                        << std::endl;
        }
        else
        {
            dbg::warning() << "Socket " << pSocket->getSocketHandle() << "not found" << std::endl;
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
    
        dbg::debug() << "Deactivated socket " << pSocket->getSocketHandle() << std::endl;
    }
    
    void
    reactivate_socket(net::CSocket *pSocket)
    {
        utils::Guard<utils::Thread_Mutex> guard(m_mutex);
    
        m_OsLevelDemultiplexer.reactivate_fd(pSocket->getSocketHandle());
    
        dbg::debug() << "Reactivated socket " << pSocket->getSocketHandle() << std::endl;
    }
    
    int
    handle_events(void)
    {
        int iResult = m_OsLevelDemultiplexer.watch_fds();
    
        if(iResult < 0)
        {
            dbg::error() << "Error with the os-level demultiplexer: " << ::strerror(errno) << std::endl;
            return iResult;
        }
        
        if(iResult == 0)
        {
            dbg::debug() << "Timer expired" << std::endl;
            return iResult;
        }
        
        //lock the map to serialize the access from different threads - multiple threads can add/remove callbacks simultaneously
        utils::Guard<utils::Thread_Mutex> guard(m_mutex);
        // Invoke handler - the first matching ready handler will be triggered
        for(EventHandlersTableIterator_t IT = m_EventHandlersTable.begin(); IT != m_EventHandlersTable.end(); IT++)
        {
            // Read event has higher priority than write event
            if((get_events(*(IT->second)) & CReactorUtils::READ_MASK) &&
                m_OsLevelDemultiplexer.check_fd(IT->first, CReactorUtils::READ_MASK)) // handler for read
            {
                dbg::debug() << "Invoking fd " << IT->first << " (" << IT->second->pSocket->getSocketHandle() << ") "
                                            << "for event " << CReactorUtils::READ_MASK << std::endl;
    
                guard.release();
                IT->second->pReadEventHandler->callback(IT->second->pSocket, CReactorUtils::READ_MASK); // trigger handler
                return iResult;
            }
    
            if((get_events(*(IT->second)) & CReactorUtils::WRITE_MASK) &&
                m_OsLevelDemultiplexer.check_fd(IT->first, CReactorUtils::WRITE_MASK)) // handler for write
            {
                dbg::debug() << "Invoking fd " << IT->first << " (" << IT->second->pSocket->getSocketHandle() << ") "
                                            << "for event " << CReactorUtils::WRITE_MASK << std::endl;
    
                guard.release();
                IT->second->pWriteEventHandler->callback(IT->second->pSocket, CReactorUtils::WRITE_MASK); // trigger handler
                return iResult;
            }
        }
 
        dbg::error() << "Nandler not found!!! Something is wrong..." << std::endl;
        return iResult;
    }
};

}//namespace reactor

#endif /*AIM_APP_EVENT_DEMULTIPLXER_MAP_H*/

