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

/**
    @file serv_socket.h 

*/

#ifndef SERV_SOCKET_H
#define SERV_SOCKET_H

#include "socket.h"
#include "conn_socket.h"
#include "address.h"
#include <akula/dbg/dbg.h>
#include <sys/socket.h>    /*listen()*/
#include <netinet/in.h>    /*struct sockaddr_in*/
#include <cerrno>


namespace net
{
    /**  @class CTCPServerSocket
     * @brief Abstraction of TCP server socket: listen, accept.
     */
    class CTCPServerSocket : public CSocket
    {
     public:
        CTCPServerSocket()
            : CSocket(TCP)
        {
        }

        virtual ~CTCPServerSocket()
        {
        }

        /**
         * Wrapper for the listen() standard fubction
         *
         * @exception x_socket_exception	In case listen() returns error value
         */
        void listen() const
        {
            if((::listen(m_iSocketDescr, SOMAXCONN)) < 0)
                throw CNetException(strerror(errno), errno);
        }

        /**
         * Wrapper for the accept() standard fubction
         *
         * @return CConnectionSocket	In common this is a CTCPConnectionSocket connection socket
         * initialized with the connection socket descriptor and with the remote socket address structure
         */
        CTCPSocket* accept()
        {
            // client's socket address info
            struct sockaddr_in clientSocketAddress;
            int iAddrLength = sizeof(clientSocketAddress);
            
            // get the file descriptor of the connected socket, returned by accept;
            // fill in the client's address structure info into client_sock_addr
            int connSocketDescr = ::accept(m_iSocketDescr, (sockaddr*)&clientSocketAddress, (socklen_t*)&iAddrLength);
            
            if(connSocketDescr <= 0)
                throw CNetException(strerror(errno), errno);
            
            // returning the instance of CTCPConnectionSocket, initialised with the descriptor of the connected socket
            // and the remote host's socket information
            net::CTCPSocket* pConnectedSocket = new net::CTCPSocket(connSocketDescr, net::CAddress::parse(clientSocketAddress));
            
            return pConnectedSocket;
        }

     private:
        // make sure they are not generated automatically for us
        CTCPServerSocket(const CTCPServerSocket&);

        CTCPServerSocket& operator=(const CTCPServerSocket&);
    };
}/*namespace net*/
#endif //SERV_SOCKET_H

