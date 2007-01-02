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
    @file caddress.cpp 

    @brief Implementation for CAddress class
 */

#include "address.h"
#include <akula/dbg/dbg.h>
#include <sstream>
#include <netdb.h>          /* hostent struct, gethostbyname() */
#include <arpa/inet.h>      /* inet_ntoa() to format IP address */
#include <netinet/in.h>     /* in_addr structure */

/*
---------------------------------------------------CAddress---------------------------------------------
*/
net::CAddress::CAddress(const std::string& sHost, const unsigned short int& iPort)
    :m_sHost(sHost),
    m_uiPort(iPort)
{

}

std::string
net::CAddress::toString() const
{
    std::stringstream ss;
    ss << m_sHost << ":" << m_uiPort;
    return ss.str();
}

net::CAddress*
net::CAddress::parse(const std::string& sAddress)
{
    /*transform is member function of std::string.
     *OutputIterator transform (InputIterator first, InputIterator last, OutputIterator result, UnaryOperation op);
     *Iterate through a container (here, a string) from first to just before last, applying the operation op to each
     *container element and storing the results through the result iterator, which is incremented after each value is stored. 
     *This STL algorithm (from the <algorithm> library) is a little tricky, but simple uses are easy. For example, we can
     *iterate through the characters in a string, modifying them in some way, and returning the modified characters back
     *to their original positions. In this case, we set the result iterator to specify the beginning of the string.
     *A common application is to convert a string to upper or lower case.
     *string str22 = "This IS a MiXed CaSE stRINg";
     *transform (str22.begin(),str22.end(), str22.begin(), tolower);
     *cout << "[" << str22 << "]" << endl; // [this is a mixed case string]
     *Note that the result iterator must specify a destination that is large enough to accept all the modified values;
     *here it is not a problem, since we're putting them back in the same positions.
     *The tolower function (along with toupper, isdigit, and other useful stuff) is in the <cctype> library.
     */
    //DO WE NEED THIS?
    //std::transform(sAddress.begin(), sAddress.end(), sAddress.begin(), tolower);

    std::string::size_type colonPos;
    if( (colonPos = sAddress.find(":", 0)) == std::string::npos )
        return NULL;
    
    std::string sHostName = sAddress.substr(0, colonPos);
    if(sHostName.compare("*") != 0)
    {
    //___________________________<RESOLVING>_______________________________
    char *ptr, **pptr;
    struct hostent *hptr;
    char szIP[INET_ADDRSTRLEN];

    if((hptr = gethostbyname(sHostName.c_str())) == NULL)
    {
        dbg::warning() << "gethostbyname() error for host: " << sHostName << ", try with IP address!\n";
    }
    else
    {
#ifndef NDEBUG
        for(pptr = hptr->h_aliases; *pptr != NULL; pptr++)
            dbg::debug() << "alias: " << *pptr << "\n";
#endif //NDEBUG
        switch (hptr->h_addrtype)
        {
            case AF_INET:
                pptr = hptr->h_addr_list;
                if(inet_ntop(hptr->h_addrtype, *pptr, szIP, sizeof(szIP)) <= 0)
                {
                    dbg::warning() << "gethostbyname() error for host: " << sHostName << ", try with IP address!\n";
                }

                dbg::debug() << "\taddress: " << szIP << "\n";

            break;

            case AF_INET6:
            default:
                dbg::error() << "Unknown address type! Do not support IP6 yet...\n";
            break;
        }
    }

    sHostName = szIP;
    //____________________________</RESOLVING>_________________________________________
    }

    std::stringstream ssPort(sAddress.substr(colonPos+1));
    unsigned short int iPort;
    ssPort >> iPort;

    return new  CAddress(sHostName, iPort);
}

bool
net::CAddress::toSocketAddress(struct sockaddr_in& out) const
{
    out.sin_family = AF_INET;
    out.sin_port = htons(m_uiPort);

    if(m_sHost.compare("*") == 0)
    {
        out.sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        if(inet_pton(AF_INET, m_sHost.c_str(), (void *)&(out.sin_addr)) <= 0)
            return false;
    }

    return true;
}

net::CAddress*
net::CAddress::parse(const struct sockaddr_in& socketAddress)
{
    if(socketAddress.sin_family != AF_INET)
    {
        dbg::error() << "Unsupported socket address type\n";
        return NULL;
    }

    std::string sHostName;
    if(socketAddress.sin_addr.s_addr == INADDR_ANY)
    {
        sHostName = "*";
    }
    else
    {
        // that is the buffer used from inet_ntop() to write the IP address in presentation format: xxx.yyy.zzz.qqq
        char szIP4Address[INET_ADDRSTRLEN];

        if(NULL == inet_ntop(AF_INET, (void *)&socketAddress.sin_addr, szIP4Address, sizeof(szIP4Address)))
        {
            return NULL;
        }

        sHostName = szIP4Address;
    }

    unsigned short int iPort = ntohs(socketAddress.sin_port);

    return new  CAddress(sHostName, iPort);
}

