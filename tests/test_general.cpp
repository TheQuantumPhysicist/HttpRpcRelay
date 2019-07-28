#include "gtest/gtest.h"

#include "Client/ClientSession.h"
#include "Filters/JsonRPCFilter.h"
#include "Server/RelayServer.h"
#include <future>
#include <string>

#include <boost/asio/io_context.hpp>

std::string GenerateRandomString__test(const int len)
{
    static const char alphanum[] = "0123456789"
                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "abcdefghijklmnopqrstuvwxyz";

    std::string s;
    s.resize(len);

    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return s;
}

TEST(Relay, ServerAndClient)
{
    auto const address = net::ip::make_address("127.0.0.1");
    uint16_t   port    = 3001;
    int        threads = 4;

    net::io_context ioc{threads};

    RelaySession::SetReqValidatorFunctor([](const RequestType& req) -> ResponseType {
        if (req.body() == "{}") {
            boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::ok,
                                                                              req.version()};
            res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(boost::beast::http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = std::string("Success!");
            res.prepare_payload();
            return res;
        } else {
            return make_response_bad_request(req, "Invalid body!");
        }
    });

    std::shared_ptr<RelayServer> server = std::make_shared<RelayServer>(ioc, net::ip::tcp::endpoint{address, port});
    server->run();

    auto threadDestructor = [&ioc](std::thread* t) {
        ioc.stop();
        t->join();
        delete t;
    };

    std::vector<std::unique_ptr<std::thread, decltype(threadDestructor)>> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i) {
        v.emplace_back(std::unique_ptr<std::thread, decltype(threadDestructor)>(new std::thread([&ioc] { ioc.run(); }),
                                                                                threadDestructor));
    }

    {
        std::shared_ptr<ClientSession> client = std::make_shared<ClientSession>(ioc);
        client->run(boost::beast::http::verb::get, "127.0.0.1", std::to_string(port), "/", "{}", 11);

        auto responseFuture = client->getResponse();

        while (responseFuture.wait_for(std::chrono::milliseconds(10)) != std::future_status::ready) {
        }

        auto response = responseFuture.get();
        EXPECT_EQ(response.result_int(), 200);
    }

    {
        std::shared_ptr<ClientSession> client = std::make_shared<ClientSession>(ioc);
        client->run(boost::beast::http::verb::get, "127.0.0.1", std::to_string(port), "/", "xxx", 11);

        auto responseFuture = client->getResponse();

        while (responseFuture.wait_for(std::chrono::milliseconds(10)) != std::future_status::ready) {
        }

        auto response = responseFuture.get();
        EXPECT_EQ(response.result_int(), 400);
    }
}

TEST(Relay, ServerAndClientWithFilter)
{
    auto const address = net::ip::make_address("127.0.0.1");
    uint16_t   port    = 3001;
    int        threads = 4;

    net::io_context ioc{threads};

    JsonRPCFilter filter;
    filter.addAllowedMethod("thatmethod");

    RelaySession::SetReqValidatorFunctor([&filter](const RequestType& req) -> ResponseType {
        if (filter(req)) {
            boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::ok,
                                                                              req.version()};
            res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(boost::beast::http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = std::string("Success!");
            res.prepare_payload();
            return res;
        } else {
            return make_response_bad_request(req, "Invalid body!");
        }
    });

    std::shared_ptr<RelayServer> server = std::make_shared<RelayServer>(ioc, net::ip::tcp::endpoint{address, port});
    server->run();

    auto threadDestructor = [&ioc](std::thread* t) {
        ioc.stop();
        t->join();
        delete t;
    };

    std::vector<std::unique_ptr<std::thread, decltype(threadDestructor)>> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i) {
        v.emplace_back(std::unique_ptr<std::thread, decltype(threadDestructor)>(new std::thread([&ioc] { ioc.run(); }),
                                                                                threadDestructor));
    }

    {
        // allowed method, this should be OK

        std::string body = R"({"jsonrpc": "2.0", "method": "thatmethod", "params": [42, 23], "id": 1})";

        std::shared_ptr<ClientSession> client = std::make_shared<ClientSession>(ioc);
        client->run(boost::beast::http::verb::get, "127.0.0.1", std::to_string(port), "/", body, 11);

        auto responseFuture = client->getResponse();

        while (responseFuture.wait_for(std::chrono::milliseconds(10)) != std::future_status::ready) {
        }

        auto response = responseFuture.get();
        EXPECT_EQ(response.result_int(), 200);
    }

    {
        // unknown method, should fail

        std::string body = R"({"jsonrpc": "2.0", "method": "subtract", "params": [42, 23], "id": 1})";

        std::shared_ptr<ClientSession> client = std::make_shared<ClientSession>(ioc);
        client->run(boost::beast::http::verb::get, "127.0.0.1", std::to_string(port), "/", body, 11);

        auto responseFuture = client->getResponse();

        while (responseFuture.wait_for(std::chrono::milliseconds(10)) != std::future_status::ready) {
        }

        auto response = responseFuture.get();
        EXPECT_EQ(response.result_int(), 400);
    }

    {
        // parse error

        std::string body = R"({"jsonrpc": "2.0" "method": "thatmethod", "params": [42, 23], "id": 1})";

        std::shared_ptr<ClientSession> client = std::make_shared<ClientSession>(ioc);
        client->run(boost::beast::http::verb::get, "127.0.0.1", std::to_string(port), "/", body, 11);

        auto responseFuture = client->getResponse();

        while (responseFuture.wait_for(std::chrono::milliseconds(10)) != std::future_status::ready) {
        }

        auto response = responseFuture.get();
        EXPECT_EQ(response.result_int(), 400);
    }

    {
        // method doesn't exists

        std::string body = R"({"jsonrpc": "2.0", "params": [42, 23], "id": 1})";

        std::shared_ptr<ClientSession> client = std::make_shared<ClientSession>(ioc);
        client->run(boost::beast::http::verb::get, "127.0.0.1", std::to_string(port), "/", body, 11);

        auto responseFuture = client->getResponse();

        while (responseFuture.wait_for(std::chrono::milliseconds(10)) != std::future_status::ready) {
        }

        auto response = responseFuture.get();
        EXPECT_EQ(response.result_int(), 400);
    }

    {
        // multiple json calls

        std::string body =
            R"({"jsonrpc": "2.0", "params": [42, 23], "id": 1}{"jsonrpc": "2.0", "params": [42, 23], "id": 1})";

        std::shared_ptr<ClientSession> client = std::make_shared<ClientSession>(ioc);
        client->run(boost::beast::http::verb::get, "127.0.0.1", std::to_string(port), "/", body, 11);

        auto responseFuture = client->getResponse();

        while (responseFuture.wait_for(std::chrono::milliseconds(10)) != std::future_status::ready) {
        }

        auto response = responseFuture.get();
        EXPECT_EQ(response.result_int(), 400);
    }
}
