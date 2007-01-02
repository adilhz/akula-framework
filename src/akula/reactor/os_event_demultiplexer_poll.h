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

#ifndef OS_EVENT_DEMULTIPLEXER_POLL_H
#define OS_EVENT_DEMULTIPLEXER_POLL_H

#include <unistd.h> /*fdatasync(), read()*/

/*forward declarations*/
struct pollfd;

namespace reactor
{

// Engine for the OsEventDemultiplexer
class OsEventDemultiplexerPollImpl
{
 public:
    OsEventDemultiplexerPollImpl();
    ~OsEventDemultiplexerPollImpl();
    
    void add_fd(int iFD, unsigned long ulFlag);
    void remove_fd(int iFD, unsigned long ulFlag);

    void deactivate_fd(int);
    void reactivate_fd(int);

    int watch_fds(void);
    bool check_fd(int iFD, unsigned long ulFlag);

 inline void trigger_internal_command(char command)
 {
     // send command to the reactor (writing to the pipe)
     ::write(m_pipe[1], &command, 1);
     ::fdatasync(m_pipe[1]);
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

 int m_pipe[2];

 private:
    struct pollfd* m_pPollDataTable;
    int* m_pIndexTable;

    //max used index into struct pollfd array
    int m_iNumberOfPolledFds;
};

}//namespace reactor

#endif /*OS_EVENT_DEMULTIPLEXER_POLL_H*/

