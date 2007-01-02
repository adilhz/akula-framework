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

#include "dbg.h"
#include <streambuf>
#include <iostream>

#ifdef NDEBUG
    void dbg::setLogLevel(dbg::ELevel) {}
    void dbg::enableLevelPrefix(bool) {}
    void dbg::setLogStream(std::ostream*) {}

    dbg::CNullStream g_nullOStream;
    dbg::CNullStream& dbg::out(dbg::ELevel l) {return g_nullOStream;}

    inline std::ostream& dbg::operator<<(std::ostream& s, const struct dbg::SPosition& pos) {}
#else /*NDEBUG*/
    /// a "donothing" fake streambuf
    class CNullStreambuf : public std::streambuf
    {
     public:
        CNullStreambuf()  {}
        ~CNullStreambuf() {}

     protected:
        int overflow(int) { return 0; }
        int sync()        { return 0; }
    };

    CNullStreambuf g_nullStreambuf;
    std::ostream g_nullStdOStream(&g_nullStreambuf);
    dbg::ELevel g_level = dbg::WARNING;
    bool g_bEnableLevelPrefix = true;
    std::ostream* g_pStream = &std::cout;

    void dbg::setLogLevel(dbg::ELevel l)
    {
        g_level = l;
    }

    void dbg::enableLevelPrefix(bool enable)
    {
        g_bEnableLevelPrefix = enable;
    }

    void dbg::setLogStream(std::ostream* ps)
    {
        g_pStream = ps;
    }

    std::ostream& dbg::out(dbg::ELevel l)
    {
        if(l > g_level)
        {
            return g_nullStdOStream;
        }

        if(g_bEnableLevelPrefix)
        {
            switch(l)
            {
                case dbg::ALL: {*g_pStream << "all:"; break;}
                case dbg::DEBUG: {*g_pStream << "debug:"; break;}
                case dbg::ERROR: {*g_pStream << "error:"; break;}
                case dbg::FATAL: {*g_pStream << "fatal:"; break;}
                case dbg::INFO: {*g_pStream << "info:"; break;}
                case dbg::NONE: {*g_pStream << "none"; break;}
                case dbg::TRACE: {*g_pStream << "trace:"; break;}
                case dbg::WARNING: {*g_pStream << "warning:"; break;}
            }
        }

        return *g_pStream;
    }

    std::ostream& dbg::operator<<(std::ostream& s, const struct dbg::SPosition& pos)
    {
        s << pos.pchFile << "," << pos.iLine << ":";
        return s;
    }
#endif /*NDEBUG*/


