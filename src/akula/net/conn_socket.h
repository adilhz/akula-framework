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
    @file conn_socket.h 

    @brief
 */

#ifndef CONN_SOCKET_H
#define CONN_SOCKET_H

#include "socket.h"
#include "address.h"
#include <netinet/in.h>     /*struct sockaddr_in*/
#include <cerrno>

namespace net
{
    /*
    -------------------------------------------------CSocketBase--------------------------------------------
    */
    /**
        @class CConnectionSocket 

        @brief Abstract class. Common functions for the TCP and UDP sockets - receiving, transmitting data...
    */
    class CSocketBase : public CSocket
    {
     protected:
        /**Constructs socket*/
        CSocketBase(ESocketType type)
            :CSocket(type),
            m_pPeerAddress(NULL)
        {
        }

        /**
        * Constructs connected socket.
        * @param iHandle The socket handle
        * @param peerAddress The peer's address object IMPORTANT - You only pass the pointer to the
        * object and this class will take care for cleaning up the object. This is done in sake of performance, i.e.
        * to skip any unnecessary CAddress constructing and copying. This ctor is used for creating connected
        * TCP sockets after accepting new connection. The @param iHandle is the descriptor and the @param
        * peerAddress is the peer's address
        */
        CSocketBase(const int& iHandle, const net::CAddress* pPeerAddress)
            :CSocket(iHandle),
            m_pPeerAddress(pPeerAddress)
        {
        }

     public:
        virtual ~CSocketBase()
        {
            delete m_pPeerAddress;
        }
        

        /**
         * Wrapper for the standard connect() routine.
         * Connects to the remote sockaddr_in, specified using its abstraction CAddress
         * @param poCAddress	Abstraction of the sockaddr_in structure
         */
        void connect(const net::CAddress& peerAddress)
        {
            struct sockaddr_in socketAddress;
            peerAddress.toSocketAddress(socketAddress);
            
            if(::connect(m_iSocketDescr, (sockaddr *)&socketAddress, sizeof(socketAddress)) < 0)
                throw CNetException(strerror(errno), errno);
            
            //keep the peer's address to determine if the socket is connected
            m_pPeerAddress = new CAddress(peerAddress);
        }


        /**
         * Read data from connection socket.
         * @param pchBuffer	The buffer to store the data read (receiving buffer).
         * @param ilBufLen	The size of the receiving buffer.
         */
        long read(char* ptr, long n)
        {
            long nread = ::recv(m_iSocketDescr, ptr, n, 0);
        
            return nread;
        }

        long readN(char* ptr, long n)
        {
            long nleft = n;
            char* p = ptr;
        
            while(nleft > 0)
            {
                long nread = ::recv(m_iSocketDescr, p, nleft, 0);
                if(nread < 0) //error
                {
                    if(errno == EINTR)
                        nread = 0;          //and call read again
                    else
                        return -1;
                }
                else if(nread == 0) //EOF
                    break;
        
                nleft -= nread;
                p += nread;
            }
        
            return (n-nleft); // return >= 0
        }

        /**
         * Read data from connectionless socket.
         * @param pchBuffer     The buffer to store the data (receiving buffer).
         * @param ulBufLen      The size of the receiving buffer.
         * @param ppAddressFrom    Pointer to the pointer to the peer's address. NULL value means 
         * we don't need the peer's address. NOTE that if not-NULL you'll get new CAddress instance every
         * time this method is invoked, so be carefull to prevent memory leaks.
         */
         // receive data from socket and fill in the remote socket's address info into CAddress
        long readFrom(char* ptr, long n, net::CAddress** ppPeerAddress = NULL)
        {
            struct sockaddr_in peerSocketAddress;
            int iPeerSocketAddressLength = sizeof(peerSocketAddress);
            
            // do we want the peer's address?
            struct sockaddr_in* ppeerSocketAddress = (ppPeerAddress) ? &peerSocketAddress : NULL;
            
            // loads data into pBuffer and fills in the remote socket's address info into remote_socket_address
            long lReceivedBytes = ::recvfrom(m_iSocketDescr, ptr, n, 0,
                                                                 (sockaddr*)ppeerSocketAddress, (socklen_t*)&iPeerSocketAddressLength);
            
            // peer's address
            if(ppPeerAddress)
            {
                *ppPeerAddress = CAddress::parse(peerSocketAddress);
            }
            
            return lReceivedBytes;
        }

        /**
         * Write data to connection socket - Exits when all the given bytes ulBufLen are sent.
         * @param pchBuffer	The buffer to send.
         * @param ilBufLen	The size of the buffer.
         * @return number of sent bytes (= ulBufLen, if everything is OK)
         */
        long sendN(const char* ptr, long n)
        {
            long nleft = n;
            const char* p = ptr;
            
            while (nleft > 0)
            {
                long nwritten = ::send(m_iSocketDescr, p, nleft, 0);
                
                if(nwritten <= 0)
                {
                    if (errno == EINTR)
                        nwritten = 0;           /* and call write() again */
                    else
                        return(-1);             /* error */
                }
            
                nleft -= nwritten;
                p += nwritten;
            }
            
            return(n);
        }

        /**
         * Write data to connection socket - Do not guarantee all the bytes will be sent.
         * @param pchBuffer	The buffer to send.
         * @param ilBufLen	The size of the buffer.
         * @return number of sent bytes
         */
        long send(const char* ptr, long n)
        {
            long lBytesSent = ::send(m_iSocketDescr, ptr, n, 0);
            
            return lBytesSent;
        }

        /**
         * Write data to connectionless socket - Do not guarantee all the bytes will be sent.
         * @param pchBuffer	The buffer to send.
         * @param ilBufLen	The size of the buffer.
         * @param pAddressFrom	The far-side address - where the data should be sent
         * @return number of sent bytes
         */
        long sendTo(const char* ptr, long n, const net::CAddress& peerAddress)
        {
            struct sockaddr_in peerSocketAddress;
            peerAddress.toSocketAddress(peerSocketAddress);
            
            long lBytesSent = ::sendto(m_iSocketDescr, ptr, n, 0,
                                                   (sockaddr*)&peerSocketAddress, sizeof(peerSocketAddress));
            
            return lBytesSent;
        }

        /**
         * @return Abstraction of the remote socket address structure, only in case it exists, i.e.
         * the socket is connected.
         */
        const CAddress* getRemoteAddress() const {return m_pPeerAddress;};

        /**
         * This method is used to determine if the socket is connected.
         * The determination is based on the m_remote_socket_address structure. Its fields are 0 filled
         * till the socket is not connected. In case of connect() or accept() they are filled with the
         * appropriate values.
         * So in this function we just check if these fields are zero or non-zero.
         * @return bool	True if connected(the sockaddr_in for the remote socket address is w/ zero fields -
         * connected), otherwise false (not connected)
         */
        bool Connected() const
        {
#if 0
                // testing if the given descriptor is really socket descriptor
                // 'cos one of the possible errors returned from getpeername() is the fd is not socket fd
                // and we want to exclude this possibility :-)
                // for isfdtype() - R.Stevens, UNP Vol.1, page 81(Section 3.10) :-)
            
                if(0 <= isfdtype(m_iSocketDescr, S_IFSOCK))
                    return false;
            
            
                sockaddr_in remote_socket_address;
                int iAddrLength = sizeof(remote_socket_address);
            
                // for getsockname() & getpeername() - R.Stevens, UNP Vol.1, page 107(Section 4.10)
                if(::getpeername(CSocket::m_iSocketDescr, (sockaddr*)&remote_socket_address, (socklen_t*)&iAddrLength) < 0)
                    return true;
                else
                    return false;
#endif
            
            return m_pPeerAddress ? true : false;
       }


     protected:
        const CAddress* m_pPeerAddress;
    };

    /*
    ---------------------------------------------CTCPSocket--------------------------------------------------
    */
    /**
        @class CTCPConnectionSocket 

        @brief TCP connection socket abstraction
    */
    class CTCPSocket : public CSocketBase
    {
     public:
         CTCPSocket(const int& iHandle, const net::CAddress* pPeerAddress)
             :CSocketBase(iHandle, pPeerAddress)
         {
         }
         
         CTCPSocket()
             : CSocketBase(TCP)
         {
         }
         
         virtual ~CTCPSocket()
         {
         }
    };

    /*
    ----------------------------------------------CUDPSocket------------------------------------------
    */
    /**
        @class CTCPConnectionSocket 

        @brief UDP socket abstraction
    */  
    class CUDPSocket : public CSocketBase
    {
     public:
         CUDPSocket()
             : CSocketBase(UDP)
         {
         }
         
         virtual ~CUDPSocket()
         {
         }
    };

}/*namespace net*/
#endif //CONN_SOCKET_H

