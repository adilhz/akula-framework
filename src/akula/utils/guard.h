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

#ifndef GUARD_H
#define GUARD_H

/**
Guard class that implements the C++ Scoped
Locking idiom, which ensures that a Thread Mutex is
properly released regardless of how the program's flow of
control exits a scope
*/

namespace utils
{
    template<class LOCK>
    class Guard
    {
    private:
        ///Hold the lock by reference to avoid use of the copy constructor
        LOCK& m_lock;
        ///Records if the lock is held by this object to prevent double unlocking
        bool m_bOwner;

    public:
        Guard(LOCK& lock)
            : m_lock(lock)
            , m_bOwner(false)
        {
            if(m_lock.acquire() == 0)
                m_bOwner = true;
        }

        ~Guard()
        {
            if(m_bOwner)
                m_lock.release();
        }

        int acquire()
        {
            int r = m_lock.acquire();
            if(r == 0)
                m_bOwner = true;

            return r;
        }

        int release()
        {
            int r = 0;
            if(m_bOwner)
            {
                r = m_lock.release();
                m_bOwner = false;
            }

            return r;
        }
    };
}//namespace utils
#endif /*GUARD_H*/

