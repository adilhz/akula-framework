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
#ifndef REACTOR_UTILS_H
#define REACTOR_UTILS_H

#include "reactor_utils.h"
#include <akula/net/socket.h>
#include <unistd.h>                     //for getdtablesize()

namespace reactor
{
    class CReactorUtils
    {
     public:
        typedef unsigned long EventType_t;

        enum Events
        {
            NO_EVENTS = 0,
            READ_EVENT = 0x1 << 0,
            WRITE_EVENT = 0x1 << 1,
            TIMER_EVENT =  0x1 << 2
        };

        static unsigned long maxOpenFilesPerProcess(void)
        {
            return ::getdtablesize();
        }

        struct IEventHandler
        {
            virtual bool handle_read(net::CSocket* pSocket)
            {
                return false;
            }
            
            virtual bool handle_write(net::CSocket* pSocket)
            {
                return false;
            }
        };

        struct SHandlerTriple
        {
            IEventHandler* m_phandler;
            net::CSocket* m_psocket;
            EventType_t m_events;

            SHandlerTriple()
                : m_phandler(NULL) ,m_psocket(NULL) ,m_events(NO_EVENTS)
            {}

            SHandlerTriple(IEventHandler* peh, net::CSocket* ps, EventType_t e)
                : m_phandler(peh) ,m_psocket(ps) ,m_events(e)
            {}
        };

     private:
        /*Disallow copying and assignment*/
        CReactorUtils(const CReactorUtils&);
        CReactorUtils& operator=(const CReactorUtils&);
    };
}//namespace reactor

#endif /*REACTOR_UTILS_H*/

