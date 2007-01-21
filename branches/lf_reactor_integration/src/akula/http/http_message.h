/***************************************************************************
 *   Copyright (C) 2006 by Krasimir Marinov   *
 *   krasimir.vanev@gmail.com  *
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

#ifndef HTTP_MESSAGE_H
#define HTTP_MESSAGE_H

#include <string>
#include <vector>
#include <map>
#include <akula/utils/string_utils.h>

namespace http
{

//TEST---------------------------
class CDataBlock
{
 private:
    ///the buffer
    char* m_pchBuffer;
    ///current read position
    char* m_pchReadPosition;
    ///current write position
    char* m_pchWritePosition;
    ///internal buffer size
    int m_iBufferSize;

 public:
    CDataBlock(int iBufferSize = 8192)
        : m_iBufferSize(iBufferSize)
    {
        m_pchBuffer = new char[m_iBufferSize];
        m_pchReadPosition = m_pchWritePosition = m_pchBuffer;
    }

    ///Create the data block from buffer. The data is copied into the external buffer!!!
    CDataBlock(const char* pchBuffer, int iSize)
    {
        m_pchBuffer = new char[iSize];
        ::memcpy(m_pchBuffer, pchBuffer, iSize);
        m_pchReadPosition = m_pchBuffer;
        m_pchWritePosition = m_pchBuffer + iSize;
    }

    ~CDataBlock()
    {
        delete [] m_pchBuffer;
    }

    char* getStart(void) const { return m_pchBuffer; }

    char* getEnd(void) const {return m_pchBuffer + m_iBufferSize; }

    char* getReadPosition(void) const { return m_pchReadPosition; }
    void setReadPosition(char* pchPosition) {m_pchReadPosition = pchPosition;}

    char* getWritePosition(void) const { return m_pchWritePosition; }
    void setWritePosition(char* pchPosition) {m_pchWritePosition = pchPosition;}

    /**
    * Updates the buffer with the sub-buffer pointed by its start and end positions.
    * Updates the read and write pointers acordingly
    */
    int update(char* start, char* end)
    {
        if(end < start)
            return -1; //failure
            
        if(getStart() == start)
        {
            if(getWritePosition() != end) //set only the write position
            {
                setWritePosition(end);
            }
        }
        else
        {
            size_t n = end - start;
            ::memmove(getStart(), start, n);
            setWritePosition(getStart() + n);
            setReadPosition(getStart());
        }
    }
};
//TEST---------------------------

 class CHttpMessage
 {
  public:
     static const std::string HTTP_PROTOCOL;
 
     static const std::string MESSAGE_CONTINUE;
     static const std::string MESSAGE_OK;
     static const std::string MESSAGE_MULTIPLE_CHOICES;
     static const std::string MESSAGE_BAD_REQUEST;
     static const std::string MESSAGE_NOT_FOUND;
     static const std::string MESSAGE_METHOD_NOT_ALLOWED;
 
     static const std::string METHOD_OPTIONS;
     static const std::string METHOD_GET;
     static const std::string METHOD_HEAD;
     static const std::string METHOD_POST;
     static const std::string METHOD_PUT;
     static const std::string METHOD_DELETE;
     static const std::string METHOD_TRACE;
     static const std::string METHOD_CONNECT;

    static const char CR;
    static const char LF;
    static const char SP;
    static const char COLON;
    static const char COMMA;
    static const std::string CRLF;
    static const std::string DELIMITER;
 
     static const std::string HEADER_CONTENT_TYPE;
     static const std::string HEADER_CONTENT_LENGTH;
     static const std::string HEADER_HOST;
     static const std::string HEADER_DATE;
     
     typedef enum {
        NOT_VALID,
        OPTIONS,
        GET,
        HEAD,
        POST,
        PUT,
        DELETE,
        TRACE,
        CONNECT
    } EHttpMethods_t;

     static const char* HTTP_METHODS_STRING[];

    typedef std::pair<std::string, std::string> NameToContentTypePair_t;
    typedef std::map<std::string, std::string> NameToContentTypeMap_t;
    typedef NameToContentTypeMap_t::const_iterator NameToContentTypeMapIt_t;
    static const NameToContentTypePair_t NAME_TO_CONTENTTYPE_RANGE[];
    static const NameToContentTypeMap_t NAME_TO_CONTENTTYPE_CONTAINER;
    
    class CHeader
    {
     private:
        std::string m_sName;
        std::string m_sValue;

     public:

        /**The returned value will exist untill this header is existing!!!*/
        std::string& getName(void) {return m_sName;}
        std::string& getValue(void) {return m_sValue;}

        CHeader(std::string& sName, std::string& sValue)
            :m_sName(sName),
            m_sValue(sValue)
        {
            utils::CTrimmer<>::trim(m_sName);
            utils::CTrimmer<>::trim(m_sValue);
        }

        CHeader(const std::string& sName, const std::string& sValue)
            :m_sName(sName),
            m_sValue(sValue)
        {
        }

        CHeader(char* pchName, int iNameLength, char* pchValue, int iValueLength)
            :m_sName(pchName, iNameLength),
            m_sValue(pchValue, iValueLength)
        {
            utils::CTrimmer<>::trim(m_sName);
            utils::CTrimmer<>::trim(m_sValue);
        }
    };
    
     typedef std::vector<CHeader*> headers_t;
     
     headers_t m_Headers;
     
     static EHttpMethods_t methodString2Value(const std::string&);
     static std::string methodValue2String(EHttpMethods_t);
 
     /**
     * Predicate used for finding headers from the vector containing all the headers
     */
     class CIsThisHeader : public std::unary_function<CHttpMessage::CHeader*, bool>
     {
     private:
       std::string mStr;
     public:
         CIsThisHeader(const std::string& sHeader) : mStr(sHeader) {}
         inline bool operator()(CHttpMessage::CHeader* pHeader) const
         {
             return (mStr.compare(pHeader->getName()) == 0);
         }
     };

    CHttpMessage();
    virtual ~CHttpMessage();
    
     virtual std::string startLine() = 0;
     std::string getHeader(const std::string&);
     void addHeader(CHeader*);
     void headersToString(std::stringstream& out);
     
     static std::string guessContentTypeFromName(const std::string& sName);
     static bool rfc1123Date(std::string& sDate);

 protected:
    std::string::const_iterator getline(std::string::const_iterator from, std::string::const_iterator to, std::string& line);
    char* getline(char* from, char* to);
    CHeader* toHeader(char* from, char* to);
    CHeader* toHeader(const std::string&);


 };

}//namespace http

 #endif /*HTTP_MESSAGE_H*/
 
