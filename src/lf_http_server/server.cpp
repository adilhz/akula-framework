/**
    @file server.cpp 
*/

#include <akula/net/net.h>
#include <akula/utils/utils.h>
#include "server.h"
#include "connection.h"
#include "lf_thread_pool.h"
#include <csignal>
#include <cassert>

CServer::CServer()
    : m_pReactor(NULL)
    , m_pThreadPool(NULL)
    , m_pConnectionsTable(NULL)
    , m_pReceiver(NULL)
    , m_pSender(NULL)
    , m_pServerAddress(NULL)
    , m_pTCPServerSocket(NULL)
    , m_pTCPServerSocketLogic(NULL)
    , m_pServerSocketDecorator(NULL)
    , m_bInitialized(false)

{
}

CServer::~CServer()
{
    if(m_bInitialized)
        m_pReactor->unregister_socket(m_pTCPServerSocket, reactor::CReactorUtils::READ_MASK);

    delete m_pServerAddress;
    delete m_pTCPServerSocket;
    delete m_pTCPServerSocketLogic;
    delete m_pServerSocketDecorator;
    delete m_pThreadPool;
    delete m_pConnectionsTable;
    delete m_pReceiver;
    delete m_pSender;
}

bool
CServer::initialize(const std::string& sServerAddress)
{
    m_pReactor = reactor::getReactor();
    
    m_pThreadPool = new CLFThreadPool();
    
    m_pConnectionsTable = new ConnectionsTable_t();

    m_pServerAddress = net::CAddress::parse(sServerAddress);
    if(m_pServerAddress == NULL)
    {
        dbg::error() << "Wrong server address\n";
        return false;
    }

    try
    {
        m_pTCPServerSocket = new net::CTCPServerSocket();
    }
    catch(const net::x_socket_exception& se)
    {
        dbg::error() << se.what() << "\n";
        return false;
    }
    m_pTCPServerSocket->setNonBlocking();
    m_pTCPServerSocket->setReuseAddress();
    m_pTCPServerSocket->bind(m_pServerAddress);
    m_pTCPServerSocket->listen();

    m_pTCPServerSocketLogic = new CTCPServerSocketLogic(m_pConnectionsTable, m_pThreadPool);
    m_pServerSocketDecorator = new CSocketCallbackDecorator(m_pThreadPool, m_pTCPServerSocketLogic);

    m_pReactor->register_socket(m_pTCPServerSocket, reactor::CReactorUtils::READ_MASK, m_pServerSocketDecorator);

    m_bInitialized = true;
    return true;
}

bool
CServer::start(const std::string& sServerAddress, const int iNumberOfThreads)
{
    if(!initialize(sServerAddress))
        return false;
    
    ::signal(SIGPIPE, SIG_IGN);

    assert(iNumberOfThreads > 0);

    if(iNumberOfThreads > 1)
    {
        for(int i = 0; i < iNumberOfThreads-1; i++)
        {
            utils::CThread<SServerThread>* pThread = 
                utils::CThread<SServerThread>::getInstance(reinterpret_cast<void*>(m_pThreadPool));
            
            if(!pThread->run())
                assert(false);
        }
    }

    m_pThreadPool->join(); //join the main thread into the thread pool
}

