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

// forward declarations
namespace net {class CSocket;}

namespace reactor
{
 class CReactorUtils
 {
  public:
     static const unsigned long NO_REGISTERED_EVENTS;
     static const unsigned long READ_MASK;
     static const unsigned long WRITE_MASK;
     static const unsigned long REMOVE_MASK;
 
     static const unsigned long MAX_OPEN_FILES_PER_PROCESS;

    /**
     * @class IEventHandler
     * @brief   This is the logic of the given socket - must be defined from the class
     */
    struct IEventHandler
    {
        virtual long callback(net::CSocket* pSocket, unsigned long ulFlags) = 0;
    };

    class CEventHandlers
    {
     public:
        CEventHandlers(IEventHandler* pRHandler, IEventHandler* pWHandler, net::CSocket* socket);

        ~CEventHandlers();

        CEventHandlers& operator=(const CEventHandlers& rhs);

        IEventHandler* pReadEventHandler;
        IEventHandler* pWriteEventHandler;
        net::CSocket*            pSocket;
    };

  private:
      /*Disallow copying and assignment*/
      CReactorUtils(const CReactorUtils&);
      CReactorUtils& operator=(const CReactorUtils&);
 
 };

}//namespace reactor

#endif /*REACTOR_UTILS_H*/

