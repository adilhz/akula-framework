
#include "connection.h"
#include "event_handlers.h"
#include <akula/net/net.h>
#include <akula/http/http.h>
#include <akula/utils/utils.h>
#include <iostream>
#include <cassert>

const unsigned long CConnection::RECEIVING_BUFFER_SIZE = 2048;
const unsigned long CConnection::SENDING_BUFFER_SIZE = 100000;

CConnection::CConnection(net::CTCPSocket *pSocket,
                                            reactor::CReactorUtils::IEventHandler* pReadCallback,
                                            reactor::CReactorUtils::IEventHandler* pWriteCallback)
    : m_pConnSocket(pSocket)
    , m_pReadCallback(pReadCallback)
    , m_pWriteCallback(pWriteCallback)
    , m_RcvDataBlock(RECEIVING_BUFFER_SIZE)
    , m_SndDataBlock(SENDING_BUFFER_SIZE)
    , m_pReactor(reactor::getReactor())
{
    //m_pRepliesQueue = new CThreadSafeQueue<CHttpReply*>; //use this for thread-safe queue
    m_pRepliesQueue = new utils::CQueue<http::CHttpReply*>;

    m_pRequest = new http::CHttpRequest();
    m_pReply = NULL;
}

CConnection::~CConnection()
{
    delete m_pConnSocket;
    m_pConnSocket = NULL;
    delete m_pRepliesQueue;
    m_pRepliesQueue = NULL;
}

bool CConnection::registerToReactor(const unsigned long ulEvent)
{
    if(ulEvent & reactor::CReactorUtils::READ_MASK)
    {
        m_pReactor->register_socket(m_pConnSocket, reactor::CReactorUtils::READ_MASK, m_pReadCallback);
    }
    
    if(ulEvent & reactor::CReactorUtils::WRITE_MASK)
    {
        m_pReactor->register_socket(m_pConnSocket, reactor::CReactorUtils::WRITE_MASK, m_pWriteCallback);
    }
}

bool CConnection::unregisterFromReactor(const unsigned long ulEvent)
{
    m_pReactor->unregister_socket(m_pConnSocket, ulEvent);
}

CConnection::EConnectionState
CConnection::send()
{
    if(m_SndDataBlock.getReadPosition() == m_SndDataBlock.getWritePosition()) //current data block sent
    {
        //reset the positions
        m_SndDataBlock.setReadPosition(m_SndDataBlock.getStart());
        m_SndDataBlock.setWritePosition(m_SndDataBlock.getStart());

        //get the very first reply
        if(m_pReply == NULL)
        {
            assert(m_pRepliesQueue->isEmpty() != true);
            m_pReply = m_pRepliesQueue->get();
        }
        
        unsigned long ulEmited = 0;
        //emit() return 0 means the current reply is sent, try to send the next or unregister for read
        while((ulEmited = m_pReply->emit(m_SndDataBlock.getWritePosition(),
                                                        m_SndDataBlock.getEnd() - m_SndDataBlock.getWritePosition()))
                  == 0)
        {
            delete m_pReply;
            m_pReply = NULL;
            
            if(m_pRepliesQueue->isEmpty())
            {
                unregisterFromReactor(reactor::CReactorUtils::WRITE_MASK);
                return CONNECTION_OK;
            }

            m_pReply = m_pRepliesQueue->get();
        }
        
        m_SndDataBlock.setWritePosition(m_SndDataBlock.getWritePosition() + ulEmited);
    }

    long lBytesSent = m_pConnSocket->send(m_SndDataBlock.getReadPosition(),
                                            m_SndDataBlock.getWritePosition() - m_SndDataBlock.getReadPosition());

    if(lBytesSent <= 0)
    {
        unregisterFromReactor(reactor::CReactorUtils::REMOVE_MASK | reactor::CReactorUtils::READ_MASK | reactor::CReactorUtils::WRITE_MASK);
        m_SndDataBlock.setReadPosition(m_SndDataBlock.getStart());
        m_SndDataBlock.setWritePosition(m_SndDataBlock.getStart());

        if(lBytesSent < 0)
        {
            //log something
        }
        
        return CONNECTION_CLOSE;
    }

    m_SndDataBlock.setReadPosition(m_SndDataBlock.getReadPosition() + lBytesSent);

    return CONNECTION_OK;
}

CConnection::EConnectionState
CConnection::read()
{
    long lReceivedBytes = m_pConnSocket->read(m_RcvDataBlock.getWritePosition(),
                                                        m_RcvDataBlock.getEnd() - m_RcvDataBlock.getWritePosition());

    if(lReceivedBytes <= 0)
    {
        unregisterFromReactor(reactor::CReactorUtils::REMOVE_MASK | reactor::CReactorUtils::READ_MASK | reactor::CReactorUtils::WRITE_MASK);
        m_RcvDataBlock.setReadPosition(m_RcvDataBlock.getStart());
        m_RcvDataBlock.setWritePosition(m_RcvDataBlock.getStart());
        if(lReceivedBytes < 0)
        {
            //log something
        }

        return CONNECTION_CLOSE;
    }

    m_RcvDataBlock.setWritePosition(m_RcvDataBlock.getWritePosition() + lReceivedBytes);

    while(1)
    {
        int r = m_pRequest->parse(&m_RcvDataBlock);
        if(r == 0) //need more data to construct request
        {
            //TODO: check if the buffer is filled and still have no request line - return error to the client
            break;
        }
        else if(r == 1) //request parsed, try to parse the rest of the data
        {
            http::CHttpReply* pReply = http::CResource::performRequest(m_pRequest);
            m_pRepliesQueue->put(pReply);
            delete m_pRequest;
            m_pRequest = new http::CHttpRequest();
            registerToReactor(reactor::CReactorUtils::WRITE_MASK);
        }
        else if(r < 1) //error
        {
            delete m_pRequest;
            m_pRequest = new http::CHttpRequest();
            m_RcvDataBlock.setReadPosition(m_RcvDataBlock.getStart());
            m_RcvDataBlock.setWritePosition(m_RcvDataBlock.getStart());
            http::CHttpReply* pReply = new http::CHttpReply(400, http::CHttpMessage::MESSAGE_BAD_REQUEST, NULL);
            m_pRepliesQueue->put(pReply);
            registerToReactor(reactor::CReactorUtils::WRITE_MASK);
            break;
        }
    }

    return CONNECTION_OK;
}

