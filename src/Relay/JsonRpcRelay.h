#ifndef JSONRPCRELAY_H
#define JSONRPCRELAY_H

#include "Relay.h"

#include "Filters/JsonRPCFilter.h"

class JsonRpcRelay : public Relay<JsonRpcRelay>
{
    JsonRPCFilter filter;

public:
    JsonRpcRelay(JsonRPCFilter&& Filter, std::string ServerBindAddress, uint16_t ServerBindPort,
                 std::string ClientTargetAddress, uint16_t ClientTargetPort,
                 uint32_t ThreadCount = std::thread::hardware_concurrency());

    bool validateRequest(const RequestType& request);
};

#endif // JSONRPCRELAY_H
