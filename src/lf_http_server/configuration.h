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

#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <akula/utils/utils.h>
#include <akula/dbg/dbg.h>

class CConfiguration
{
 public:
    static const char*            CONFIG_FILE;
    static const int                CONFIG_LINE_MAX_LENGTH;
    
    static const std::string        CONFIG_ENTRY_THREADS;
    static const int                   CONFIG_ENTRY_THREADS_DEFAULT_VALUE;

    static const std::string        CONFIG_ENTRY_ADDRESS;
    static const std::string        CONFIG_ENTRY_ADDRESS_DEFAULT_VALUE;

    static const std::string        CONFIG_ENTRY_LOG;
    static dbg::ELevel                    CONFIG_ENTRY_LOG_DEFAULT_VALUE;
    
    typedef std::map<std::string, std::string>      ConfigContainer_t;
    typedef ConfigContainer_t::iterator                 ConfigContainerIt_t;

 private:
    struct SFileResource
    {
        std::ifstream file;

        SFileResource(const char* pchFileName) 
            : file(pchFileName, std::ios::in | std::ios::binary)
        {}

        ~SFileResource()
        {
            if(file.is_open())
                file.close();
        }
    };

 private:
    ConfigContainer_t m_configuration;
    bool m_bConfigFileRead;

 private:
    CConfiguration();
    
 public:
    ~CConfiguration() {}

    static CConfiguration* instance();

    bool read();

    template<class T>
    bool get(const std::string& key, T& value)
    {
        ConfigContainerIt_t IT = m_configuration.find(key);
        
        if(IT == m_configuration.end())
            return false;

        return utils::from_string(value, IT->second, std::dec);
    }

    inline int getThreads()
    {
        int iThreads;
        if(get(CONFIG_ENTRY_THREADS, iThreads))
            return iThreads;

        return CONFIG_ENTRY_THREADS_DEFAULT_VALUE;
    }

    inline std::string getAddress()
    {
        std::string sAddress;
        if(get(CONFIG_ENTRY_ADDRESS, sAddress))
            return sAddress;

        return CONFIG_ENTRY_ADDRESS_DEFAULT_VALUE;
    }

    inline dbg::ELevel getLogLevel()
    {
        std::string sLogLevel;
        dbg::ELevel l = CONFIG_ENTRY_LOG_DEFAULT_VALUE;
        
        if(get(CONFIG_ENTRY_LOG, sLogLevel))
        {
            if(sLogLevel.compare("none") == 0)
                l = dbg::NONE;
            else if(sLogLevel.compare("info") == 0)
                l = dbg::INFO;
            else if(sLogLevel.compare("warning") == 0)
                l = dbg::WARNING;
            else if(sLogLevel.compare("error") == 0)
                l = dbg::ERROR;
            else if(sLogLevel.compare("fatal") == 0)
                l = dbg::FATAL;
            else if(sLogLevel.compare("trace") == 0)
                l = dbg::TRACE;
            else if(sLogLevel.compare("debug") == 0)
                l = dbg::DEBUG;
            else if(sLogLevel.compare("all") == 0)
                l = dbg::ALL;
        }

        return l;
    }
};

