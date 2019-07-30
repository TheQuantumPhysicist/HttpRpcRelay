#include "RelaySession.h"

#include "Logging/DefaultLogger.h"

boost::beast::http::response<boost::beast::http::string_body>
make_response_bad_request(const RequestType& req, const boost::string_view why)
{
    boost::beast::http::response<boost::beast::http::string_body> res{
        boost::beast::http::status::bad_request, req.version()};
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();
    return res;
}

boost::beast::http::response<boost::beast::http::string_body>
make_response_server_error(const RequestType& req, const boost::string_view why)
{
    boost::beast::http::response<boost::beast::http::string_body> res{
        boost::beast::http::status::service_unavailable, req.version()};
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();
    return res;
}

void RelaySession::run() { do_read(); }

void RelaySession::do_read()
{
    // Make the request empty before reading,
    // otherwise the operation behavior is undefined.
    req_ = {};

    // Set the timeout.
    stream_.expires_after(std::chrono::seconds(60));

    // Read a request
    boost::beast::http::async_read(
        stream_,
        buffer_,
        req_,
        boost::beast::bind_front_handler(&RelaySession::on_read, shared_from_this()));
}

void RelaySession::on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    // This means they closed the connection
    if (ec == boost::beast::http::error::end_of_stream) {
        return do_close();
    }

    if (ec) {
        LogWrite("Failed to read: " + ec.message(), b_sev::err);
        return;
    }

    // Send the response
    handle_request(std::move(req_), lambda_);
}

void RelaySession::on_write(bool close, boost::beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        LogWrite("Failed to write: " + ec.message(), b_sev::err);
        return;
    }

    if (close) {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        return do_close();
    }

    // We're done with the response so delete it
    res_ = nullptr;

    // Read another request
    do_read();
}

void RelaySession::do_close()
{
    // Send a TCP shutdown
    boost::beast::error_code ec;
    stream_.socket().shutdown(net::ip::tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully
}
