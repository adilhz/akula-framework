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

#include "http_request.h"
#include "http_requestline.h"
#include "http_exceptions.h"
#include <sstream>


/**
* Not tested!!!
*
* This ctor can be used to parse the request, represented as a string
* @param sRequest String representation of the request
* @throws CHttpException if the input data sRequest doesn't present valid request
*/
http::CHttpRequest::CHttpRequest(const std::string& sRequest) throw(http::CHttpException)
{
    state = PARSE_REQUESTLINE;
    std::string::const_iterator position = sRequest.begin();
    
    //request line
    std::string sRequestLine;
    position = getline(position, sRequest.end(), sRequestLine);
    m_pRequestLine = CRequestLine::parse(sRequestLine);
    if(m_pRequestLine == NULL)
    {
        throw http::CBadRequestHttpException();
    }

    //headers
    state = PARSE_HEADERS;
    std::string sHeader;
    while(true)
    {
        position = getline(position, sRequest.end(), sHeader);
        if(sHeader.empty()) //the "empty" line between headers and body
        {
            break;
        }

        CHeader* pHeader = toHeader(sHeader);
        if(pHeader == NULL)
        {
            throw http::CBadRequestHttpException();
        }
        
        addHeader(pHeader);
    }

    state = PARSED;
}

http::CHttpRequest::CHttpRequest()
    :m_pRequestLine(NULL)
{
    state = PARSE_REQUESTLINE;
}

http::CHttpRequest::~CHttpRequest()
{
    delete m_pRequestLine;
}

/**
* This method parses the input data (presented as CDataBlock) and updates the parse state
* of the request. The idea of this method is parsing of the received data on-the-fly w/o any copying
* moving of data, etc. I.e. imagine you have an empty request (with no data yet) and invoking this 
* method you are filing it. The parsed state is returned back to you with the following return values:
* -1 - parse error, the input data is not a valid request. It's up to you how to proceed
* 0 - part or all of the data is good and consumed, but it's not enough to build the entire request, waiting 
* for more input data
* 1 - good, the request is ready
*
* Important NOTE is that this method updates the input data block according to the consumed data.
* I.e. in the buffer remains only the data that is not consumed and the read and write positions are OK.
* For example when you receive 0 that means in the data block is remained only the non-parsed data 
* and you can continue to receive the new data. Same is for return value 1 - you can try to parse for the
* next request in the data block
*
* Thus CDataBlock is the data passing unit between the different components avoiding data copying.
* Yes, it seems to be more error-prone every good thing costs something else. You can easy use the
* ctor CHttpRequest(const std::string&) to parse the request at once.
*/
int
http::CHttpRequest::parse(http::CDataBlock* pDataBlock)
{
    if(state == PARSED)
    {
        return 1;
    }
    
    while(1)
    {
        char* pEOL = getline(pDataBlock->getReadPosition(), pDataBlock->getWritePosition());
        
        if(pEOL == NULL) // EOL not found, i.e. we still haven't full header
        {
            //set the remaining data in the beginning of the buffer and update the read/write positions
            //accordingly, i.e. remove the consumed data (if nothing is consumed this call will not do anything)
            pDataBlock->update(pDataBlock->getReadPosition(), pDataBlock->getWritePosition());
            return 0; //cannot parse, continue receiving data from the current position
        }
        else
        {
            switch(state)
            {
                case PARSE_REQUESTLINE:
                    m_pRequestLine = CRequestLine::parse(pDataBlock->getReadPosition(), pEOL);
                    //in case of parse error we return -1 and it is up to the user how to proceed
                    //it should be best if he sent 4xx back to the client, but the question is what to do 
                    //with the information left in the buffer. Since the chances to parse it remain very
                    //low we give the user chance to try to parse it or delete it after sending 4xx
                    // Will it be better if we delete the information after parsing failure???
                    if(m_pRequestLine == NULL)
                    {
                        return -1;
                    }

                    state = PARSE_HEADERS;
                break;
                case PARSE_HEADERS:
                {
                    if(*(pDataBlock->getReadPosition())== '\r' && *pEOL == '\n') //delimiter reached
                    {
                        state = PARSED;
                        //update the positions to try to parse the data left in the buffer after the parsed request
                        pDataBlock->setReadPosition(pEOL+1);
                        return 1;
                    }
                        
                    CHeader* pHeader = toHeader(pDataBlock->getReadPosition(), pEOL);

                    //cannot parse it :(, see the previous (long) comment
                    if(pHeader == NULL)
                    {
                        return -1;
                    }
                    addHeader(pHeader);
                }
                break;
                case PARSED:
                    return 1;
                break;

                default:
                    //shouldn't reach here!!!
                break;
            }

            pDataBlock->setReadPosition(pEOL+1);
        }
    }
}
    
std::string
http::CHttpRequest::startLine()
{
    return m_pRequestLine->toString();
}

http::CRequestLine*
http::CHttpRequest::getRequestLine() const
{
    return m_pRequestLine;
}

