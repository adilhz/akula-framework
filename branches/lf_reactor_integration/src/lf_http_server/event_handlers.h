
#ifndef EVENT_HANDLERS_H
#define EVENT_HANDLERS_H

#include "connections_table.h"
#include <akula/reactor/reactor.h>

//forward decl.
class CLFThreadPool;
namespace net {class CSocket;}


/*
--------------------------------------CSocketDecoratorCallback--------------------------------
*/
class CSocketCallbackDecorator : public reactor::CReactorUtils::IEventHandler
{
private:
    CLFThreadPool *m_pThreadPool;
    reactor::CReactorUtils::IEventHandler* m_pActualSocketCallback;
    reactor::Reactor_t* m_pReactor;

public:
    CSocketCallbackDecorator(CLFThreadPool*, reactor::CReactorUtils::IEventHandler*);

    ~CSocketCallbackDecorator();

    virtual long callback(net::CSocket *pSocket, unsigned long ulFlags);
};

/*
------------------------------------------------CTCPServerSocketLogic--------------------------------------------
*/
/**
 * @class CTCPServerSocketLogic
 * @brief	The logic for accepting connections and creates CReceiver for every connection
 */
class CTCPServerSocketLogic : public reactor::CReactorUtils::IEventHandler
{
private:
    ConnectionsTable_t* m_pConnectionsTable;
    
    /**Used to pass to the newly created connections*/
    CSocketCallbackDecorator* m_pReceiverDecorator;
    CSocketCallbackDecorator* m_pSenderDecorator;

public:
    CTCPServerSocketLogic(ConnectionsTable_t*,CLFThreadPool*);

    ~CTCPServerSocketLogic();

    /**
     * The main logic of the CTCPServerSocketLogic - accepting connections,
     * creating CReceiver objects for every connection
     * @param poSocket	Pointer to the registered in CSelect socket
     * @param lFlags	The event the socket is registered for
     */
    virtual long callback(net::CSocket *poSocket, unsigned long ulFlags);
};

/*
---------------------------------------------CReceiver----------------------------------------
*/

/**
 * @class CReceiver
 * @brief	The logic for processing the compressed chunks received from the network
 * @brief	Has CDecoder for decompressing the chunks
 * @brief	Has CFileCollector for the chunks to the file
 */
class CReceiver : public reactor::CReactorUtils::IEventHandler
{
private:
    ConnectionsTable_t* m_pConnectionsTable;

public:
    CReceiver(ConnectionsTable_t*);

    ~CReceiver();

    /**
     * The logic that will be invoked for the given connection socket, i.e. this is the main logic of the CReceiver
     * This function (method) is being registered in the CSelect for reading event.
     */
    virtual long callback(net::CSocket * poSocket, unsigned long ulFlags);
};

class CSender : public reactor::CReactorUtils::IEventHandler
{
private:
    ConnectionsTable_t* m_pConnectionsTable;
public:
    CSender(ConnectionsTable_t*);

    ~CSender();
private:
    virtual long callback(net::CSocket * pSocket, unsigned long ulFlags);
};

#endif //EVENT_HANDLERS_H

