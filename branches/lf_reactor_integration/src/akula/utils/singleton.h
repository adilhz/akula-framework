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
#ifndef SINGLETON_H
#define SINGLETON_H

#include "thread_mutex.h"
#include "guard.h"

namespace utils
{
    
    //NOT THREAD_SAFE!!!
    template <class TYPE>
    class Singleton
    {
    public:
        static TYPE* getInstance(void)
        {
            if(m_pInstance == 0)
            {
                m_pInstance = new TYPE;
            }
            
            return m_pInstance;
        }
    private:
        static TYPE* m_pInstance;
    };

    /** @brief Thread-safe singleton pattern
    * Based on double-checked locking optimization to reduce the overhead of excessive locking.
    * Here is the pseudo code for this locking optimization:
    *
    * if(first_time_in_flag is FALSE) {
    *   acqire the mutex
    *   //Perform double check to avoid race conditions
    *   if(first_time_in_flag is FALSE) {
    *       execute the critical section
    *       set first_time_in_flag to TRUE
    *   }
    *   relese the mutex
    * }
    *
    * In our case the first_time_in_flag is the address of the instantiated singleton object.
    * To work this model we have to be sure that that the access to the first_time_in_flag, in our
    * case m_pInstance, are atomic. If the write to memory after the call to new is not atomic, other
    * threads may try to read an invalid pointer, which can result in sporadic illegal memory access.
    * Thanks to "Pattern-oriented software architecture, v. 2 by D. Schmidt"
    */
    template<class TYPE>
    class ThreadSafeSingleton
    {
     public:
        static TYPE* getInstance()
        {
            if(m_pInstance == 0)
            {
                Guard<Thread_Mutex> guard(m_SingletonLock);

                if(m_pInstance == 0)
                {
                    m_pInstance = new TYPE;
                }
            }

            return m_pInstance;
        }
        
     private:
        static TYPE* m_pInstance;
        static Thread_Mutex m_SingletonLock;
    };
}//namespace utils

template<class TYPE> TYPE* utils::Singleton<TYPE>::m_pInstance = 0;
template<class TYPE> TYPE* utils::ThreadSafeSingleton<TYPE>::m_pInstance = 0;
template<class TYPE> utils::Thread_Mutex utils::ThreadSafeSingleton<TYPE>::m_SingletonLock;

#endif /*SINGLETON_H*/

