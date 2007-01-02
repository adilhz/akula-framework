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

namespace net
{
    class CAddress;

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
        CSocket(ESocketType) throw(x_socket_exception);

        CSocket(int iFD);

     public:
        virtual ~CSocket();

        /**
         * Binds the socket
         * @param poCAddress	The interface and port to bind to.
         */
        bool bind(CAddress *pAddress);

        /**
         * Returns the address the server is bound to
         */
        const CAddress* getBindAddress() const {return m_pBoundAddress;};

        bool isValid() const { return m_iSocketDescr != -1; }

        virtual void close () const { if(isValid()) ::close(m_iSocketDescr); }

        int getSocketHandle() const {return m_iSocketDescr;}

        bool setNonBlocking(void);

        bool setReuseAddress(void);
    };
}/*namespace net*/
#endif // SOCKET_H

