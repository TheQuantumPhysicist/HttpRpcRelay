#include "RelayServer.h"

namespace net = boost::asio; // from <boost/asio.hpp>

RelayServer::RelayServer(boost::asio::io_context& ioc, boost::asio::ip::tcp::endpoint endpoint)
    : ioc_(ioc), acceptor_(net::make_strand(ioc))
{
    boost::beast::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
        LogWrite("Failed to open acceptor: " + ec.message(), b_sev::err);
        return;
    }

    // Allow address reuse
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
        LogWrite("Failed to set_option: " + ec.message(), b_sev::err);
        return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if (ec) {
        LogWrite("Failed to bind: " + ec.message(), b_sev::err);
        return;
    }

    // Start listening for connections
    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
        LogWrite("Failed to listen: " + ec.message(), b_sev::err);
        return;
    }
}

void RelayServer::run() { do_accept(); }

void RelayServer::setRequestPassingFunctor(const std::function<ResponseType(const RequestType&)>& func)
{
    requestPassingFunctor = func;
}

void RelayServer::do_accept()
{
    // The new connection gets its own strand
    acceptor_.async_accept(net::make_strand(ioc_),
                           boost::beast::bind_front_handler(&RelayServer::on_accept, shared_from_this()));
}

void RelayServer::on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket)
{
    if (ec) {
        LogWrite("Failed to accept connection: " + ec.message(), b_sev::err);
    } else {
        // Create the session and run it
        std::make_shared<RelaySession>(std::move(socket), requestPassingFunctor)->run();
    }

    // Accept another connection
    do_accept();
}
