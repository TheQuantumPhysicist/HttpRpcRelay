#ifndef RELAY_H
#define RELAY_H

#include "Client/ClientSession.h"
#include "Filters/JsonRPCFilter.h"
#include "Server/RelayServer.h"
#include "Server/RelaySession.h"

template <typename Derived>
class Relay
{
    std::string serverBindAddress;
    uint16_t    serverBindPort;
    std::string clientTargetAddress;
    uint16_t    clientTargetPort;
    uint32_t    threadCount;

    std::unique_ptr<net::io_context>       ioc_client;
    std::unique_ptr<net::io_context::work> ioc_client_work;

    std::unique_ptr<net::io_context>       ioc_server;
    std::unique_ptr<net::io_context::work> ioc_server_work;

    std::function<void(std::thread*)> serverThreadDestructor = [this](std::thread* t) {
        assert(ioc_server != nullptr);
        ioc_server->stop();
        t->join();
        delete t;
    };

    std::function<void(std::thread*)> clientThreadDestructor = [this](std::thread* t) {
        assert(ioc_client != nullptr);
        ioc_client->stop();
        t->join();
        delete t;
    };

    std::vector<std::unique_ptr<std::thread, std::function<void(std::thread*)>>> serverThreadsVector;
    std::vector<std::unique_ptr<std::thread, std::function<void(std::thread*)>>> clientThreadsVector;

    void startThreadsAndIoContext();

    std::shared_ptr<RelayServer> server;

    Derived& derived() { return static_cast<Derived&>(*this); }

public:
    Relay(std::string ServerBindAddress,
          uint16_t    ServerBindPort,
          std::string ClientTargetAddress,
          uint16_t    ClientTargetPort,
          uint32_t    ThreadCount = std::thread::hardware_concurrency());

    void stop();
};

template <typename Derived>
void Relay<Derived>::startThreadsAndIoContext()
{

    ioc_server_work.reset();
    serverThreadsVector.clear(); // destroy any running threads
    ioc_server.reset();

    ioc_client_work.reset();
    clientThreadsVector.clear(); // destroy any running threads
    ioc_client.reset();

    ioc_server      = std::make_unique<net::io_context>(threadCount);
    ioc_server_work = std::make_unique<net::io_context::work>(*ioc_server);

    ioc_client      = std::make_unique<net::io_context>(threadCount);
    ioc_client_work = std::make_unique<net::io_context::work>(*ioc_client);

    serverThreadsVector.reserve(threadCount);
    for (auto i = threadCount; i > 0; --i) {
        serverThreadsVector.emplace_back(std::unique_ptr<std::thread, decltype(serverThreadDestructor)>(
            new std::thread([this] { ioc_server->run(); }), serverThreadDestructor));
    }

    clientThreadsVector.reserve(threadCount);
    for (auto i = threadCount; i > 0; --i) {
        clientThreadsVector.emplace_back(std::unique_ptr<std::thread, decltype(clientThreadDestructor)>(
            new std::thread([this] { ioc_client->run(); }), clientThreadDestructor));
    }
}

template <typename Derived>
Relay<Derived>::Relay(std::string ServerBindAddress,
                      uint16_t    ServerBindPort,
                      std::string ClientTargetAddress,
                      uint16_t    ClientTargetPort,
                      uint32_t    ThreadCount)
    : serverBindAddress(std::move(ServerBindAddress)), serverBindPort(ServerBindPort),
      clientTargetAddress(std::move(ClientTargetAddress)), clientTargetPort(ClientTargetPort),
      threadCount(ThreadCount)
{
    auto const address = net::ip::make_address(serverBindAddress);
    uint16_t   port    = serverBindPort;

    startThreadsAndIoContext();

    server = std::make_shared<RelayServer>(*ioc_server, net::ip::tcp::endpoint{address, port});
    server->setRequestPassingFunctor([this](const RequestType& req) -> ResponseType {
        if (derived().validateRequest(req)) {
            std::shared_ptr<ClientSession> client = std::make_shared<ClientSession>(*ioc_client);
            client->run(clientTargetAddress, std::to_string(clientTargetPort), req);
            return client->getResponse().get();
        } else {
            return make_response_bad_request(req, "Failed to validate request\n");
        }
    });
    server->run();
}

template <typename Derived>
void Relay<Derived>::stop()
{
    ioc_server_work.reset();
    ioc_client_work.reset();
}

#endif // RELAY_H
