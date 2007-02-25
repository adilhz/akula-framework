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
#ifndef HTTP_REQUESTLINE_H
#define HTTP_REQUESTLINE_H

#include "http_message.h"
#include "http_exceptions.h"
#include <string>

namespace http
{
    class CRequestLine
    {
     private:
        http::CHttpMessage::EHttpMethods_t m_Method;
        std::string m_sURI;
        std::string m_sVersion;

     public:
        http::CHttpMessage::EHttpMethods_t getMethod() const;
        std::string getURI() const;
        std::string getVersion() const;

        static http::CRequestLine* parse(const std::string& sRequestLine);
        static http::CRequestLine* parse(char* from, char* to);
        CRequestLine(const http::CHttpMessage::EHttpMethods_t method, const std::string& sURI, const std::string& sVersion);

        std::string toString();
    };
}//namespace http

#endif /*HTTP_REQUESTLINE_H*/

