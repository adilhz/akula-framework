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
#include "TUT/tut.h"
#include "akula/reactor/reactor.h"
#include "akula/net/net.h"
#include "akula/utils/utils.h"
#include <functional>

namespace tut
{
    struct test_reactor
    {
    };

    typedef test_group<test_reactor> factory;
    typedef factory::object object;

    factory test_reactor("test_reactor");

        class CClientLogic : public reactor::CReactorUtils::IEventHandler
        {
            char buffer[1024];
            net::CTCPSocket* m_pTCPSocket;
            reactor::Reactor_t* m_pReactor;
            
          public:
            CClientLogic(net::CTCPSocket* pSocket, reactor::Reactor_t* pReactor)
                : m_pTCPSocket(pSocket)
                , m_pReactor(pReactor)
            {
            }
            
            ~CClientLogic()
            {
                delete m_pTCPSocket;
            }
            
            long callback(net::CSocket *pSocket, unsigned long ulFlags)
            {
                //net::CTCPSocket* pClient = dynamic_cast<net::CTCPSocket*>(pSocket);
                int bytes = m_pTCPSocket->read(buffer,1024);
                ensure("bytes received", bytes == 5);
                ensure("information", std::string(buffer, 5).compare("aaaaa") == 0);

                //ahh, yes, clean me, I've notthing to do
                m_pReactor->unregister_socket(pSocket, ulFlags);
                delete this;
                
                return 0;
            }
        };

        class CServerLogic : public reactor::CReactorUtils::IEventHandler
        {
         private:
            reactor::Reactor_t* m_pReactor;
            net::CTCPServerSocket* m_pServerSocket;
            
         public:
            CServerLogic(reactor::Reactor_t* pReactor, net::CAddress* pAddress)
                : m_pReactor(pReactor)
            {
                //create the server
                m_pServerSocket = new net::CTCPServerSocket;
                m_pServerSocket->setReuseAddress();
                m_pServerSocket->bind(pAddress);
                m_pServerSocket->listen();

                m_pReactor->register_socket(m_pServerSocket, reactor::CReactorUtils::READ_MASK, this);
            }

            ~CServerLogic()
            {
                delete m_pServerSocket;
            }
            
            long callback(net::CSocket *pSocket, unsigned long ulFlags)
            {
                //net::CTCPServerSocket* pServer = dynamic_cast<net::CTCPServerSocket*>(pSocket);
                net::CTCPSocket* pClient = m_pServerSocket->accept();
                m_pReactor->register_socket( pClient,
                                                            reactor::CReactorUtils::READ_MASK,
                                                            new CClientLogic(pClient, m_pReactor));

                return 0;
            }
        };

        struct SServerThread : public std::unary_function<char*,char*>
        {
            void* operator()(void* parg)
            {
                reactor::Reactor_t* pReactor = reinterpret_cast<reactor::Reactor_t*>(parg);
                if(pReactor)
                {
                    int i = pReactor->handle_events();
                    ensure("connection accepted", i ==1);
                    i = pReactor->handle_events();
                    ensure("data received", i == 1);
                }

                return NULL;
            }
        };

    /**
    * Client and server: the client sends some data, the server creates conn. socket that reads and
    * compares it. All is done through reactor on the server side, which runs in a different thread (different
    * from the client's one)
    */
    template<>
    template<>
    void object::test<1>()
    {
        dbg::enableLevelPrefix( true);
        dbg::setLogLevel( dbg::ERROR);
        dbg::setLogStream( &std::cerr);

        reactor::Reactor_t* pReactor = reactor::getReactor();
        net::CAddress* pServerAddress = net::CAddress::parse("127.0.0.1:40000");
        
        CServerLogic* pServerLogic = new CServerLogic(pReactor, pServerAddress);

        utils::CThread<SServerThread>* pServerThread = 
            utils::CThread<SServerThread>::getInstance(reinterpret_cast<void*>(pReactor));

        pServerThread->run();

        //client
        net::CTCPSocket client;
        client.connect(*pServerAddress);
        client.send("aaaaa", 5);

        pServerThread->join();

        delete pServerAddress;
        delete pServerLogic;
    }
}
