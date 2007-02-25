/**
    @file server.h 
*/

#ifndef SERVER_H
#define SERVER_H

#include <akula/reactor/reactor.h>
#include <string>

//forward decl.
namespace net 
{
    class CTCPServerSocket;
    class CSocket;
    class CAddress;
}

/**
 * @class CServer
 * @brief The server
 */
class CServer : reactor::CReactorUtils::IEventHandler
{
private:
    reactor::CReactor* m_pReactor;
    net::CAddress* m_pServerAddress;
    net::CTCPServerSocket* m_pTCPServerSocket;

    /**
    *   Initialize the server
    */
    bool initialize(const std::string&);

    bool handle_read(net::CSocket* pSocket);
public:
    CServer();

    ~CServer();

    /**
    *   Start the server
    */
    bool start(const std::string&, const int);
};

#endif //SERVER_H

