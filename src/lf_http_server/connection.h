
#ifndef __CONNECTION_H
#define __CONNECTION_H

#include <akula/http/http.h>
#include <akula/utils/utils.h>
#include <akula/reactor/reactor_type.h>
#include <string>

//forward decl.
namespace net {class CTCPSocket;}
namespace http
{
    class CHttpRequest;
    class CHttpReply;
}

class CConnection
{
 private:
    net::CTCPSocket* m_pConnSocket;
    reactor::CReactorUtils::IEventHandler* m_pReadCallback, *m_pWriteCallback;
    reactor::Reactor_t* m_pReactor;

    ///Buffers for receiving/sendinf data
    http::CDataBlock m_RcvDataBlock, m_SndDataBlock;
    ///Current request
    http::CHttpRequest* m_pRequest;
    ///Current reply
    http::CHttpReply* m_pReply;

 public:
    enum EConnectionState {CONNECTION_OK = 0, CONNECTION_CLOSE};
    
    static const unsigned long RECEIVING_BUFFER_SIZE;
    static const unsigned long SENDING_BUFFER_SIZE;

    /*We will not use thread safe queue because a single connection will be reached only from a single
     * thread for read/write, because the descriptor concerned with the connection is temporary deactivated
     * when invoked for a given event. All that means that this queue will not be reached from multiple
     * threads simultaneously.
     */
    utils::CQueue<http::CHttpReply*>* m_pRepliesQueue;
    
 public:
    CConnection(net::CTCPSocket*, reactor::CReactorUtils::IEventHandler*, reactor::CReactorUtils::IEventHandler*);

    ~CConnection();

    bool registerToReactor(const unsigned long ulEvent);

    bool unregisterFromReactor(const unsigned long ulEvent);

    EConnectionState read();
    EConnectionState send();

    net::CTCPSocket* getSocket() const {return m_pConnSocket;}
};

#endif //__CONNECTION_H

