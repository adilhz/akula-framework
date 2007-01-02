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

#ifndef DBG_H
#define DBG_H

#include <ostream>

/**
* Logging utilities intarface. Be carefull, the logger is not thread-safe, i.e. when using in not-threadsafe
* critical sections it is not guarantied what the result will be.
*
* Examples of logging interface usage you can find in the unit tests <code>test_dbg.cpp</code>
* In common:

* dbg::setLogLevel(dbg::DEBUG); //set the logging level
* dbg::enableLevelPrefix(true); //enable printing the level for the given message in the form "level:message"
* dbg::setLogStream(&std::cerr); //set the stream you to log into
*
* and now the real login 
*
* dbg::error() << "Error message" << std::endl; //which is equivalent to
* dbg::out(dbg::ERROR) << "Error message" << std::endl;
*
* You can use the macro LOG_POSITION to specify the file and line of the logging message, for example:
* dbg::warning() << "Here: " << LOG_POSITION << "happens something" << std::endl;
*
* If defined NDEBUG during build all the log routines should compile to nothing, i.e. they would be optimized by the compiler.
*/

namespace dbg
{
        enum ELevel
        {
            NONE,
            FATAL,
            ERROR,
            WARNING,
            INFO,
            DEBUG,
            TRACE,
            ALL
        };

        struct SPosition
        {
            const char* pchFile;
            const unsigned int iLine;
            SPosition()
                : pchFile(NULL),
                iLine(0)
             {}

            SPosition(const char* file, const unsigned int line)
                : pchFile(file),
                iLine(line)
            {}
        };

#ifdef NDEBUG
        #define LOG_POSITION ((void*)0)
#else
        #define LOG_POSITION (aim_log::SPosition(__FILE__, __LINE__))
#endif

        std::ostream& operator<<(std::ostream&, const SPosition&);
    
        void setLogLevel(ELevel);

        void enableLevelPrefix(bool);

        void setLogStream(std::ostream*);

#ifdef NDEBUG
        /// a "donothing" fake stream
        //The idea behind using CNullStream is the code to be stripped by the compiler, i.e.
        // in non-debug versions the additional debug code to be stripped by the compiler
        // CNullstream is only for internal use!!!
        class CNullStream
        {
        public:
            template<class T>
            CNullStream& operator<<(T& t)
            { return *this; }

            template<class T>
            CNullStream& operator<<(const T& t)
            { return *this; }
            
            CNullStream& operator<<(std::ios& (*pfn)(std::ios&))
            {return *this;}
            
            CNullStream& operator<<(std::ostream& (*pfn)(std::ostream&))
            {return *this;}
        };

        CNullStream& out(ELevel);
        
        inline CNullStream& info() {return out(INFO);}
        inline CNullStream& warning() {return out(WARNING);}
        inline CNullStream& error() {return out(ERROR);}
        inline CNullStream& fatal() {return out(FATAL);}
        inline CNullStream& trace() {return out(TRACE);}
        inline CNullStream& debug() {return out(DEBUG);}
        inline CNullStream& none() {return out(NONE);}
        inline CNullStream& all() {return out(ALL);}
        
#else
        std::ostream& out(ELevel);
        
        inline std::ostream& info() {return out(INFO);}
        inline std::ostream& warning() {return out(WARNING);}
        inline std::ostream& error() {return out(ERROR);}
        inline std::ostream& fatal() {return out(FATAL);}
        inline std::ostream& trace() {return out(TRACE);}
        inline std::ostream& debug() {return out(DEBUG);}
        inline std::ostream& none() {return out(NONE);}
        inline std::ostream& all() {return out(ALL);}
        
#endif
} /*namespace dbg*/

#endif /*DBG_H*/

