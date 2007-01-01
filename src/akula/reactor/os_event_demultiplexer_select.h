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

#ifndef OS_EVENT_DEMULTIPLEXER_SELECT_H
#define OS_EVENT_DEMULTIPLEXER_SELECT_H

#include <sys/select.h>
#include <exception>
#include <string>

/*!!! Not working. To be implemented!!!*/

namespace reactor
{

// Engine for OsEvenetDemultiplexer
class OsEventDemultiplexerSelectImpl
{
 public:
    OsEventDemultiplexerSelectImpl();
    ~OsEventDemultiplexerSelectImpl();

    inline void trigger_internal_command(char command)
    {
        // send command to the reactor (writing to the pipe)
        ::write(m_pipe[1], &command, 1);
    }
        
    inline bool process_internal_command()
    {
        char command;
        ::read(m_pipe[0], &command, 1);
        
        // do something
        switch(command)
        {
            case 'a': // event added
                return true;
            break;
            case 'r': // event removed
                return true;
            break;
            case 'e': // dispatch loop exit
                //return m_bStopRequest = true;
            break;
            default:
                return false;
            break;
        }
    }

    
    void add_fd(int iFD, unsigned long ulFlag);
    void remove_fd(int iFD, unsigned long ulFlag);

    int watch_fds(void);
    bool check_fd(int iFD, unsigned long ulFlag);

 private:
    static const int MAX_ALLOWED_FD = FD_SETSIZE;

    fd_set m_master_rfdset;
    fd_set m_master_wfdset;
    fd_set m_working_rfdset;
    fd_set m_working_wfdset;

    int m_pipe[2];
        
 protected:
    int get_max_fd(void);

public:
    class x_select : public std::exception
    {
     public:
        x_select(const std::string& sMessage) throw()
            : m_sMessage(sMessage)
        {}

        virtual ~x_select() throw()
        {}

        virtual const char* what() const throw()
        {
            return m_sMessage.c_str();
        }

     protected:
        std::string m_sMessage;
    };

    class x_select_max_fd_exceeded : public x_select
    {
     public:
        x_select_max_fd_exceeded() throw()
            : x_select("select(): trying to add fd greater than FD_SETSIZE.")
        {}
        virtual ~x_select_max_fd_exceeded() throw()
        {}
    };

};

}//namespace reactor

#endif /*OS_EVENT_DEMULTIPLEXER_SELECT_H*/

