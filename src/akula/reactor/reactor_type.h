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
#ifndef REACTOR_TYPE_H
#define REACTOR_TYPE_H

#include "app_event_demultiplexer_array.h"
#include "os_event_demultiplexer_poll.h"
#include "reactor_utils.h"
#include "lf_thread_pool.h"
#include <akula/net/socket.h>
#include <akula/utils/singleton.h>
#include <akula/dbg/dbg.h>

namespace reactor
{
    class CReactor
    {
     private:
        // Here is the place to add engines
        AppEventDemultiplexerArrayImpl<OsEventDemultiplexerPollImpl> m_engine;

        CLFThreadPool<CReactor> m_LFThreadPool;

         struct SLFThread : public std::unary_function<void*,void*>
         {
             void* operator()(void* parg)
             {
                 CLFThreadPool<CReactor>* pThreadPool = reinterpret_cast<CLFThreadPool<CReactor>*>(parg);
                 if(pThreadPool)
                 {
                     pThreadPool->join();
                 }
                 
                 return NULL;
             }
         };

        bool m_bStop;

     public:
        CReactor()
            : m_LFThreadPool(*this)
            , m_bStop(false)
        {
        }
        
         static CReactor* singleton()
         {
             return utils::ThreadSafeSingleton<CReactor>::getInstance();
         }

        void stop(void)
        {
            m_bStop = true;
            m_engine.stop();
            m_LFThreadPool.stop();
        }

        bool isStopped() const { return m_bStop;}

        void register_socket(net::CSocket* pSocket, CReactorUtils::EventType_t events, CReactorUtils::IEventHandler* pHandler)
        {
            m_engine.register_socket(pSocket, events, pHandler);
        }
        
        void unregister_socket(net::CSocket* pSocket, CReactorUtils::EventType_t events)
        {
            m_engine.unregister_socket(pSocket, events);
        }

        void deactivate_socket(net::CSocket* pSocket)
        {
            m_engine.deactivate_socket(pSocket);
        }
        
        void reactivate_socket(net::CSocket* pSocket)
        {
            m_engine.reactivate_socket(pSocket);
        }

        bool getReadyEventHandler(CReactorUtils::SHandlerTriple& ready)
        {
            return m_engine.getReadyEventHandler(ready);
        }

        void handle_events(unsigned int threads = 1)
        {
            if(threads > 1)
                handle_events_lf(threads);
            else
            {
                while(!isStopped())
                {
                    CReactorUtils::SHandlerTriple ready;
                    if(!getReadyEventHandler(ready))
                        continue; //ERROR or Reactor stopped?! It'll be better to check explicitely for both - stop: (getReadyEventHandler() == false && isStopped() == true)

                    if(ready.m_events & CReactorUtils::READ_EVENT)
                        ready.m_phandler->handle_read(ready.m_psocket);
                    else if(ready.m_events & CReactorUtils::WRITE_EVENT)
                        ready.m_phandler->handle_write(ready.m_psocket);
                    else
                        assert(false);
                }
            }
        }

    private:
        void handle_events_lf(unsigned int threads)
        {
            //start threads
            for(int i = 0; i < threads -1; i++)
            {
                utils::CThread<SLFThread>* pThread = 
                    utils::CThread<SLFThread>::getInstance(reinterpret_cast<void*>(&m_LFThreadPool), true /*detached*/);
                
                if(!pThread || !pThread->run())
                    assert(false);
            }

            // join the thread that invoked reactor
            m_LFThreadPool.join();
        }
    };
}/*namespace reactor*/

#endif /*REACTOR_TYPE_H*/

