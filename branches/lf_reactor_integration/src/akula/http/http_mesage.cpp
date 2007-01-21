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

#include "http_message.h"
#include <algorithm>
#include <sstream>
#include <ctime> /*for time() and gmtime()*/
#include <cstring> /*for strerror()*/
#include <cassert> /*for assert()*/
#include <cerrno> /*for errno*/
#include <iomanip>

const std::string http::CHttpMessage::HTTP_PROTOCOL = "HTTP/1.1";

const std::string http::CHttpMessage::MESSAGE_CONTINUE = "Continue";
const std::string http::CHttpMessage::MESSAGE_OK = "OK";
const std::string http::CHttpMessage::MESSAGE_MULTIPLE_CHOICES = "Multiple Choises";
const std::string http::CHttpMessage::MESSAGE_BAD_REQUEST = "Bad Request";
const std::string http::CHttpMessage::MESSAGE_NOT_FOUND = "Not Found";
const std::string http::CHttpMessage::MESSAGE_METHOD_NOT_ALLOWED = "Method Not Allowed";

const std::string http::CHttpMessage::METHOD_OPTIONS = "OPTIONS";
const std::string http::CHttpMessage::METHOD_GET = "GET";
const std::string http::CHttpMessage::METHOD_HEAD = "HEAD";
const std::string http::CHttpMessage::METHOD_POST = "POST";
const std::string http::CHttpMessage::METHOD_PUT = "PUT";
const std::string http::CHttpMessage::METHOD_DELETE = "DELETE";
const std::string http::CHttpMessage::METHOD_TRACE = "TRACE";
const std::string http::CHttpMessage::METHOD_CONNECT = "CONNECT";

const char http::CHttpMessage::SP = ' ';
const char http::CHttpMessage::COLON = ':';
const char http::CHttpMessage::CR = '\r';
const char http::CHttpMessage::LF = '\n';
const char http::CHttpMessage::COMMA = ',';
const std::string http::CHttpMessage::CRLF = "\r\n";
const std::string http::CHttpMessage::DELIMITER = "\r\n\r\n";

const std::string http::CHttpMessage::HEADER_CONTENT_TYPE = "Content-Type";
const std::string http::CHttpMessage::HEADER_CONTENT_LENGTH = "Content-Length";
const std::string http::CHttpMessage::HEADER_HOST = "Host";
const std::string http::CHttpMessage::HEADER_DATE = "Date";

const char* http::CHttpMessage::HTTP_METHODS_STRING[] = {
        "NOT_VALID",
        "OPTIONS",
        "GET",
        "HEAD",
        "POST",
        "PUT",
        "DELETE",
        "TRACE",
        "CONECT"
};

const http::CHttpMessage::NameToContentTypePair_t  http::CHttpMessage::NAME_TO_CONTENTTYPE_RANGE[] = {
    NameToContentTypePair_t("gif", "image/gif"),
    NameToContentTypePair_t("jpeg", "image/jpeg"),
    NameToContentTypePair_t("jpg", "image/jpeg"),
    NameToContentTypePair_t("html", "text/html"),
    NameToContentTypePair_t("pdf", "application/pdf")
};

const http::CHttpMessage::NameToContentTypeMap_t
http::CHttpMessage::NAME_TO_CONTENTTYPE_CONTAINER(
    http::CHttpMessage::NAME_TO_CONTENTTYPE_RANGE,
    http::CHttpMessage::NAME_TO_CONTENTTYPE_RANGE + (sizeof(http::CHttpMessage::NAME_TO_CONTENTTYPE_RANGE) / sizeof(http::CHttpMessage::NAME_TO_CONTENTTYPE_RANGE[0]))
);

http::CHttpMessage::CHttpMessage()
{

}

http::CHttpMessage::~CHttpMessage()
{
    for(headers_t::iterator IT = m_Headers.begin(); IT < m_Headers.end(); ++ IT)
    {
        delete *IT;
    }
}


/**
 Returns constant corresponding to the given string HTTP method
*/
http::CHttpMessage::EHttpMethods_t
http::CHttpMessage::methodString2Value(const std::string& sMethod)
{
    const char** pMethodsEnd = HTTP_METHODS_STRING +
                                            sizeof(HTTP_METHODS_STRING)/sizeof(HTTP_METHODS_STRING[0]);

    const char** IT = std::find(HTTP_METHODS_STRING, pMethodsEnd, sMethod);

    if(IT == pMethodsEnd)
    {
        return NOT_VALID;
    }

    return CHttpMessage::EHttpMethods_t(NOT_VALID + (IT - HTTP_METHODS_STRING));
}

std::string
http::CHttpMessage::methodValue2String(http::CHttpMessage::EHttpMethods_t method)
{
    return (HTTP_METHODS_STRING[method]);
}

/**
 * Get header's value corresponding to the header identified by sName
 */
std::string
http::CHttpMessage::getHeader(const std::string& sName)
{
    headers_t::iterator IT = std::find_if(m_Headers.begin(), m_Headers.end(), CIsThisHeader(sName));

    return (IT==m_Headers.end()) ? "" : (*IT)->getValue();
}

/**
 * Add header <sName : sValue>
 */
void
http::CHttpMessage::addHeader(CHeader* pHeader)
{
    m_Headers.push_back(pHeader);
}

/**
 * Headers to string
 */
void
http::CHttpMessage::headersToString(std::stringstream& out)
{
    headers_t::iterator IT;

    for(IT = m_Headers.begin(); IT < m_Headers.end(); IT++)
    {
        out << (*IT)->getName() << CHttpMessage::COLON << (*IT)->getValue();
        out << CHttpMessage::CRLF;
    }
}

/**
 * HTTP/1.1 defines the sequence CR LF as the end-of-line marker for all protocol elements
 * except the entity-body (see appendix 19.3 for tolerant applications). 
 * The end-of-line marker within an entity-body is defined by its associated media type, as described in section 3.7.
 *       CRLF           = CR LF
 */
std::string::const_iterator
http::CHttpMessage::getline(std::string::const_iterator from, std::string::const_iterator to, std::string& line)
{
    for(std::string::const_iterator IT = from; IT != to; IT++)
    {
        if(*IT == CHttpMessage::CR)
        {
            std::string::const_iterator endOfLine = IT;
            if(++IT != to && *IT == CHttpMessage::LF)
            {
                IT++;
                line = std::string(from, endOfLine);
                return IT;
            }
        }
    }

    line = std::string(from, to);
    return from;
}

//TEST-----------------------------------------------------------------
/**Return the last position of the line*/
char*
http::CHttpMessage::getline(char* from, char* to)
{
    for(char* IT = from; IT != to; IT++)
    {
        if(*IT == CHttpMessage::CR && ++IT != to && *IT == CHttpMessage::LF)
                return IT;
    }

    return NULL; //not found CRLF
}
//TEST-----------------------------------------------------------------

/**
 * HTTP-date    = rfc1123-date | rfc850-date | asctime-date
 * rfc1123-date = wkday "," SP date1 SP time SP "GMT"
 * rfc850-date  = weekday "," SP date2 SP time SP "GMT"
 * asctime-date = wkday SP date3 SP time SP 4DIGIT
 * date1        = 2DIGIT SP month SP 4DIGIT
 *                ; day month year (e.g., 02 Jun 1982)
 * date2        = 2DIGIT "-" month "-" 2DIGIT
 *                ; day-month-year (e.g., 02-Jun-82)
 * date3        = month SP ( 2DIGIT | ( SP 1DIGIT ))
 *                ; month day (e.g., Jun 2)
 * time         = 2DIGIT ":" 2DIGIT ":" 2DIGIT
 *                ; 00:00:00 - 23:59:59
 * wkday        = "Mon" | "Tue" | "Wed"
 *              | "Thu" | "Fri" | "Sat" | "Sun"
 * weekday      = "Monday" | "Tuesday" | "Wednesday"
 *              | "Thursday" | "Friday" | "Saturday" | "Sunday"
 * month        = "Jan" | "Feb" | "Mar" | "Apr"
 *              | "May" | "Jun" | "Jul" | "Aug"
 *              | "Sep" | "Oct" | "Nov" | "Dec"
 */
bool
http::CHttpMessage::rfc1123Date(std::string& sDate)
{
    const char* wkday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char* month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    time_t t = time(NULL);
    if(t == (time_t)-1)
    {
        return false;
    }
    
    struct tm* ptm = gmtime(&t);
    if(ptm == NULL)
    {
        return false;
    }

    assert(ptm->tm_wday >= 0 && ptm->tm_wday <= 6);
    assert(ptm->tm_mon >= 0 && ptm->tm_mon <= 11);

    std::ostringstream oss;
    
    oss << wkday[ptm->tm_wday] << COMMA << SP; //wkday
    //date1
    oss << std::setfill('0') << std::setw(2) << ptm->tm_mday << SP;
    oss << month[ptm->tm_mon] << SP;
    oss << std::setw(4) << 1900+ptm->tm_year << SP;
    //time
    oss << std::setfill('0') << std::setw(2) << ptm->tm_hour << COLON;
    oss << std::setfill('0') << std::setw(2) << ptm->tm_min << COLON;
    oss << std::setfill('0') << std::setw(2) << ptm->tm_sec << SP;
    oss << "GMT";

    sDate = oss.str();
    return true;
}

std::string
http::CHttpMessage::guessContentTypeFromName(const std::string& sName)
{
    // find the file extension
    std::string::size_type extensionPos = sName.find_last_of('.');

    if(extensionPos != std::string::npos)
    {
        std::string sExtension = sName.substr(extensionPos+1);

        NameToContentTypeMapIt_t IT = NAME_TO_CONTENTTYPE_CONTAINER.find(sExtension);

        return (IT != NAME_TO_CONTENTTYPE_CONTAINER.end()) ? IT->second : "application/octet-stream";
    }

    return "application/octet-stream";
}

http::CHttpMessage::CHeader*
http::CHttpMessage::toHeader(char* from, char* to)
{
    CHeader* pHeader = NULL;
    
    char* colon = std::find_if(from, to, utils::CIsColon());
    if(colon != to)
    {
        // I use this constructor for performance reasons - to minimize the string copying and 
        // memory allocation as possible. 
        // The trimming is done after constructing the strings, i.e. in the CHeader constructor
        pHeader = new CHeader(from, colon - from,
                                             colon+1, to - colon);
    }

    return pHeader;
}

//Not tested!!!
http::CHttpMessage::CHeader*
http::CHttpMessage::toHeader(const std::string& sHeaderLine)
{
    CHeader* pHeader = NULL;
    
    std::string::const_iterator colon  = std::find_if(sHeaderLine.begin(), sHeaderLine.end(), utils::CIsColon());
    if(colon != sHeaderLine.end())
    {
        std::string sName = std::string(sHeaderLine.begin(), colon);
        std::string sValue = std::string(colon+1, sHeaderLine.end());
        //trimming should trim the EOF characters (\r\n)
        utils::CTrimmer<>::trim(sName);
        utils::CTrimmer<>::trim(sValue);

        pHeader = new CHeader(sName, sValue);
    }

    return pHeader;
}


