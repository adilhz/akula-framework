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
#include "TUT/tut.h"
#include <akula/utils/utils.h>
#include <akula/reactor/reactor.h>
#include <iostream>
#include <string>
#include <functional>
#include <cstdlib>

namespace tut
{

    struct SReactorStarter : public std::unary_function<void*,void*>
    {
        void* operator()(void* parg)
        {
            reactor::CReactor* pReactor = reinterpret_cast<reactor::CReactor*>(parg);
            pReactor->handle_events(4);
            return NULL;
        }
    };

    struct SReactorStopper : public std::unary_function<void*,void*>
    {
        void* operator()(void* parg)
        {
            reactor::CReactor* pReactor = reinterpret_cast<reactor::CReactor*>(parg);
            ::sleep(5);
            pReactor->stop();
            return NULL;
        }
    };

    struct test_reactor
    {
    };

    typedef test_group<test_reactor> factory;
    typedef factory::object object;

    factory test_reactor("test_reactor");

    // start the reactor from main
    template<>
    template<>
    void object::test<1>()
    {
        dbg::enableLevelPrefix( true);
        dbg::setLogLevel( dbg::ALL);
        dbg::setLogStream( &std::cout);

        reactor::CReactor reactor;
        utils::CThread<SReactorStopper>* pThread = utils::CThread<SReactorStopper>::getInstance(&reactor);
        pThread->run();

        reactor.handle_events(4);

        pThread->join();

        ensure("reactor stopped", reactor.isStopped());
    }

    // start the reactor from another thread
    template<>
    template<>
    void object::test<2>()
    {
        dbg::enableLevelPrefix( true);
        dbg::setLogLevel( dbg::ALL);
        dbg::setLogStream( &std::cout);

        reactor::CReactor reactor;
        utils::CThread<SReactorStarter>* pThread1 = utils::CThread<SReactorStarter>::getInstance(&reactor);
        utils::CThread<SReactorStopper>* pThread2 = utils::CThread<SReactorStopper>::getInstance(&reactor);
        pThread1->run();
        pThread2->run();
        pThread1->join();
        pThread2->join();

        ensure("reactor stopped", reactor.isStopped());
    }

}

