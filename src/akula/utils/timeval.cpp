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

/**
 * @file ctimevalue.cpp
 */

#include "timeval.h"

/**Construct, if bNow is <code>true</code> initialize with the current time, otherwise set to <code>0</code>*/
utils::CTimeVal::CTimeVal(bool bNow)
{
    if(bNow)
    {
        ::gettimeofday (&m_timeval, NULL);
    }
    else
    {
        m_timeval.tv_sec = 0;
        m_timeval.tv_usec = 0;
    }
}

/**Construct from milliseconds*/
utils::CTimeVal::CTimeVal(time_t mseconds )
{
    if(mseconds < 0 )
    {
        m_timeval.tv_sec = 0;
        m_timeval.tv_usec = 0;
    }
    else
    {
        m_timeval.tv_sec = (mseconds / 1000 );
        m_timeval.tv_usec = ((mseconds % 1000) * 1000 );
    }
}

/**Construct from seconds and microseconds*/
utils::CTimeVal::CTimeVal(time_t seconds, suseconds_t useconds)
{
    // If the microseconds value is negative, then "borrow" from the seconds value.
    while(useconds < 0 )
    {
        useconds += 1000000 ;
        seconds-- ;
    }

    // "Normalize" the time so that the microseconds field is less than a million.
    while(useconds >= 1000000 )
    {
        seconds++ ;
        useconds -= 1000000 ;
    }

    if(seconds < 0 )
    {
        m_timeval.tv_sec = 0;
        m_timeval.tv_usec = 0;
    }
    else
    {
        m_timeval.tv_sec = seconds;
        m_timeval.tv_usec = useconds;
    }
}

utils::CTimeVal::CTimeVal(const struct  timeval& time)
{
    m_timeval.tv_sec = time.tv_sec ;
    m_timeval.tv_usec = time.tv_usec ;
}

/**Allow copying (copy ctor)*/
utils::CTimeVal::CTimeVal(const utils::CTimeVal& tv)
{
    m_timeval.tv_sec = tv.m_timeval.tv_sec;
    m_timeval.tv_usec = tv.m_timeval.tv_usec;
}

/**Allow assignment (assignment operator)*/
utils::CTimeVal&
utils::CTimeVal::operator=(const utils::CTimeVal& rhs)
{
    m_timeval.tv_sec = rhs.m_timeval.tv_sec;
    m_timeval.tv_usec = rhs.m_timeval.tv_usec;
    return *this;
}

//add two time values
utils::CTimeVal
utils::operator+(const utils::CTimeVal& lhs, const utils::CTimeVal& rhs)
{
    return CTimeVal(lhs.m_timeval.tv_sec + rhs.m_timeval.tv_sec,
                                    lhs.m_timeval.tv_usec + rhs.m_timeval.tv_usec );
}

// Subtract one time value from another.
utils::CTimeVal
utils::operator-(const utils::CTimeVal& lhs, const  utils::CTimeVal& rhs)
{
    return CTimeVal(lhs.m_timeval.tv_sec - rhs.m_timeval.tv_sec,
                                    lhs.m_timeval.tv_usec - rhs.m_timeval.tv_usec);
}

// T1 == T2?
bool
utils::operator==(const utils::CTimeVal& lhs, const utils::CTimeVal& rhs)
{
    return ((lhs.m_timeval.tv_sec == rhs.m_timeval.tv_sec ) &&
                (lhs.m_timeval.tv_usec == rhs.m_timeval.tv_usec ));
}

// T1 < T2?
bool
utils::operator<(const utils::CTimeVal& lhs, const utils::CTimeVal& rhs)
{
    return ((lhs.m_timeval.tv_sec < rhs.m_timeval.tv_sec) ||
                ((lhs.m_timeval.tv_sec == rhs.m_timeval.tv_sec) && (lhs.m_timeval.tv_usec < rhs.m_timeval.tv_usec))) ;
}

// T1 > T2?
bool
utils::operator>(const utils::CTimeVal& lhs, const utils::CTimeVal& rhs)
{
    return ((lhs.m_timeval.tv_sec > rhs.m_timeval.tv_sec) ||
                ((lhs.m_timeval.tv_sec == rhs.m_timeval.tv_sec) && (lhs.m_timeval.tv_usec > rhs.m_timeval.tv_usec))) ;
}


