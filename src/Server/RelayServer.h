#ifndef RELAYSERVER_H
#define RELAYSERVER_H

#include "Logging/DefaultLogger.h"
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <iostream>
#include <memory>

#include "RelaySession.h"

class RelayServer : public std::enable_shared_from_this<RelayServer>
{
    boost::asio::io_context&       ioc_;
    boost::asio::ip::tcp::acceptor acceptor_;

    std::function<ResponseType(const RequestType&)> requestPassingFunctor = [](const RequestType& req) -> ResponseType {
        LogWrite("No validation function set; returning false by default, i.e., all requests are rejected",
                 b_sev::warn);
        return make_response_bad_request(req, "Handler not set");
    };

public:
    RelayServer(net::io_context& ioc, net::ip::tcp::endpoint endpoint);

    // Start accepting incoming connections
    void run();

    /**
     * set the function that validates whether the request should be passed further to the
     * @brief SetReqValidatorFunctor
     * @param func is the function object
     */
    void setRequestPassingFunctor(const std::function<ResponseType(const RequestType&)>& func);

private:
    void do_accept();
    void on_accept(boost::beast::error_code ec, net::ip::tcp::socket socket);
};

#endif // RELAYSERVER_H
