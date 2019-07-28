#ifndef RELAYSERVER_H
#define RELAYSERVER_H

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

public:
    RelayServer(net::io_context& ioc, net::ip::tcp::endpoint endpoint);

    // Start accepting incoming connections
    void run();

private:
    void do_accept();
    void on_accept(boost::beast::error_code ec, net::ip::tcp::socket socket);
};

#endif // RELAYSERVER_H
