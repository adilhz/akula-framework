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

#ifndef CONNECTIONS_TABLE_H
#define CONNECTIONS_TABLE_H

#include <akula/utils/utils.h>
#include <vector>

class CConnection;
namespace net {class CSocket;}

/**
* Connections table interface to plug-in different implementations
*/
template<class T_engine>
class CConnTable
{
 private:
    T_engine m_engine;

 public:
    typedef typename T_engine::iterator iterator;

 public:
    iterator begin()
    {
        return m_engine.begin();
    }

    iterator end()
    {
        return m_engine.end();
    }

    bool addConnection(CConnection* pConnection)
    {
        return m_engine.addConnection(pConnection);
    }

    bool removeConnection(CConnection* pConnection)
    {
        return m_engine.removeConnection(pConnection);
    }

    CConnection* getConnection(net::CSocket* pSocket)
    {
        return m_engine.getConnection(pSocket);
    }
};

/**
* Vector-based connections table implementation. Using direct indexing.
*/
class CConnTableVectorImpl
{
 public:
    typedef std::vector<CConnection*> ConnContainer_t;
    ConnContainer_t m_container;
    utils::Thread_Mutex m_mutex;

 public:
    typedef ConnContainer_t::iterator iterator;

    CConnTableVectorImpl();
    ~CConnTableVectorImpl();

    iterator begin()
    {
        return m_container.begin();
    }

    iterator end()
    {
        return m_container.end();
    }

    bool addConnection(CConnection*);

    bool removeConnection(CConnection*);

    CConnection* getConnection(net::CSocket*);

};

/*
Example:
for(ConnectionsTable_t::iterator IT = m_pConnectionsTable->begin(); IT != m_pConnectionsTable->end(); IT++)
    if(*IT != NULL)
        std::cout << "<" << (*IT)->getSocket()->getSocketHandle() << ">" << " ";
*/
typedef CConnTable<CConnTableVectorImpl> ConnectionsTable_t;

#endif /*CONNECTIONS_TABLE_H*/

