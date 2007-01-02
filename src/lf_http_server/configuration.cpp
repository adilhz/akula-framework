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

#include "configuration.h"

const char*               CConfiguration::CONFIG_FILE = "akulahttp.config";
const int                   CConfiguration::CONFIG_LINE_MAX_LENGTH = 1024;
    
const std::string        CConfiguration::CONFIG_ENTRY_THREADS("threads");
const int                   CConfiguration::CONFIG_ENTRY_THREADS_DEFAULT_VALUE = 2;

const std::string        CConfiguration::CONFIG_ENTRY_ADDRESS("address");
const std::string        CConfiguration::CONFIG_ENTRY_ADDRESS_DEFAULT_VALUE("localhost:60000");

const std::string        CConfiguration::CONFIG_ENTRY_LOG("log");
dbg::ELevel                    CConfiguration::CONFIG_ENTRY_LOG_DEFAULT_VALUE = dbg::INFO;

CConfiguration::CConfiguration()
{
    m_bConfigFileRead = read();
}

CConfiguration*
CConfiguration::instance()
{
    return new CConfiguration();
}


bool
CConfiguration::read()
{
    SFileResource config(CONFIG_FILE);
    
    if(!config.file.is_open()) //the file doesn't exist
        return false;
    
    char line[CONFIG_LINE_MAX_LENGTH];
    std::vector<std::string> tokenizedLine;

    while(!config.file.eof())
    {
        if(config.file.bad() || config.file.fail())
            return false;
        
        config.file.getline(line, CONFIG_LINE_MAX_LENGTH);

        if(line[0] == '#' || ::strlen(line) == 0) continue;

        utils::CTokenizer<>::tokenize(tokenizedLine, line, line + strlen(line) -1);

        if(tokenizedLine.size() == 2)
        {
            utils::CTrimmer<>::trim(tokenizedLine[0]);
            utils::CTrimmer<>::trim(tokenizedLine[1]);

            m_configuration[tokenizedLine[0]] = tokenizedLine[1];
        }
    }

    return true;
}

