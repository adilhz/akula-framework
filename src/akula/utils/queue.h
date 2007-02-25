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
#ifndef AIM_QUEUE_H
#define AIM_QUEUE_H

#include <queue>

namespace utils
{
    template< class T, class T_engine = std::queue<T> >
    class CQueue
    {
    protected:
        T_engine m_engine;

    public:
        virtual bool isEmpty(void)
        {
            return m_engine.empty();
        }

        /// Returns and *removes* element from the queue
        virtual T get(void)
        {
            T element = m_engine.front();
            m_engine.pop();
            return element;
        }

        virtual void put(T element)
        {
            m_engine.push(element);
        }
    };
}//namespace utils
#endif /*AIM_QUEUE_H*/

