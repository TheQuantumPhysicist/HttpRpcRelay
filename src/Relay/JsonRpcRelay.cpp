#include "JsonRpcRelay.h"

JsonRpcRelay::JsonRpcRelay(JsonRPCFilter&& Filter, std::string ServerBindAddress, uint16_t ServerBindPort,
                           std::string ClientTargetAddress, uint16_t ClientTargetPort, uint32_t ThreadCount)
    : Relay(ServerBindAddress, ServerBindPort, ClientTargetAddress, ClientTargetPort, ThreadCount), filter(Filter)
{
}

bool JsonRpcRelay::validateRequest(const RequestType& request) { return filter(request); }
