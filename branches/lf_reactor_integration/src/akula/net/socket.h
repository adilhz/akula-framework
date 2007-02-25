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
    @file csocket.h 

    @brief Interface for CSocket class
*/

#ifndef SOCKET_H
#define SOCKET_H

#include "exceptions.h"
#include <fcntl.h>           /*fcntl()*/
#include <cerrno>          /*errno*/
#include <cstring>          /*strerror()*/
#include <sstream>
#include <netinet/in.h>    /*struct sockaddr_in*/
#include <sys/socket.h>
#include <sys/types.h>
#include "address.h"

namespace net
{
    /**    @class CSocket 
    * @brief Abstraction and basic functions for the socket in network programming
    */
    class CSocket
    {
     protected:
           enum ESocketType {TCP, UDP};
           int m_iSocketDescr;
           const CAddress* m_pBoundAddress;

     protected:
        /**Creates socket of type specified by
        * @param ESocketType TCP or UDP
        * @throws x_socket_excpetion in case socket creation failed
        */
        CSocket(ESocketType type)
            :m_pBoundAddress(NULL)
        {
            switch(type)
            {
                case TCP:
                    m_iSocketDescr = socket(AF_INET, SOCK_STREAM, 0);
                break;
                case UDP:
                    m_iSocketDescr = socket(AF_INET, SOCK_DGRAM, 0);
                break;
            }
            
            if(!isValid())
            {
                std::stringstream ss;
                ss << "Couldn't create socket: " << strerror(errno);
                throw CNetException( ss.str());
            }
        }

        /*This is used for initialising a CTCPConnectionSockets which are created by 
         *accepting connections by CServerSocket's accept() method, which wraps the accept() function; 
         *accept() creates connection socket on the server and returns the socket descriptor of the new created connected socket 
         */
        CSocket(int iFD)
            :m_iSocketDescr(iFD),
            m_pBoundAddress(NULL)
        {
        }

     public:
        virtual ~CSocket()
        {
            delete m_pBoundAddress;
            close();
        }

        /**
         * Binds the socket
         * @param poCAddress	The interface and port to bind to.
         */
        void bind(net::CAddress *pAddress)
        {
            struct sockaddr_in socketAddress;
            pAddress->toSocketAddress(socketAddress);
            
            if(::bind(m_iSocketDescr, (struct sockaddr *)&socketAddress, sizeof(socketAddress)) < 0)
                throw CNetException(strerror(errno), errno);
            
            //keep the bind address
            // we are using default copy constructor
            m_pBoundAddress = new CAddress(*pAddress);
        }

        /**
         * Returns the address the server is bound to
         */
        const CAddress* getBindAddress() const {return m_pBoundAddress;};

        bool isValid() const { return m_iSocketDescr != -1; }

        virtual void close () const { if(isValid()) ::close(m_iSocketDescr); }

        int getSocketHandle() const {return m_iSocketDescr;}

        bool setNonBlocking(void)
        {
            //make the opened socket blocking - R. Stevens - Chapter 15 (15.1 - Introduction)
            int flags;
            if((flags = ::fcntl(m_iSocketDescr, F_GETFL, 0)) < 0)
                return false;
        
            flags |= O_NONBLOCK;
            if(fcntl(m_iSocketDescr, F_SETFL, flags) < 0)
                return false;
        
            return true;
        }

        bool setReuseAddress(void)
        {
            /* This socket option tells the kernel that even if this port is busy (in
            * the TIME_WAIT state), go ahead and reuse it anyway.  If it is busy,
            * but with another state, you will still get an address already in use
            * error.  It is useful if your server has been shut down, and then
            * restarted right away while sockets are still active on its port.  You
            * should be aware that if any unexpected data comes in, it may confuse
            * your server, but while this is possible, it is not likely.
            *
            * It has been pointed out that "A socket is a 5 tuple (proto, local
            * addr, local port, remote addr, remote port).  SO_REUSEADDR just says
            * that you can reuse local addresses.  The 5 tuple still must be
            * unique!" by Michael Hunter (mphunter@qnx.com).  This is true, and this
            * is why it is very unlikely that unexpected data will ever be seen by
            * your server.  The danger is that such a 5 tuple is still floating
            * around on the net, and while it is bouncing around, a new connection
            * from the same client, on the same system, happens to get the same
            * remote port.  This is explained by Richard Stevens in ``2.7 Please
            * explain the TIME_WAIT state.''.
            */
            int on = 1;
            return (setsockopt(m_iSocketDescr, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) < 0) ? false : true;
        }
    };
}/*namespace net*/

#endif // SOCKET_H

