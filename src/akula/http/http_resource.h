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

#ifndef HTTP_RESOURCE_H
#define HTTP_RESOURCE_H

#include <string>

namespace http
{
    class CHttpRequest;
    class CHttpRequest;
    class CHttpReply;

    class CResource
    {
    public:
        static http::CHttpReply* performRequest(http::CHttpRequest*);

    private:
        static void getHostAndTarget(http::CHttpRequest*, std::string&, std::string&);
        CResource();
        /*Disallow copying and assignment*/
        CResource(const CResource&);
        void operator=(const CResource&);
    };
}//namespace http

#endif /*HTTP_RESOURCE_H*/
