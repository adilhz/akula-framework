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

#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include "guard.h"
#include "thread_mutex.h"
#include "queue.h"

namespace utils
{
    template<class T>
    class CThreadSafeQueue : public CQueue<T>
    {
    private:
        Thread_Mutex m_mutex;

    public:
        virtual bool isEmpty(void)
        {
            Guard<Thread_Mutex> guard(m_mutex);
            return CQueue<T>::isEmpty();
        }
        
        virtual T get(void)
        {
            Guard<Thread_Mutex> guard(m_mutex);
            return CQueue<T>::get();
        }

        virtual void put(T element)
        {
            Guard<Thread_Mutex> guard(m_mutex);
            CQueue<T>::put(element);
        }
    };
}//namespace utils

#endif /*THREAD_SAFE_QUEUE_H*/

