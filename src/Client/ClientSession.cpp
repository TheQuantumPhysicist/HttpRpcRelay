#include "ClientSession.h"

#include "Logging/DefaultLogger.h"

ClientSession::ClientSession(boost::asio::io_context& ioc)
    : resolver_(net::make_strand(ioc)), stream_(net::make_strand(ioc))
{
}

void ClientSession::run(http::verb                                verb,
                        const std::string&                        host,
                        const std::string&                        port,
                        const std::string&                        target,
                        const std::string&                        body,
                        int                                       version,
                        const std::map<std::string, std::string>& fields)
{
    // Set up an HTTP GET request message
    req_.version(version);
    req_.method(verb);
    req_.target(target);
    req_.set(http::field::host, host);
    req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req_.set(http::field::content_type, "application/json");
    req_.body() = body;
    req_.set(http::field::content_length, body.size());
    for (const auto& f : fields) {
        req_.insert(f.first, f.second);
    }

    // Look up the domain name
    resolver_.async_resolve(
        host, port, beast::bind_front_handler(&ClientSession::on_resolve, shared_from_this()));
}

void ClientSession::run(const std::string&                             host,
                        const std::string&                             port,
                        boost::beast::http::request<http::string_body> request)
{
    req_ = std::move(request);

    resolver_.async_resolve(
        host, port, beast::bind_front_handler(&ClientSession::on_resolve, shared_from_this()));
}

void ClientSession::on_resolve(beast::error_code ec, tcp::resolver::results_type results)
{
    if (ec) {
        LogWrite("Failed to resolve: " + ec.message(), b_sev::err);
        finished_promise.set_exception(std::make_exception_ptr(boost::system::system_error(ec)));
        return;
    }

    // Set a timeout on the operation
    stream_.expires_after(std::chrono::seconds(60));

    // Make the connection on the IP address we get from a lookup
    stream_.async_connect(results,
                          beast::bind_front_handler(&ClientSession::on_connect, shared_from_this()));
}

void ClientSession::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type)
{
    if (ec) {
        LogWrite("Failed to connect: " + ec.message(), b_sev::err);
        finished_promise.set_exception(std::make_exception_ptr(boost::system::system_error(ec)));
        return;
    }

    // Set a timeout on the operation
    stream_.expires_after(std::chrono::seconds(30));

    // Send the HTTP request to the remote host
    http::async_write(
        stream_, req_, beast::bind_front_handler(&ClientSession::on_write, shared_from_this()));
}

void ClientSession::on_write(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        LogWrite("Failed to write: " + ec.message(), b_sev::err);
        finished_promise.set_exception(std::make_exception_ptr(boost::system::system_error(ec)));
        return;
    }

    // Receive the HTTP response
    http::async_read(
        stream_, buffer_, res_, beast::bind_front_handler(&ClientSession::on_read, shared_from_this()));
}

void ClientSession::on_read(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        LogWrite("Failed to read: " + ec.message(), b_sev::err);
        finished_promise.set_exception(std::make_exception_ptr(boost::system::system_error(ec)));
        return;
    }

    // Gracefully close the socket
    stream_.socket().shutdown(tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes so don't bother reporting it.
    if (ec && ec != beast::errc::not_connected) {
        LogWrite("Failed to shutdown: " + ec.message(), b_sev::err);
        finished_promise.set_exception(std::make_exception_ptr(boost::system::system_error(ec)));
        return;
    }

    finished_promise.set_value(res_);
    // If we get here then the connection is closed gracefully
}

std::future<http::response<http::string_body>> ClientSession::getResponse()
{
    return finished_promise.get_future();
}
