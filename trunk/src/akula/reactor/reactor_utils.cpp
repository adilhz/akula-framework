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

#include "reactor_utils.h"
#include <akula/net/socket.h>
#include <unistd.h>                                     /*getdtablesize()*/

const unsigned long reactor::CReactorUtils::MAX_OPEN_FILES_PER_PROCESS = ::getdtablesize();

const unsigned long reactor::CReactorUtils::NO_REGISTERED_EVENTS = 0x0;
const unsigned long reactor::CReactorUtils::READ_MASK =( 0x01 << 0 );
const unsigned long reactor::CReactorUtils::WRITE_MASK =( 0x01 << 1 );
const unsigned long reactor::CReactorUtils::REMOVE_MASK = (0x01 << 2);

reactor::CReactorUtils::CEventHandlers::CEventHandlers(reactor::CReactorUtils::IEventHandler* pRHandler,
                                                                                        reactor::CReactorUtils::IEventHandler* pWHandler,
                                                                                        net::CSocket* socket)
    : pReadEventHandler(pRHandler),
    pWriteEventHandler(pWHandler),
    pSocket(socket)
{
}

reactor::CReactorUtils::CEventHandlers::~CEventHandlers()
{
    pReadEventHandler = NULL;
    pWriteEventHandler = NULL;
    pSocket = NULL;
}

reactor::CReactorUtils::CEventHandlers&
reactor::CReactorUtils::CEventHandlers::operator=(const reactor::CReactorUtils::CEventHandlers& rhs)
{
    pReadEventHandler = rhs.pReadEventHandler;
    pWriteEventHandler = rhs.pWriteEventHandler;
    pSocket = rhs.pSocket;
    return *this;
}

