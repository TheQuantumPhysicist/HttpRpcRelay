#ifndef EASYSERVER_H
#define EASYSERVER_H

#include "RelayServer.h"

namespace net = boost::asio;

class EasyServer
{
    std::string bindAddress;
    uint16_t bindPort;
    uint32_t threadCount;

    std::unique_ptr<net::io_context> ioc;
    std::unique_ptr<net::io_context::work> ioc_work;
    std::shared_ptr<RelayServer> server;
    std::vector<std::unique_ptr<std::thread, std::function<void(std::thread*)>>> threadsVector;

    std::function<void(std::thread*)> threadDestructor = [this](std::thread* t) {
        assert(ioc != nullptr);
        ioc->stop();
        t->join();
        delete t;
    };

    void startThreadsAndIoContext();
public:
    EasyServer(const std::string& BindAddress, uint16_t BindPort, uint32_t threads = std::thread::hardware_concurrency());
    void setRequestResponseFunctor(const std::function<ResponseType(const RequestType&)>& func);
    void run();
};

#endif // EASYSERVER_H
