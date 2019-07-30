#include "EasyClient.h"

EasyClient::EasyClient()
{
    startThreadsAndIoContext();
    client = std::make_shared<ClientSession>(*ioc);
}

void EasyClient::run(http::verb                                verb,
                     const std::string&                        host,
                     const std::string&                        port,
                     const std::string&                        target,
                     const std::string&                        body,
                     int                                       version,
                     const std::map<std::string, std::string>& fields)
{
    client->run(verb, host, port, target, body, version, fields);
}

void EasyClient::run(const std::string&                             host,
                     const std::string&                             port,
                     boost::beast::http::request<http::string_body> request)
{
    client->run(host, port, request);
}

std::future<http::response<http::string_body>> EasyClient::getResponse()
{
    return client->getResponse();
}

void EasyClient::startThreadsAndIoContext()
{
    ioc_work.reset();
    thread.reset(); // destroy any running threads
    ioc.reset();

    ioc      = std::make_unique<net::io_context>(1);
    ioc_work = std::make_unique<net::io_context::work>(*ioc);

    thread = std::unique_ptr<std::thread, decltype(threadDestructor)>(
        new std::thread([this] { ioc->run(); }), threadDestructor);
}
