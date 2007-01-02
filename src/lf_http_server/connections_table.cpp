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

#include <cassert>
#include "connections_table.h"
#include <akula/reactor/reactor.h>
#include "connection.h"
#include <akula/utils/utils.h>
#include <akula/net/net.h>


/*
-------------------------------------------------
*/
//can we be sure that the MAX_OPEN_FILES_PER_PROCESS pointers are initizlized, i.e. with NULL values
// or we have to do it explicitly? Think about this! Is this specified by the standard?
CConnTableVectorImpl::CConnTableVectorImpl()
    :m_container(reactor::CReactorUtils::MAX_OPEN_FILES_PER_PROCESS) //yes, initialize with MAX_OPEN_...CConnection pointers
{

}

CConnTableVectorImpl::~CConnTableVectorImpl()
{

}

bool
CConnTableVectorImpl::addConnection(CConnection* pConnection)
{
    int iHandle = pConnection->getSocket()->getSocketHandle();

    //does not work concurrently with the connection, only a single thread is adding/removing
    //Guard<Thread_Mutex> guard(m_mutex); //TODO: Possible atomic operation!?

    m_container[iHandle] = pConnection;

    return true;
}

bool
CConnTableVectorImpl::removeConnection(CConnection *pConnection)
{
    int iHandle = pConnection->getSocket()->getSocketHandle();

    //does not work concurrently with the connection, only a single thread is adding/removing
    //Guard<Thread_Mutex> guard(m_mutex); //TODO: Possible atomic operation?!
    
    m_container[iHandle] = NULL;

    return true;
}

CConnection*
CConnTableVectorImpl::getConnection(net::CSocket *pSocket)
{
    int iHandle = pSocket->getSocketHandle();
    
    CConnection *pConnection = NULL;

    //does not work concurrently with the connection, only a single thread is adding/removing
    //Guard<Thread_Mutex> guard(m_mutex); //TODO: Atomic == possible ?
    
    pConnection = m_container[iHandle];

    return pConnection;
}
