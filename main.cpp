#include "Server/RelayServer.h"
#include <cstdlib>
#include <thread>
#include <vector>

int main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc != 5) {
        std::cerr << "Usage: http-server-async <address> <port> <threads>\n"
                  << "Example:\n"
                  << "http-server-async 0.0.0.0 8080 . 1\n";
        return EXIT_FAILURE;
    }
    auto const address = net::ip::make_address(argv[1]);
    auto const port    = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const threads = std::max<int>(1, std::atoi(argv[3]));

    // The io_context is required for all I/O
    net::io_context ioc{threads};

    // Create and launch a listening port
    std::make_shared<RelayServer>(ioc, net::ip::tcp::endpoint{address, port})->run();

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back([&ioc] { ioc.run(); });
    ioc.run();

    return EXIT_SUCCESS;
}
