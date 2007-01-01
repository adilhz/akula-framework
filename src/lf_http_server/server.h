/**
	@file server.h 
	
	@brief Interface for CTCPServerSocketLogic, CServer classes
 */

#ifndef SERVER_H
#define SERVER_H

#include "event_handlers.h"
#include "lf_thread_pool.h"
#include "connections_table.h"
#include <akula/reactor/reactor.h>
#include <string>
#include <functional>                   /*std::unary_function<>*/

//forward decl.
namespace net 
{
    class CTCPServerSocket;
    class CAddress;
}
class CLFThreadPool;
class CTCPServerSocketLogic;

/**
 * @class CServer
 * @brief The server
 */
class CServer
{
private:
    reactor::Reactor_t* m_pReactor;
    CLFThreadPool* m_pThreadPool;
    ConnectionsTable_t* m_pConnectionsTable;
    CReceiver* m_pReceiver;
    CSender* m_pSender;
    net::CAddress* m_pServerAddress;
    net::CTCPServerSocket* m_pTCPServerSocket;
    CTCPServerSocketLogic* m_pTCPServerSocketLogic;
    CSocketCallbackDecorator* m_pServerSocketDecorator;
    bool m_bInitialized;

    /** Server thread's logic - join to the server's thread pool*/
    struct SServerThread : public std::unary_function<void*,void*>
    {
        void* operator()(void* parg)
        {
            CLFThreadPool* pThreadPool = reinterpret_cast<CLFThreadPool*>(parg);
            if(pThreadPool)
            {
                pThreadPool->join();
            }
            
            return NULL;
        }
    };

    /**Initialize the server
    */
    bool initialize(const std::string&);
public:
    CServer();

    ~CServer();

    /**Start the server*/
    bool start(const std::string&, const int);
};

#endif //SERVER_H

