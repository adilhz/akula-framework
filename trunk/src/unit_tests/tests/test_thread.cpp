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
#include "akula/utils/thread.h"
#include <iostream>
#include <string>
#include <functional>

namespace tut
{
    struct SThreadFunctor : public std::unary_function<void*,void*>
    {
        void* operator()(void* pv)
        {
            int* parg = reinterpret_cast<int*>(pv);
            ensure("thread argument", *parg == 0);

            (*parg)++;

            return parg;
        }
    };

    struct test_thread
    {
    };

    typedef test_group<test_thread> factory;
    typedef factory::object object;

    factory test_thread("test_thread");

    /**Test joinable thread - creation, argument receiving, return value*/
    template<>
    template<>
    void object::test<1>()
    {
        int arg = 0;
        int* parg = &arg;
        
        utils::CThread<SThreadFunctor>* pThread = 
            utils::CThread<SThreadFunctor>::getInstance(reinterpret_cast<void*>(parg));

        ensure("thread starting", pThread->run());
        ensure("thread joining", pThread->join((void**)&parg));
        ensure("thread result", *parg == 1);
    }
}

