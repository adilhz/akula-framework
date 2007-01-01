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
#include "server.h"
#include "configuration.h"
#include <akula/dbg/dbg.h>
#include <cassert>

main()
{
    CConfiguration* pConfiguration = CConfiguration::instance();

    dbg::ELevel level = pConfiguration->getLogLevel();
    std::string sServerAddress = pConfiguration->getAddress();
    int iNumThreads = pConfiguration->getThreads();

    dbg::enableLevelPrefix(true);
    dbg::setLogLevel(level);
    dbg::setLogStream(&std::cout);

    dbg::info() << "Log level: " << level << std::endl;
    dbg::info() << "Server address: " << sServerAddress << std::endl;
    dbg::info() << "Threads: " << iNumThreads << std::endl;

    CServer server;
    server.start(sServerAddress, iNumThreads);
}



