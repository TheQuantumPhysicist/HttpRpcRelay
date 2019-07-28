#include "JsonRPCFilter.h"

#include "JsonStringQueue.h"
#include "Logging/DefaultLogger.h"
#include <iostream>
#include <jsoncpp/json/json.h>

JsonRPCFilter::JsonRPCFilter() {}

bool JsonRPCFilter::operator()(const boost::beast::http::request<boost::beast::http::string_body>& req)
{
    try {
        std::string body = req.body();
        {
            JsonStringQueue jsonStringQueue; // used to ensure that only one json command is there
            jsonStringQueue.pushData(body);
            auto vec = jsonStringQueue.pullDataAndClear();
            if (vec.size() == 0) {
                LogWrite("No json input found in body: " + body, b_sev::warn);
                return false;
            }
            if (vec.size() > 1) {
                LogWrite("Multiple json calls were found in the body: " + body, b_sev::warn);
                return false;
            }
            assert(vec.size() == 1);
        }

        Json::Reader reader;
        Json::Value  root;
        if (!reader.parse(body, root, false)) {
            LogWrite("Failed to parse json: " + body, b_sev::warn);
            return false;
        }

        if (!root.isMember("method")) {
            LogWrite("Failed to find method key in json: " + body, b_sev::warn);
            return false;
        }

        std::string methodName = root["method"].asString();

        auto it = allowedMethods.find(methodName);

        if (it == allowedMethods.cend()) {
            // method is not in the list of allowed methods, return false
            LogWrite("The following jsonrpc with method is not allowed, but was attempted to be executed: " + body,
                     b_sev::warn);
            return false;
        }

        return true;

    } catch (std::exception& ex) {
        LogWrite("Error (std::exception) while testing json data: " + std::string(ex.what()), b_sev::warn);
        return false;
    } catch (...) {
        LogWrite("Error (unknown exception) while testing json data", b_sev::warn);
        return false;
    }
}

void JsonRPCFilter::addAllowedMethod(const std::string& methodName) { allowedMethods.insert(methodName); }

void JsonRPCFilter::removeAllowedMethodIfExists(const std::string& methodName) { allowedMethods.erase(methodName); }

bool JsonRPCFilter::allowedMethodExists(const std::string& methodName)
{
    return allowedMethods.find(methodName) != allowedMethods.cend();
}
