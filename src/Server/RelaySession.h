#ifndef RELAYSESSION_H
#define RELAYSESSION_H

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <iostream>
#include <memory>

namespace net = boost::asio; // from <boost/asio.hpp>

using RequestType  = boost::beast::http::request<boost::beast::http::string_body>;
using ResponseType = boost::beast::http::response<boost::beast::http::string_body>;

extern std::function<ResponseType(const RequestType&)> RequestValidatorFunctor;

boost::beast::http::response<boost::beast::http::string_body> make_response_bad_request(const RequestType&       req,
                                                                                        const boost::string_view why);

template <typename Body, typename Allocator, typename Send>
void handle_request(boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>&& req, Send&& send)
{
    return send(std::move(RequestValidatorFunctor(req)));
}

class RelaySession : public std::enable_shared_from_this<RelaySession>
{
    // This is the C++11 equivalent of a generic lambda.
    // The function object is used to send an HTTP message.
    struct send_lambda
    {
        RelaySession& self_;

        explicit send_lambda(RelaySession& self) : self_(self) {}

        template <bool isRequest, typename Body, typename Fields>
        void operator()(boost::beast::http::message<isRequest, Body, Fields>&& msg) const
        {
            // The lifetime of the message has to extend
            // for the duration of the async operation so
            // we use a shared_ptr to manage it.
            auto sp = std::make_shared<boost::beast::http::message<isRequest, Body, Fields>>(std::move(msg));

            // Store a type-erased version of the shared
            // pointer in the class to keep it alive.
            self_.res_ = sp;

            // Write the response
            boost::beast::http::async_write(
                self_.stream_, *sp,
                boost::beast::bind_front_handler(&RelaySession::on_write, self_.shared_from_this(), sp->need_eof()));
        }
    };

    boost::beast::tcp_stream                                     stream_;
    boost::beast::flat_buffer                                    buffer_;
    boost::beast::http::request<boost::beast::http::string_body> req_;
    std::shared_ptr<void>                                        res_;
    send_lambda                                                  lambda_;

public:
    // Take ownership of the stream
    RelaySession(net::ip::tcp::socket&& socket) : stream_(std::move(socket)), lambda_(*this) {}

    // Start the asynchronous operation
    void run();

    void do_read();

    void on_read(boost::beast::error_code ec, std::size_t bytes_transferred);

    void on_write(bool close, boost::beast::error_code ec, std::size_t bytes_transferred);

    void do_close();

    /**
     * set the function that validates whether the request should be passed further to the
     * @brief SetReqValidatorFunctor
     * @param func is the function object
     */
    static void SetReqValidatorFunctor(const std::function<ResponseType(const RequestType&)>& func);
};

#endif // RELAYSESSION_H
