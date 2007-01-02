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
    @file caddress.h 

    @brief Interface for CAddress class
*/


#ifndef ADDRESS_H
#define ADDRESS_H

#include <string>

struct sockaddr_in;

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
        static CAddress* parse(const std::string&);
        /**
         * Creates CAddress instance
         * @param sockaddr_in Socket address
         * @return pointer to the CAddress instance or NULL if input cannot be parsed
         */
        static CAddress* parse(const struct sockaddr_in&);

        std::string toString() const;
        bool toSocketAddress(struct sockaddr_in& out) const;

        std::string getHost() const { return m_sHost; }
        unsigned short getPort() const { return m_uiPort; }

        ~CAddress() {}
        
        // we will use default copy constructor and assignment operator, since there is no
        // pointer members
        
     private:
        CAddress();
        CAddress(const std::string&, const unsigned short int&);

     private:
        /**The port*/
        unsigned short int m_uiPort;

        /**The host*/
        std::string m_sHost;
    };

} /*namespace net*/
#endif // ADDRESS_H

