
#ifndef __CONNECTION_H
#define __CONNECTION_H

#include <akula/http/http.h>
#include <akula/utils/utils.h>
#include <akula/reactor/reactor.h>
#include <akula/net/net.h>
#include <iostream>
#include <cassert>
#include <string>

class CConnection : public reactor::CReactorUtils::IEventHandler
{
 private:
    net::CTCPSocket* m_pConnSocket;
    reactor::CReactor* m_pReactor;

    ///Buffers for receiving/sendinf data
    http::CDataBlock m_RcvDataBlock, m_SndDataBlock;
    ///Current request
    http::CHttpRequest* m_pRequest;
    ///Current reply
    http::CHttpReply* m_pReply;

    /*We will not use thread safe queue because a single connection will be reached only from a single
     * thread for read/write, because the descriptor concerned with the connection is temporary deactivated
     * when invoked for a given event. All that means that this queue will not be reached from multiple
     * threads simultaneously.
     */
    utils::CQueue<http::CHttpReply*>* m_pRepliesQueue;


 public:
    CConnection()
        : m_pConnSocket(NULL)
        , m_pReactor(reactor::CReactor::singleton())
        , m_pRequest(new http::CHttpRequest)
        , m_pReply(NULL)
        , m_pRepliesQueue(new utils::CQueue<http::CHttpReply*>)
    {
    }

    ~CConnection()
    {
        delete m_pConnSocket;
        m_pConnSocket = NULL;
        delete m_pRepliesQueue;
        m_pRepliesQueue = NULL;
    }

    void start(net::CTCPSocket* pConnSocket)
    {
        m_pConnSocket = pConnSocket;
        m_pReactor->register_socket(m_pConnSocket, reactor::CReactorUtils::READ_EVENT, this);
    }

    void stop()
    {
        m_pReactor->unregister_socket(m_pConnSocket,
                                                      reactor::CReactorUtils::READ_EVENT | reactor::CReactorUtils::WRITE_EVENT);
        delete this;
    }

    bool handle_write(net::CSocket* pSocket)
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
                    m_pReactor->unregister_socket(m_pConnSocket, reactor::CReactorUtils::WRITE_EVENT);
                    return true;
                }
        
                m_pReply = m_pRepliesQueue->get();
            }
                
            m_SndDataBlock.setWritePosition(m_SndDataBlock.getWritePosition() + ulEmited);
        }
        
        long lBytesSent = m_pConnSocket->send(m_SndDataBlock.getReadPosition(),
                                                m_SndDataBlock.getWritePosition() - m_SndDataBlock.getReadPosition());
        
        if(lBytesSent <= 0)
        {
            stop();
            return false;
        }
        
        m_SndDataBlock.setReadPosition(m_SndDataBlock.getReadPosition() + lBytesSent);
        
        return true;
    }

    bool handle_read(net::CSocket* pSocket)
    {
        long lReceivedBytes = m_pConnSocket->read(m_RcvDataBlock.getWritePosition(),
                                                            m_RcvDataBlock.getEnd() - m_RcvDataBlock.getWritePosition());
        
        if(lReceivedBytes <= 0)
        {
            stop();
            return false;
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
                m_pReactor->register_socket(m_pConnSocket, reactor::CReactorUtils::WRITE_EVENT, this);
            }
            else if(r < 1) //error
            {
                delete m_pRequest;
                m_pRequest = new http::CHttpRequest();
                m_RcvDataBlock.setReadPosition(m_RcvDataBlock.getStart());
                m_RcvDataBlock.setWritePosition(m_RcvDataBlock.getStart());
                http::CHttpReply* pReply = new http::CHttpReply(400, http::CHttpMessage::MESSAGE_BAD_REQUEST, NULL);
                m_pRepliesQueue->put(pReply);
                m_pReactor->register_socket(m_pConnSocket, reactor::CReactorUtils::WRITE_EVENT, this);
                break;
            }
        }
        
        return true;
    }
};

#endif //__CONNECTION_H

