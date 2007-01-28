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
 * @file CTimeVal
 * @brief class CTimeVal interface
 */
#ifndef TIMEVAL_H
#define TIMEVAL_H

#include <time.h>
#include <sys/time.h>

namespace utils
{

    class CTimeVal
    {
     public:
        CTimeVal(bool bNow = true)
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

        CTimeVal(time_t mseconds)
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


        CTimeVal(time_t seconds, suseconds_t useconds)
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

        CTimeVal(const struct timeval& time)
        {
            m_timeval.tv_sec = time.tv_sec ;
            m_timeval.tv_usec = time.tv_usec ;
        }

        /**Allow copying (copy ctor)*/
        CTimeVal(const CTimeVal& tv)
        {
            m_timeval.tv_sec = tv.m_timeval.tv_sec;
            m_timeval.tv_usec = tv.m_timeval.tv_usec;
        }

        ~CTimeVal() {}

        /**Allow assignment (assignment operator)*/
        CTimeVal& operator=(const CTimeVal& rhs)
        {
            m_timeval.tv_sec = rhs.m_timeval.tv_sec;
            m_timeval.tv_usec = rhs.m_timeval.tv_usec;
            return *this;
        }


        friend CTimeVal operator+(const CTimeVal& lhs, const CTimeVal& rhs)
        {
            return CTimeVal(lhs.m_timeval.tv_sec + rhs.m_timeval.tv_sec,
                                            lhs.m_timeval.tv_usec + rhs.m_timeval.tv_usec );
        }

        friend CTimeVal operator-(const CTimeVal& lhs, const CTimeVal& rhs)
        {
            return CTimeVal(lhs.m_timeval.tv_sec - rhs.m_timeval.tv_sec,
                                            lhs.m_timeval.tv_usec - rhs.m_timeval.tv_usec);
        }

        friend bool operator==(const CTimeVal& lhs, const CTimeVal& rhs)
        {
            return ((lhs.m_timeval.tv_sec == rhs.m_timeval.tv_sec ) &&
                        (lhs.m_timeval.tv_usec == rhs.m_timeval.tv_usec ));
        }

        friend bool operator>(const CTimeVal& lhs, const CTimeVal& rhs)
        {
            return ((lhs.m_timeval.tv_sec > rhs.m_timeval.tv_sec) ||
                        ((lhs.m_timeval.tv_sec == rhs.m_timeval.tv_sec) && (lhs.m_timeval.tv_usec > rhs.m_timeval.tv_usec))) ;
        }

        friend bool operator<(const CTimeVal& lhs, const CTimeVal& rhs)
        {
            return ((lhs.m_timeval.tv_sec < rhs.m_timeval.tv_sec) ||
                        ((lhs.m_timeval.tv_sec == rhs.m_timeval.tv_sec) && (lhs.m_timeval.tv_usec < rhs.m_timeval.tv_usec))) ;
        }

     private:
        struct timeval m_timeval;
    };

}//namespace utils

#endif // TIMEVAL_H


