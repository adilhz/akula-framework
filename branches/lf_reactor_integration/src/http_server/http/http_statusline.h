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

#ifndef HTTP_STATUSLINE_H
#define HTTP_STATUSLINE_H

#include <string>

namespace http
{
    class CStatusLine
    {
     private:
        int m_iStatusCode;
        std::string m_sStatusMessage;
     public:
        CStatusLine(const int iStatusCode, const std::string& sStatusMessage);

        std::string toString(void) const;
        int getStatusCode(void) const {return m_iStatusCode;}
        std::string getStatusMessage(void) const {return m_sStatusMessage;}
    };
}//namespace http

#endif /*HTTP_STATUSLINE_H*/

