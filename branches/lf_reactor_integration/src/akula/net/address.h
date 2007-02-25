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

#ifndef ADDRESS_H
#define ADDRESS_H

#include <string>
#include "exceptions.h"
#include <akula/dbg/dbg.h>
#include <sstream>
#include <netdb.h>          /* hostent struct, gethostbyname() */
#include <arpa/inet.h>      /* inet_ntoa() to format IP address */
#include <netinet/in.h>     /* in_addr structure */
#include <cerrno>

namespace net
{
    /**
     * @class CAddress
     * @brief Representation of the address of the server/client
     */
    class CAddress
    {
     public:
        /**
         * Creates CAddress instance
         * @param std::string The address in the form "address:port"
         * @return pointer to the CAddress instance or NULL if input cannot be parsed
         */
        static CAddress* parse(const std::string& sAddress)
        {
            //DO WE NEED THIS?
            //std::transform(sAddress.begin(), sAddress.end(), sAddress.begin(), tolower);

            std::string::size_type colonPos;
            if( (colonPos = sAddress.find(":", 0)) == std::string::npos )
                return NULL;
            
            std::string sHostName = sAddress.substr(0, colonPos);
            if(sHostName.compare("*") != 0)
            {
            //___________________________<RESOLVING>_______________________________
            //TODO: Use all resolved addresses, implement resolving module!
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
        
        /**
         * Creates CAddress instance
         * @param sockaddr_in Socket address
         * @return pointer to the CAddress instance or NULL if input cannot be parsed
         */
        static CAddress* parse(const struct sockaddr_in& socketAddress)
        {
            if(socketAddress.sin_family != AF_INET)
                throw CNetException("Unsopported address family");

            std::string sHostName;
            if(socketAddress.sin_addr.s_addr == INADDR_ANY)
            {
                sHostName = "*";
            }
            else
            {
                char szIP4Address[INET_ADDRSTRLEN];

                if(NULL == ::inet_ntop(AF_INET, (void *)&socketAddress.sin_addr, szIP4Address, sizeof(szIP4Address)))
                    throw CNetException(strerror(errno));

                sHostName = szIP4Address;
            }

            unsigned short int iPort = ntohs(socketAddress.sin_port);

            return new  CAddress(sHostName, iPort);
        }

        std::string toString() const
        {
            std::stringstream ss;
            ss << m_sHost << ":" << m_uiPort;
            return ss.str();
        }

        void toSocketAddress(struct sockaddr_in& out) const
        {
            out.sin_family = AF_INET;
            out.sin_port = htons(m_uiPort);
            
            if(m_sHost.compare("*") == 0)
            {
                out.sin_addr.s_addr = INADDR_ANY;
            }
            else
            {
                int rv = ::inet_pton(AF_INET, m_sHost.c_str(), (void *)&(out.sin_addr));
                if(rv < 0)
                    throw CNetException(strerror(errno));
                else if(rv == 0)
                    throw CNetException("Found not valid character string for this address family");
            }
        }


        std::string getHost() const { return m_sHost; }
        unsigned short getPort() const { return m_uiPort; }

        ~CAddress() {}
        
        // we will use default copy constructor and assignment operator, since there is no
        // pointer members
        
     private:
        CAddress();

        CAddress(const std::string& sHost, const unsigned short int& iPort)
            :m_sHost(sHost),
            m_uiPort(iPort)
         {
         }

     private:
        /**The port*/
        unsigned short int m_uiPort;

        /**The host*/
        std::string m_sHost;
    };

} /*namespace net*/
#endif // ADDRESS_H

