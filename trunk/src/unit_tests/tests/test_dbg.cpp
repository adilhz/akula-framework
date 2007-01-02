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
#include "akula/dbg/dbg.h"
#include <iostream>
#include <sstream>

namespace tut
{
    struct test_dbg_data
    {
        std::ostringstream* pOSS;

        test_dbg_data()
        {
            pOSS = new std::ostringstream();
        }

        ~test_dbg_data()
        {
            delete pOSS;
        }
    };

    typedef test_group<test_dbg_data> factory;
    typedef factory::object object;

    factory test_dbg("test_dbg");

    template<>
    template<>
    void object::test<1>()
    {
        dbg::enableLevelPrefix( true);
        dbg::setLogLevel( dbg::ALL);
        dbg::setLogStream( pOSS);

        dbg::error() << "test";
        ensure("with level prefix", pOSS->str().compare("error:test") == 0);
    }

    template<>
    template<>
    void object::test<2>()
    {
        dbg::enableLevelPrefix( false);
        dbg::setLogLevel( dbg::ALL);
        dbg::setLogStream( pOSS);

        dbg::error() << "test";
        ensure("w/o level prefix", pOSS->str().compare("test") == 0);
    }

    template<>
    template<>
    void object::test<3>()
    {
        dbg::enableLevelPrefix( false);
        dbg::setLogLevel( dbg::ERROR);
        dbg::setLogStream( pOSS);

        dbg::debug() << "test";
        ensure("debug disabled", pOSS->str().compare("") == 0);

        dbg::setLogLevel( dbg::DEBUG);
        dbg::debug() << "test";
        ensure("debug enabled", pOSS->str().compare("test") == 0);
    }
}
