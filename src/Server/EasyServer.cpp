#include "EasyServer.h"

EasyServer::EasyServer(const std::string& BindAddress, uint16_t BindPort, uint32_t Threads)
    : bindAddress(BindAddress), bindPort(BindPort), threadCount(Threads >= 1 ? Threads : 1)
{
    startThreadsAndIoContext();
    server = std::make_shared<RelayServer>(
        *ioc, net::ip::tcp::endpoint{net::ip::make_address(bindAddress), bindPort});
}

void EasyServer::setRequestResponseFunctor(const std::function<ResponseType(const RequestType&)>& func)
{
    server->setRequestPassingFunctor(func);
}

void EasyServer::run() { server->run(); }

void EasyServer::startThreadsAndIoContext()
{
    ioc_work.reset();
    threadsVector.clear(); // destroy any running threads
    ioc.reset();

    ioc      = std::make_unique<net::io_context>(threadCount);
    ioc_work = std::make_unique<net::io_context::work>(*ioc);

    threadsVector.reserve(threadCount);
    for (auto i = threadCount; i > 0; --i) {
        threadsVector.emplace_back(std::unique_ptr<std::thread, decltype(threadDestructor)>(
            new std::thread([this] { ioc->run(); }), threadDestructor));
    }
}
