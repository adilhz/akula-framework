
#include <akula/net/net.h>
#include <akula/dbg/dbg.h>
#include "connection.h"
#include "event_handlers.h"
#include "lf_thread_pool.h"
#include <iostream>
#include <cassert>


/*
------------------------CSocketCallbackDecorator-----------------------------
*/

CSocketCallbackDecorator::CSocketCallbackDecorator(CLFThreadPool *pThreadPool,
                                                                        reactor::CReactorUtils::IEventHandler* pActualSocketCallback)
    : m_pThreadPool(pThreadPool),
    m_pActualSocketCallback(pActualSocketCallback),
    m_pReactor(reactor::getReactor())
{
}

CSocketCallbackDecorator::~CSocketCallbackDecorator()
{
    delete m_pActualSocketCallback;
}

long
CSocketCallbackDecorator::callback(net::CSocket *pSocket, unsigned long ulFlags)
{
    m_pReactor->deactivate_socket(pSocket);

    m_pThreadPool->promote_new_leader();

    if(m_pActualSocketCallback->callback(pSocket, ulFlags) != CConnection::CONNECTION_CLOSE)
    {
        m_pReactor->reactivate_socket(pSocket);
    }
}

/*
------------------------------------------------CTCPServerSocketLogic--------------------------------------------
*/
CTCPServerSocketLogic::CTCPServerSocketLogic(ConnectionsTable_t* pConnectionsTable, CLFThreadPool *pThreadPool)
    : m_pConnectionsTable(pConnectionsTable)
{
    m_pReceiverDecorator = new CSocketCallbackDecorator(pThreadPool, new CReceiver(pConnectionsTable));
    m_pSenderDecorator = new CSocketCallbackDecorator(pThreadPool, new CSender(pConnectionsTable));
}

CTCPServerSocketLogic::~CTCPServerSocketLogic()
{
    delete m_pReceiverDecorator;
    delete m_pSenderDecorator;
}

long
CTCPServerSocketLogic::callback(net::CSocket *poSocket, unsigned long ulFlags)
{
    // don't use type (RTTI) checking because it can degrade performance
    //net::CTCPServerSocket *pTCPServerSocket = dynamic_cast<net::CTCPServerSocket *>(poSocket);
    net::CTCPServerSocket* pTCPServerSocket = static_cast<net::CTCPServerSocket*>(poSocket);
    
    if(pTCPServerSocket)
    {
        net::CTCPSocket* pConnectionSocket = pTCPServerSocket->accept();
        if(pConnectionSocket)
        {
            pConnectionSocket->setNonBlocking();

            CConnection *pConnection = new CConnection(pConnectionSocket, m_pReceiverDecorator, m_pSenderDecorator);
            
            m_pConnectionsTable->addConnection(pConnection);
            
            pConnection->registerToReactor(reactor::CReactorUtils::READ_MASK);
        }
        else
        {
            dbg::error() << "Couldn't accept connection\n";
        }
    }

    return CConnection::CONNECTION_OK;
}

/*
---------------------------------------------CReceiver----------------------------------------
*/

CReceiver::CReceiver(ConnectionsTable_t* pConnectionsTable)
    : m_pConnectionsTable(pConnectionsTable)
{
}

CReceiver::~CReceiver()
{
}

long
CReceiver::callback(net::CSocket *pSocket, unsigned long ulFlags)
{
    long lResult = 0;
    CConnection* pConnection = m_pConnectionsTable->getConnection(pSocket);

    if(pConnection)
    {
        lResult = pConnection->read();
        if(lResult == CConnection::CONNECTION_CLOSE)
        {
            m_pConnectionsTable->removeConnection(pConnection);
            delete pConnection;
        }
    }
    else
    {
        dbg::error() << "No connection found for the given socket " << pSocket->getSocketHandle() << "\n";
    }

    return lResult;
}

/*
---------------------------------------CSender--------------------------------------
*/
CSender::CSender(ConnectionsTable_t* pConnectionsTable)
    : m_pConnectionsTable(pConnectionsTable)
{
}

CSender::~CSender()
{
}

long
CSender::callback(net::CSocket * pSocket, unsigned long ulFlags)
{
    long lResult = 0;
    CConnection* pConnection = m_pConnectionsTable->getConnection(pSocket);

    if(pConnection)
    {
        lResult = pConnection->send();
        if(lResult == CConnection::CONNECTION_CLOSE)
        {
            m_pConnectionsTable->removeConnection(pConnection);
            delete pConnection;
        }
    }
    else
    {
        dbg::error() << "No connection found for the given socket " << pSocket->getSocketHandle() << "\n";
    }

    return lResult;
}

