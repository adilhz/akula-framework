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

#include "thread_mutex.h"

utils::Thread_Mutex::Thread_Mutex(void)
{
    ::pthread_mutex_init(&m_Mutex, NULL);
}

utils::Thread_Mutex::~Thread_Mutex(void)
{
    ::pthread_mutex_destroy(&m_Mutex);
}

int
utils::Thread_Mutex::acquire(void)
{
    return ::pthread_mutex_lock(&m_Mutex);
}

int
utils::Thread_Mutex::release(void)
{
    return ::pthread_mutex_unlock(&m_Mutex);
}

