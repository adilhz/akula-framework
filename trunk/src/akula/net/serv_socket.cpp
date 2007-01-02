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

#include "serv_socket.h"
#include "conn_socket.h"
#include "address.h"
#include <akula/dbg/dbg.h>
#include <sys/socket.h>    /*listen()*/
#include <netinet/in.h>    /*struct sockaddr_in*/
#include <cerrno>

net::CTCPServerSocket::CTCPServerSocket()
    : CSocket(TCP)
{
}

net::CTCPServerSocket::~ CTCPServerSocket()
{
}

// listening socket
bool
net::CTCPServerSocket::listen() const
{
    if((::listen(m_iSocketDescr, SOMAXCONN)) < 0)
    {
        dbg::error() << "listen() for socket " << m_iSocketDescr << "failed: " << strerror(errno) << "\n";
        return false;
    }

    return true;
}

// accepting and creating the connection socket
net::CTCPSocket*
net::CTCPServerSocket::accept()
{
    // client's socket address info
    struct sockaddr_in clientSocketAddress;
    int iAddrLength = sizeof(clientSocketAddress);

    // get the file descriptor of the connected socket, returned by accept;
    // fill in the client's address structure info into client_sock_addr
    int connSocketDescr = ::accept(m_iSocketDescr, (sockaddr*)&clientSocketAddress, (socklen_t*)&iAddrLength);
    
    if(connSocketDescr <= 0)
    {
        dbg::error() << "accept failed: " << strerror(errno) << "\n";
        return NULL;
    }

    // returning the instance of CTCPConnectionSocket, initialised with the descriptor of the connected socket
    // and the remote host's socket information
    net::CTCPSocket* pConnectedSocket = new net::CTCPSocket(connSocketDescr, net::CAddress::parse(clientSocketAddress));

    return pConnectedSocket;
}

