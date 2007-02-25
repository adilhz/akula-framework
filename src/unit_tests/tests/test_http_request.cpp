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
#include "TUT/tut.h"
#include "http_server/http/http.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

namespace tut
{
    struct test_http_request_data
    {
    };

    typedef test_group<test_http_request_data> factory;
    typedef factory::object object;

    factory test_http_request("test_http_request");

    template<>
    template<>
    void object::test<1>()
    {
        std::ostringstream one_request;
        one_request << "GET /home/krasimir/Desktop/index.html HTTP/1.1\r\n"
        << "Host: 127.0.0.1\r\n"
        << "User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.7.12) Gecko/20050922 Fedora/1.0.7-1.1.fc4 Firefox/1.0.7\r\n"
        << "Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5\r\n"
        << "Accept-Language: en-us,en;q=0.5\r\n"
        << "Accept-Encoding: gzip,deflate\r\n"
        << "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
        << "Keep-Alive: 300\r\n"
        << "Connection: keep-alive\r\n"
        << "\r\n";

        std::string sRequest = one_request.str();
        http::CDataBlock dataBlock(sRequest.data(), sRequest.size());
        http::CHttpRequest* pRequest = new http::CHttpRequest();
        int iNumberOfRequests = 0;
        while(1)
        {
            int r = pRequest->parse(&dataBlock);
            if(r == 0 || r < 0)
            {
                break;
            }
            else if(r == 1) //parsed request
            {
                iNumberOfRequests++;
                delete pRequest; //delete the parse request
                pRequest = NULL;
                pRequest = new http::CHttpRequest(); //try to parse the next one
            }
        }

        delete pRequest;
        
        ensure("ont http request", iNumberOfRequests == 1);
    }

    template<>
    template<>
    void object::test<2>()
    {
        std::ostringstream two_requests;
        two_requests << "GET /home/krasimir/Desktop/index.html HTTP/1.1\r\n"
        << "Host: 127.0.0.1\r\n"
        << "User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.7.12) Gecko/20050922 Fedora/1.0.7-1.1.fc4 Firefox/1.0.7\r\n"
        << "Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5\r\n"
        << "Accept-Language: en-us,en;q=0.5\r\n"
        << "Accept-Encoding: gzip,deflate\r\n"
        << "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
        << "Keep-Alive: 300\r\n"
        << "Connection: keep-alive\r\n"
        << "\r\n"
        << "GET /home/krasimir/Desktop/index.html HTTP/1.1\r\n"
        << "Host: 127.0.0.1\r\n"
        << "User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.7.12) Gecko/20050922 Fedora/1.0.7-1.1.fc4 Firefox/1.0.7\r\n"
        << "Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5\r\n"
        << "Accept-Language: en-us,en;q=0.5\r\n"
        << "Accept-Encoding: gzip,deflate\r\n"
        << "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
        << "Keep-Alive: 300\r\n"
        << "Connection: keep-alive\r\n"
        << "\r\n";

        std::string sTwoRequests = two_requests.str();
        http::CDataBlock dataBlock(sTwoRequests.data(), sTwoRequests.size());
        http::CHttpRequest* pRequest = new http::CHttpRequest();
        int iNumberOfRequests = 0;
        while(1)
        {
            int r = pRequest->parse(&dataBlock);
            if(r == 0 || r < 0)
            {
                break;
            }
            else if(r == 1) //parsed request
            {
                iNumberOfRequests++;
                delete pRequest;
                pRequest = new http::CHttpRequest();
            }
        }

        delete pRequest;

        ensure("two http requests", iNumberOfRequests == 2);
    }

}
