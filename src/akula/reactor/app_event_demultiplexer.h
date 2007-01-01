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

#ifndef APP_EVENT_DEMULTIPLXER_H
#define APP_EVENT_DEMULTIPLXER_H

#include "reactor_utils.h"

namespace net {class CSocket;}

namespace reactor
{
    /*Engines design pattern to prevent virtual functions, see class OsEventDemultiplexer*/
    template<class T_engine>
    class AppEventDemultiplexer
    {
     private:
        T_engine m_engine;
        
     public:
        void register_socket(net::CSocket* pSocket, const unsigned long ulEvents, CReactorUtils::IEventHandler* pCallback)
        {
            m_engine.register_socket(pSocket, ulEvents, pCallback);
        }
        
        void unregister_socket(net::CSocket* pSocket, unsigned long ulEvents)
        {
            m_engine.unregister_socket(pSocket, ulEvents);
        }

        void deactivate_socket(net::CSocket* pSocket)
        {
            m_engine.deactivate_socket(pSocket);
        }
        
        void reactivate_socket(net::CSocket* pSocket)
        {
            m_engine.reactivate_socket(pSocket);
        }

        int handle_events(void)
        {
            return m_engine.handle_events();
        }
    };
}/*namespace reactor*/
#endif /*APP_EVENT_DEMULTIPLXER_H*/

