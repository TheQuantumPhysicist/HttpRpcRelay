#ifndef CLIENTSESSION_H
#define CLIENTSESSION_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http  = beast::http;          // from <boost/beast/http.hpp>
namespace net   = boost::asio;          // from <boost/asio.hpp>
using tcp       = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

class ClientSession : public std::enable_shared_from_this<ClientSession>
{
    tcp::resolver                                   resolver_;
    beast::tcp_stream                               stream_;
    beast::flat_buffer                              buffer_; // (Must persist between reads)
    http::request<http::string_body>                req_;
    http::response<http::string_body>               res_;
    std::promise<http::response<http::string_body>> finished_promise;

public:
    // Objects are constructed with a strand to
    // ensure that handlers do not execute concurrently.
    explicit ClientSession(net::io_context& ioc);

    // Start the asynchronous operation
    void run(boost::beast::http::verb verb, const std::string& host, const std::string& port, const std::string& target,
             const std::string& body, int version,
             const std::map<std::string, std::string>& fields = std::map<std::string, std::string>());

    // Start the asynchronous operation
    void run(const std::string& host, const std::string& port,
             boost::beast::http::request<boost::beast::http::string_body> request);

    void on_resolve(beast::error_code ec, tcp::resolver::results_type results);

    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type);

    void on_write(beast::error_code ec, std::size_t bytes_transferred);

    void on_read(beast::error_code ec, std::size_t bytes_transferred);

    std::future<http::response<http::string_body>> getResponse();
};

#endif // CLIENTSESSION_H
