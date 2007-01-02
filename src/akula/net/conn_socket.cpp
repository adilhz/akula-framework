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
    @file cxxxconnsocket.h 
 */

#include "conn_socket.h"
#include <akula/dbg/dbg.h>
#include "address.h"
#include <netinet/in.h>     /*struct sockaddr_in*/
#include <cerrno>

/*
-----------------------------------------------CSocketBase------------------------------------------------
*/

net::CSocketBase::CSocketBase(const int& iHandle, const net::CAddress* pPeerAddress)
    :CSocket(iHandle),
    m_pPeerAddress(pPeerAddress)
{
}

net::CSocketBase::CSocketBase(ESocketType type)
    :CSocket(type),
    m_pPeerAddress(NULL)
{
}

net::CSocketBase::~CSocketBase()
{
    delete m_pPeerAddress;
}

bool
net::CSocketBase::connect(const net::CAddress& peerAddress)
{
    struct sockaddr_in socketAddress;
    if(!peerAddress.toSocketAddress(socketAddress))
    {
        dbg::error() << "Cannot get socket address from " << peerAddress.toString() << "\n";
        return false;
    }

    if(::connect(m_iSocketDescr, (sockaddr *)&socketAddress, sizeof(socketAddress)) < 0)
    {
        dbg::error() << "socket connect failed: " << ::strerror(errno) << "\n";
        return false;
    }

    //keep the peer's address to determine if the socket is connected
    m_pPeerAddress = new CAddress(peerAddress);
    
    return true;
}

//Good examples of how to use recv, send are given in R.Stevens's example files: .../unpv12e/lib/readn.c, writen.c :-)

long
net::CSocketBase::read(char* ptr, long n)
{
    long nread = ::recv(m_iSocketDescr, ptr, n, 0);

    if(nread < 0)
    {
        dbg::error() << "Error reading from socket: " << strerror(errno) << "\n";
    }

    return nread;
}

long
net::CSocketBase::readN(char* ptr, long n)
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

// receive data from socket and fill in the remote socket's address info into CAddress
long
net::CSocketBase::readFrom(char* ptr, long n, net::CAddress** ppPeerAddress)
{
    struct sockaddr_in peerSocketAddress;
    int iPeerSocketAddressLength = sizeof(peerSocketAddress);

    // do we want the peer's address?
    struct sockaddr_in* ppeerSocketAddress = (ppPeerAddress) ? &peerSocketAddress : NULL;

    // loads data into pBuffer and fills in the remote socket's address info into remote_socket_address
    long lReceivedBytes = ::recvfrom(m_iSocketDescr, ptr, n, 0,
                                                     (sockaddr*)ppeerSocketAddress, (socklen_t*)&iPeerSocketAddressLength);

    if(lReceivedBytes == -1)
    {
        dbg::error() << "Error reading from socket " << m_iSocketDescr << " :" << strerror(errno) << "\n";
    }

    // peer's address
    if(ppPeerAddress)
    {
        *ppPeerAddress = CAddress::parse(peerSocketAddress);
    }

    return lReceivedBytes;
}

long
net::CSocketBase::sendN(const char* ptr, long n)
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

long
net::CSocketBase::send(const char* ptr, long n)
{
    long lBytesSent = ::send(m_iSocketDescr, ptr, n, 0);

    if(lBytesSent < 0)
    {
        dbg::error() << "Error sending from socket " << m_iSocketDescr << " :" << strerror(errno) << "\n";
    }

    return lBytesSent;
}

long
net::CSocketBase::sendTo(const char* ptr, long n, const net::CAddress& peerAddress)
{
    struct sockaddr_in peerSocketAddress;
    if(!peerAddress.toSocketAddress(peerSocketAddress))
    {
        dbg::error() << "Cannot get socket address from " << peerAddress.toString() << "\n";
        return false;
    }

    long lBytesSent = ::sendto(m_iSocketDescr, ptr, n, 0,
                                            (sockaddr*)&peerSocketAddress, sizeof(peerSocketAddress));

    return lBytesSent;
}

bool
net::CSocketBase::isConnected() const
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

/*
---------------------------------------------CTCPSocket------------------------------------
*/
net::CTCPSocket::CTCPSocket(const int& iHandle, const net::CAddress* pPeerAddress)
    :CSocketBase(iHandle, pPeerAddress)
{
}

net::CTCPSocket::CTCPSocket()
    : CSocketBase(TCP)
{
}

net::CTCPSocket::~CTCPSocket()
{
}

/*
----------------------------------------------CUDPSocket------------------------------------------
*/
net::CUDPSocket::CUDPSocket()
    : CSocketBase(UDP)
{
}

net::CUDPSocket::~CUDPSocket()
{
}

