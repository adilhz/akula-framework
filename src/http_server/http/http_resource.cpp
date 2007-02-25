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

#include "http_resource.h"
#include "http_request.h"
#include "http_message.h"
#include "http_requestline.h"
#include "http_reply.h"
#include <akula/utils/utils.h>
#include <fstream>

/**
 * 5.2 The Resource Identified by a Request
 * The exact resource identified by an Internet request is determined by examining both the Request-URI and the Host header field.
 * An origin server that does not allow resources to differ by the requested host MAY ignore the Host header field value when determining the resource identified by an HTTP/1.1 request. (But see section 19.6.1.1 for other requirements on Host support in HTTP/1.1.)
 * An origin server that does differentiate resources based on the host requested (sometimes referred to as virtual hosts or vanity host names) MUST use the following rules for determining the requested resource on an HTTP/1.1 request:
 * 1. If Request-URI is an absoluteURI, the host is part of the Request-URI. Any Host header field value in the request MUST be ignored.
 * 2. If the Request-URI is not an absoluteURI, and the request includes a Host header field, the host is determined by the Host header field value.
 * 3. If the host as determined by rule 1 or 2 is not a valid host on the server, the response MUST be a 400 (Bad Request) error message.
 * Recipients of an HTTP/1.0 request that lacks a Host header field MAY attempt to use heuristics (e.g., examination of the URI path for something unique to a particular host) in order to determine what exact resource is being requested. 
 */
void
http::CResource::getHostAndTarget(http::CHttpRequest* pRequest, std::string& sHost, std::string& sAbsoluteTargetPath)
{
    std::string sRequestURI = pRequest->getRequestLine()->getURI();

    bool isAbsoluteURI = !sRequestURI.compare(0, 6, "http://");

    if(isAbsoluteURI) // get Host from the absolute URI
    {
        std::string::size_type first_slash_pos = sRequestURI.find_first_of('/', 7);
        sHost = sRequestURI.substr(7, first_slash_pos);
        sAbsoluteTargetPath = sRequestURI.substr(6, std::string::npos);
    }
    else //the RequestURI is absolute path
    {
        sAbsoluteTargetPath = sRequestURI;
        sHost = pRequest->getHeader(CHttpMessage::HEADER_HOST);
    }
}

http::CHttpReply*
http::CResource::performRequest(http::CHttpRequest* pRequest)
{
    CHttpReply* pReply = NULL;
    CHttpMessage::EHttpMethods_t method = pRequest->getRequestLine()->getMethod();
    std::string sHost, sAbsoluteTargetPath;

    getHostAndTarget(pRequest, sHost, sAbsoluteTargetPath);
    
    switch(method)
    {
        case CHttpMessage::OPTIONS:
            pReply = new CHttpReply(405, CHttpMessage::MESSAGE_METHOD_NOT_ALLOWED);
            break;
        case CHttpMessage::GET:
        case CHttpMessage::HEAD:
        /* Web Server Details
         * When a client attempts to fetch a directory (e.g. / or /6.033/www/) from the server, your web server should fetch
         * the index document, index.html, located in that directory. If the document is not found, your server should return an error.
         * For HTTP/0.9, there is no explicit way of signalling errors. Rather, you should send back a small web page spelling out what
         * went wrong.
         * The easiest way of testing your web server is to run Netscape or NCSA Mosaic against it.
         * We found that Mosaic handled the older HTTP/0.9 protocol much better than Netscape did.
         */
        {
            std::fstream* pFile =
                new std::fstream(sAbsoluteTargetPath.c_str(), std::ios::in | std::ios::binary);
            if(pFile->is_open())
            {
                /*
                *The HEAD method is identical to GET except that the server MUST NOT return a message-body in the response.
                *The metainformation contained in the HTTP headers in response to a HEAD request SHOULD be identical to the information sent in response to a GET request.
                */
                if(method == CHttpMessage::GET)
                {
                    pReply = new CHttpReply(200, CHttpMessage::MESSAGE_OK, pFile);
                }
                else
                {
                    pReply = new CHttpReply(200, CHttpMessage::MESSAGE_OK); //no body
                }

                // set Content-Length header
                pFile->seekg(0, std::ios::end);
                unsigned long ulLength = pFile->tellg();
                std::string sLength;
                utils::to_string(ulLength, sLength, std::dec);
                pReply->addHeader(new CHttpMessage::CHeader(CHttpMessage::HEADER_CONTENT_LENGTH, sLength));
                /*Any HTTP/1.1 message containing an entity-body SHOULD include a Content-Type header field defining the
                    media type of that body. If and only if the media type is not given by a Content-Type field, the recipient MAY
                    attempt to guess the media type via inspection of its content and/or the name extension(s) of the URI used to identify
                    the resource. If the media type remains unknown, the recipient SHOULD treat it as type ?application/octet-
                    stream?.
                */
                pReply->addHeader(new CHttpMessage::CHeader (
                                                        CHttpMessage::HEADER_CONTENT_TYPE,
                                                        CHttpMessage::guessContentTypeFromName(sAbsoluteTargetPath)
                                                        ));
            }
            else
            {
                pReply = new CHttpReply(404, CHttpMessage::MESSAGE_NOT_FOUND);
            }
        }
            break;
        case CHttpMessage::POST:
            pReply = new CHttpReply(405, CHttpMessage::MESSAGE_METHOD_NOT_ALLOWED);
            break;
        case CHttpMessage::PUT:
            pReply = new CHttpReply(405, CHttpMessage::MESSAGE_METHOD_NOT_ALLOWED);
            break;
        case CHttpMessage::DELETE:
            pReply = new CHttpReply(405, CHttpMessage::MESSAGE_METHOD_NOT_ALLOWED);
            break;
        case CHttpMessage::TRACE:
            pReply = new CHttpReply(405, CHttpMessage::MESSAGE_METHOD_NOT_ALLOWED);
            break;
        case CHttpMessage::CONNECT:
            pReply = new CHttpReply(405, CHttpMessage::MESSAGE_METHOD_NOT_ALLOWED);
            break;
        default:
            pReply = new CHttpReply(405, CHttpMessage::MESSAGE_METHOD_NOT_ALLOWED);
            break;
    }
    /*
    14.18 Date
    Origin servers MUST include a Date header field in all responses, except in these cases:
    1.    If the response status code is 100 (Continue) or 101 (Switching Protocols), the response MAY include a
          Date header field, at the server?s option.
    2.    If the response status code conveys a server error, e.g. 500 (Internal Server Error) or 503 (Service
          Unavailable), and it is inconvenient or impossible to generate a valid Date.
    3.    If the server does not have a clock that can provide a reasonable approximation of the current time, its
          responses MUST NOT include a Date header field. In this case, the rules in section 14.18.1 MUST be
          followed.
    */
    std::string sDate;
    CHttpMessage::rfc1123Date(sDate);
    pReply->addHeader(new CHttpMessage::CHeader(CHttpMessage::HEADER_DATE, sDate));
    
    return pReply;
}
