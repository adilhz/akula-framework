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

namespace net
{
    class CTCPSocket;

    /**  @class CTCPServerSocket
     * @brief Abstraction of TCP server socket: listen, accept.
     */
    class CTCPServerSocket : public CSocket
    {
     public:
        CTCPServerSocket();

        virtual ~CTCPServerSocket(); 

        /**
         * Wrapper for the listen() standard fubction
         *
         * @exception x_socket_exception	In case listen() returns error value
         */
        bool listen() const;

        /**
         * Wrapper for the accept() standard fubction
         *
         * @return CConnectionSocket	In common this is a CTCPConnectionSocket connection socket
         * initialized with the connection socket descriptor and with the remote socket address structure
         */
        CTCPSocket* accept();

     private:
        // make sure they are not generated automatically for us
        CTCPServerSocket(const CTCPServerSocket&);

        CTCPServerSocket& operator=(const CTCPServerSocket&);
    };
}/*namespace net*/
#endif //SERV_SOCKET_H

