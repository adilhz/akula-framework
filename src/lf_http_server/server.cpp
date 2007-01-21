/**
    @file server.cpp 
*/

#include <akula/net/net.h>
#include <akula/utils/utils.h>
#include "server.h"
#include "connection.h"
#include <csignal>
#include <cassert>

CServer::CServer()
    : m_pReactor(NULL)
    , m_pServerAddress(NULL)
    , m_pTCPServerSocket(NULL)
{
}

CServer::~CServer()
{
    delete m_pServerAddress;
    delete m_pTCPServerSocket;
}

bool
CServer::initialize(const std::string& sServerAddress)
{
    m_pReactor = reactor::CReactor::singleton();
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

    return true;
}

bool
CServer::handle_read(net::CSocket* pSocket)
{
    net::CTCPSocket* pConnSocket = m_pTCPServerSocket->accept();
    
    CConnection* pConnection = new CConnection();
    pConnection->start(pConnSocket);
    return true;
}

bool
CServer::start(const std::string& sServerAddress, const int iNumberOfThreads)
{
    if(!initialize(sServerAddress))
        return false;
    
    ::signal(SIGPIPE, SIG_IGN);

    assert(iNumberOfThreads > 0);

    m_pReactor->register_socket(m_pTCPServerSocket, reactor::CReactorUtils::READ_EVENT, this);
    m_pReactor->handle_events(iNumberOfThreads);
}

