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

#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "http_message.h"
#include "http_exceptions.h"
#include <string>

namespace http
{
    class CRequestLine;
    class CDataBlock;

    class CHttpRequest : public http::CHttpMessage
    {
    private:
        http::CRequestLine* m_pRequestLine;
        enum parsing_state {PARSE_REQUESTLINE = 0, PARSE_HEADERS, PARSED};

        enum parsing_state state;

     public:
        CHttpRequest();
        CHttpRequest(const std::string&) throw(http::CHttpException);
        ~CHttpRequest();

        int parse(CDataBlock* pDataBlock);
        
        std::string startLine();
        http::CRequestLine* getRequestLine() const;
    };
}//namespace http
#endif /*HTTP_REQUEST_H*/

