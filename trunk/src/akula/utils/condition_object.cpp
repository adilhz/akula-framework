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

#include "condition_object.h"
#include "thread_mutex.h"

utils::CConditionObject::CConditionObject(utils::Thread_Mutex& mutex) :
    m_Mutex(mutex)
{
    ::pthread_cond_init(&m_Condition, NULL);
}

utils::CConditionObject::~CConditionObject(void)
{
    ::pthread_cond_destroy(&m_Condition);
}

int
utils::CConditionObject::wait(void)
{
    return ::pthread_cond_wait(&m_Condition, &m_Mutex.m_Mutex);
}

int
utils::CConditionObject::signal(void)
{
    return ::pthread_cond_signal(&m_Condition);
}

