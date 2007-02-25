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

#ifndef HTTP_REPLY_H
#define HTTP_REPLY_H

#include "http_message.h"
#include "http_statusline.h"
#include <string>
#include <sstream>


namespace http
{
    class CHttpReply : public http::CHttpMessage
    {
     public:
        std::string startLine(); //Status-Line
        CHttpReply(const int, const std::string&, std::fstream* pFile = NULL);

        /**
         * Emits the request
         * @param buffer The supplied buffer to write the reply bytes
         * @param bufferSize The size of the supplied buffer
         * @return The number of bytes written, <code>0</code> if the entire reply is emitted
         */
        unsigned long emit(char* buffer, unsigned long bufferSize);

    private:
        std::fstream* m_pFileTarget;
        CStatusLine m_statusLine;
        std::stringstream m_ssReply;
        bool m_bReadyForEmit;
    };

}//namespace http
#endif /*HTTP_REPLY_H*/

