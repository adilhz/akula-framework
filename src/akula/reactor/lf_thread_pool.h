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
#ifndef LF_THREAD_POOL_H
#define LF_THREAD_POOL_H

#include <akula/dbg/dbg.h>
#include <akula/utils/utils.h>
#include "reactor_utils.h"
#include <functional>

/**
LEADERS/FOLLOWERS pattern, described very well from D.Schmidt
Try google.com for "lf.pdf"

The following collaborations occur in the Leader/Followers pattern.
===> Leader thread demultiplexing. The leader thread waits for an event to occur on any
  handle in the handle set. If there is no current leader thread, for example, due to
  events arriving faster than the available threads can service them, the underlying
  operating system can queue events internally until a leader thread is available.
===> Follower thread promotion. After the leader thread has detected an new event, it
  uses the thread pool to choose a follower thread to become the new leader by using
  one of the promotion protocols described in implementation activity 5.
===> Event handler demultiplexing and event processing. After helping to promote a
  follower thread to become the new leader, the former leader thread then plays the
  role of a processing thread. This thread concurrently demultiplexes the event it
  detected to the event's associated handler and then dispatches the handler's hook
  method to process the event. A processing thread can execute concurrently with
  the leader thread and any other threads that are in the processing state.
===> Rejoining the thread pool. After the processing thread has run its event handling
  to completion, it can rejoin the thread pool and wait to process another event. A
  processing thread can become the leader immediately if there is no current leader
  thread. Otherwise, the processing thread returns to playing the role of a follower
  thread and waits on the thread pool synchronizer until it is promoted by a leader
  thread.
*/
namespace reactor
{
    template <class T_reactor>
    class CLFThreadPool
    {
    private:
        T_reactor& m_reactor;

        /**The current leader*/
        utils::CThread<>::ThreadId_t m_leaderThread;
        utils::Thread_Mutex m_mutex;
        utils::CConditionObject m_condition;
    
    public:
        static const utils::CThread<>::ThreadId_t NO_CURRENT_LEADER = 0;

    public:
        CLFThreadPool(T_reactor& reactor)
            : m_reactor(reactor)
            , m_leaderThread(NO_CURRENT_LEADER)
            , m_condition(m_mutex)
        {
        }

        ~CLFThreadPool()
        {
        }

        /**The threads that invokes this method joins the thread pool
        * @return <code>false</code> in case of error
        */
        bool join(void)
        {
            utils::Guard<utils::Thread_Mutex> guard(m_mutex);
                
            for(;;)
            {
                while(m_leaderThread != NO_CURRENT_LEADER)
                {
                    if(m_condition.wait())
                    {
                        dbg::error() << "Thread pool join failed: condition wait\n";
                        return false;
                    }
                }
            
                m_leaderThread = utils::CThread<>::self();
                //dbg::info() << "Leader thread: 0x" << std::hex << m_LeaderThread << "\n";
            
                if(guard.release())
                {
                    dbg::error() << "Thread pool join failed: mutex release\n";
                    return false;
                }
            
                reactor::CReactorUtils::SHandlerTriple ready;
                if(!m_reactor.getReadyEventHandler(ready))
                    continue; //ERROR!
            
                m_reactor.deactivate_socket(ready.m_psocket);

                promote_new_leader();

                bool bReactivate;

                if(ready.m_events & reactor::CReactorUtils::READ_EVENT)
                    bReactivate = ready.m_phandler->handle_read(ready.m_psocket);
                else if(ready.m_events & reactor::CReactorUtils::WRITE_EVENT)
                    bReactivate = ready.m_phandler->handle_write(ready.m_psocket);
                else
                    assert(false);

                if(bReactivate)
                    m_reactor.reactivate_socket(ready.m_psocket);
            
                if(guard.acquire())
                {
                    dbg::error() << "Thread pool join failed: mutex acqire\n";
                    return false;
                }
            }
        }


        /**After the current "leader" thread has been notified by the reactor for a new event it first
        * promotes new leader to wait for events on the reactor and becomes worker thread to
        * process the event. After it rejoins the pool in the role of follower (waiting to be promoted for leader)
        * or if there is no current leader (i.e. all threads are workers) it becomes leader.
        * So this method chooses the new leader. We have no strategy for leader choosing, so 
        * the OS itself promotes the new leader from all the followers.
        *
        * @return <code>false</code> in case of error
        */
        bool promote_new_leader()
        {
            utils::Guard<utils::Thread_Mutex> guard(m_mutex);
    
            if(m_leaderThread != utils::CThread<>::self())
                return false;
    
            m_leaderThread = NO_CURRENT_LEADER;
    
            if(m_condition.signal())
            {
                dbg::error() << "Thread pool promote_new_leader failed: condition signal\n";
                return false;
            }
    
            return true;
        }
    };
    
}
#endif /*LF_THREAD_POOL_H*/

