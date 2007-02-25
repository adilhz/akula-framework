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

#include "http_requestline.h"
#include <akula/utils/utils.h>
#include <sstream>

http::CHttpMessage::EHttpMethods_t
http::CRequestLine::getMethod() const
{
    return m_Method;
}

std::string
http::CRequestLine::getURI() const
{
    return m_sURI;
}

std::string
http::CRequestLine::getVersion() const
{
    return m_sVersion;
}

http::CRequestLine*
http::CRequestLine::parse(const std::string& sRequestLine)
{
    std::vector<std::string> result;
    utils::CTokenizer<>::tokenize(result, sRequestLine); //default tokenizer (white spaces)

    if(result.size() != 3)
    {
        return NULL;
    }

    return new  CRequestLine(CHttpMessage::methodString2Value(result[0]),
                                            result[1], result[2]);
}

http::CRequestLine*
http::CRequestLine::parse(char* from, char* to)
{
    std::vector<std::string> result;
    utils::CTokenizer<>::tokenize(result, from, to); //default tokenizer (white spaces)

    if(result.size() != 3)
    {
        return NULL;
    }

    return new  CRequestLine(CHttpMessage::methodString2Value(result[0]),
                                            result[1], result[2]);
}


http::CRequestLine::CRequestLine(const http::CHttpMessage::EHttpMethods_t method, const std::string& sURI, const std::string& sVersion) :
    m_Method(method),
    m_sURI(sURI),
    m_sVersion(sVersion)
{

}
    
std::string
http::CRequestLine::toString()
{
    std::ostringstream oss;
    oss << CHttpMessage::methodValue2String(m_Method) << CHttpMessage::SP;
    oss << getURI() << CHttpMessage::SP;
    oss << getVersion();

    return oss.str();
}
