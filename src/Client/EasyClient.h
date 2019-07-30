#ifndef EASYCLIENT_H
#define EASYCLIENT_H

#include "ClientSession.h"

class EasyClient
{
    std::unique_ptr<net::io_context> ioc;
    std::unique_ptr<net::io_context::work> ioc_work;
    std::shared_ptr<ClientSession> client;
    std::unique_ptr<std::thread, std::function<void(std::thread*)>> thread;

    std::function<void(std::thread*)> threadDestructor = [this](std::thread* t) {
        assert(ioc != nullptr);
        ioc->stop();
        t->join();
        delete t;
    };

    void startThreadsAndIoContext();
public:
    EasyClient();

    // Start the asynchronous operation
    void run(boost::beast::http::verb verb, const std::string& host, const std::string& port, const std::string& target,
             const std::string& body, int version,
             const std::map<std::string, std::string>& fields = std::map<std::string, std::string>());

    // Start the asynchronous operation
    void run(const std::string& host, const std::string& port,
             boost::beast::http::request<boost::beast::http::string_body> request);

    std::future<http::response<http::string_body>> getResponse();
};

#endif // EASYCLIENT_H
