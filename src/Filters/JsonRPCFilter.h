#ifndef JSONRPCFILTER_H
#define JSONRPCFILTER_H

#include <boost/beast/http.hpp>
#include <string>
#include <unordered_set>

class JsonRPCFilter
{
    std::unordered_set<std::string> allowedMethods;

public:
    JsonRPCFilter();

    bool operator()(const boost::beast::http::request<boost::beast::http::string_body>& req);

    void addAllowedMethod(const std::string& methodName);
    void removeAllowedMethodIfExists(const std::string& methodName);
    bool allowedMethodExists(const std::string& methodName);
};

#endif // JSONRPCFILTER_H
