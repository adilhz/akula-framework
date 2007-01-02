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
        CTimeVal(bool bNow = true);
        CTimeVal(time_t);
        CTimeVal(time_t, suseconds_t);
        CTimeVal(const struct timeval&);
        CTimeVal(const CTimeVal&);
        ~CTimeVal() {}

        CTimeVal& operator=(const CTimeVal&);

        friend CTimeVal operator+(const CTimeVal& lhs, const CTimeVal& rhs);
        friend CTimeVal operator-(const CTimeVal& lhs, const CTimeVal& rhs);
        friend bool operator==(const CTimeVal& lhs, const CTimeVal& rhs);
        friend bool operator>(const CTimeVal& lhs, const CTimeVal& rhs);
        friend bool operator<(const CTimeVal& lhs, const CTimeVal& rhs);
     private:
        struct timeval m_timeval;
    };

}//namespace utils

#endif // TIMEVAL_H


