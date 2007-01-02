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
#ifndef REACTOR_TYPE_H
#define REACTOR_TYPE_H

#include <akula/utils/singleton.h>
#include "os_event_demultiplexer_poll.h"
#include "app_event_demultiplexer.h"
#include "app_event_demultiplexer_map.h"
#include "app_event_demultiplexer_array.h"

namespace reactor
{
    /*
     * Here is the place to combine the parts into a reactor. You can combine different os and application implementations
     * to get composite reactor.
     */
    //typedef AppEventDemultiplexer< AppEventDemultiplexerMapImpl<OsEventDemultiplexerPollImpl> > Reactor_t;
    typedef AppEventDemultiplexer< AppEventDemultiplexerArrayImpl<OsEventDemultiplexerPollImpl> > Reactor_t;


    /**
     * Get the reactor, singleton
     */
    inline reactor::Reactor_t* getReactor()
    {
        return utils::ThreadSafeSingleton<Reactor_t>::getInstance();
    }
}//namespace reactor

#endif /*REACTOR_TYPE_H*/

