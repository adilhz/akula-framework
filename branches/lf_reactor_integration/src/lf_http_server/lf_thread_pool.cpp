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

#include "lf_thread_pool.h"
#include <akula/dbg/dbg.h>
#include <akula/utils/utils.h>

const utils::CThread<>::ThreadId_t CLFThreadPool::NO_CURRENT_LEADER = 0;

CLFThreadPool::CLFThreadPool()
    : m_condition(m_mutex)
    , m_pReactor(reactor::getReactor())
    , m_LeaderThread(NO_CURRENT_LEADER)
{
}

CLFThreadPool::~CLFThreadPool()
{
}


bool
CLFThreadPool::join()
{
    utils::Guard<utils::Thread_Mutex> guard(m_mutex);
    
    for(;;)
    {
        while(m_LeaderThread != NO_CURRENT_LEADER)
        {
            if(m_condition.wait())
            {
                dbg::error() << "Thread pool join failed: condition wait\n";
                return false;
            }
        }

        m_LeaderThread = utils::CThread<>::self();
        //dbg::info() << "Leader thread: 0x" << std::hex << m_LeaderThread << "\n";

        if(guard.release())
        {
            dbg::error() << "Thread pool join failed: mutex release\n";
            return false;
        }

        m_pReactor->handle_events();

        if(guard.acquire())
        {
            dbg::error() << "Thread pool join failed: mutex acqire\n";
            return false;
        }
    }
}

bool
CLFThreadPool::promote_new_leader()
{
    utils::Guard<utils::Thread_Mutex> guard(m_mutex);

    if(m_LeaderThread != utils::CThread<>::self())
        return false;

    m_LeaderThread = NO_CURRENT_LEADER;

    if(m_condition.signal())
    {
        dbg::error() << "Thread pool promote_new_leader failed: condition signal\n";
        return false;
    }

    return true;
}


