#include "Logging/DefaultLogger.h"
#include "Relay/JsonRpcRelay.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <atomic>
#include <boost/program_options.hpp>
#include <cstdlib>
#include <thread>
#include <vector>

std::atomic<bool> g_ShutdownProgram{false};

void interrupt_handler(int)
{
    if (!g_ShutdownProgram) {
        LogWrite("Signal sent to stop application. Setting stop flag.", b_sev::info);
        g_ShutdownProgram.store(true);
    }
}

int main(int argc, char* argv[])
{
    g_ShutdownProgram.store(false);
    signal(SIGINT, interrupt_handler);

    namespace params = boost::program_options;

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    LoggerSingleton::get().add_stream(console_sink, b_sev::info);

    params::options_description desc("Program options");
    // clang-format off
    desc.add_options()("help", "produce help message")
            ("bind_address", params::value<std::string>(), "Server bind address (e.g., 127.0.0.1 or 0.0.0.0)")
            ("bind_port", params::value<uint16_t>(),"Server bind port")
            ("target_address", params::value<std::string>(),"Target address to send requests to that pass")
            ("target_port", params::value<uint16_t>(),"Target port to send requests that pass")
            ("filter_kind", params::value<std::string>(),"Filter kind to be used; default is jsonrpc filter")
            ("filter_options", params::value<std::string>(),"Filter definitions based on the filter you choose (for jsonrpc, it's a comma separated list of allowed methods)")
            ("threads", params::value<uint32_t>(),"Number of threads to use in the application");
    // clang-format on

    params::variables_map vm;
    params::store(params::parse_command_line(argc, argv, desc), vm);
    params::notify(vm);

    std::string server_bind_address = vm["bind_address"].as<std::string>();
    uint16_t    server_bind_port    = vm["bind_port"].as<uint16_t>();
    std::string target_bind_address = vm["target_address"].as<std::string>();
    uint16_t    target_bind_port    = vm["target_port"].as<uint16_t>();
    uint32_t    thread_count        = std::thread::hardware_concurrency();
    if (vm.find("threads") != vm.cend()) {
        thread_count = vm["threads"].as<uint32_t>();
    }
    if (vm.find("filter_kind") != vm.cend()) {
        // currently there's only jsonrpc filter, so this is no-op
    }
    if (vm.find("filter_options") == vm.cend()) {
        throw std::runtime_error("The argument filter_options should be specified");
    }
    std::string filter_options = vm["filter_options"].as<std::string>();

    /////////// start the server

    JsonRPCFilter filter;
    filter.applyOptions(filter_options);

    JsonRpcRelay relay(std::move(filter),
                       server_bind_address,
                       server_bind_port,
                       target_bind_address,
                       target_bind_port,
                       thread_count);

    while (!g_ShutdownProgram.load(std::memory_order_relaxed)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    relay.stop();

    return EXIT_SUCCESS;
}
