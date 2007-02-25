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

#include "http_reply.h"
#include <fstream>

http::CHttpReply::CHttpReply(const int iStatusCode, const std::string& sStatusMessage, std::fstream* pFile)
    : m_statusLine(iStatusCode, sStatusMessage),
    m_pFileTarget(pFile)
{
    m_bReadyForEmit = false;
}

std::string
http::CHttpReply::startLine()
{
    return m_statusLine.toString();
}

unsigned long
http::CHttpReply::emit(char* buffer, unsigned long bufferSize)
{
    unsigned long emited = 0;
    
    if(!m_bReadyForEmit) //prepare Response header
    {
        m_ssReply << startLine() << CHttpMessage::CRLF;
        headersToString(m_ssReply);
        m_ssReply << CHttpMessage::CRLF; //the "delimiter" between headers and body

        // if we have to send a file, position the get pointer to the file beginning
        // this is needed because to get the file size we may have positioned it in the end
        if(m_pFileTarget != NULL)
        {
            m_pFileTarget->seekg(0, std::ios::beg);
        }
        
        m_bReadyForEmit = true;
    }

    if(!m_ssReply.eof()) // emit header
    {
        m_ssReply.read((char *)buffer, bufferSize);
        emited += m_ssReply.gcount();
        buffer += emited;
        bufferSize -= emited;
    }

    if(m_pFileTarget != NULL) // have entity body to emit
    {
        if(!m_pFileTarget->eof())
        {
            m_pFileTarget->read((char *)buffer, bufferSize);
            emited += m_pFileTarget->gcount();
        }
    }

    return emited;
}

