# HttpRpcRelay

### Introduction
This software is a simple link + filter between a client and an RPC system. It's prepared to take jsonrpc, but it can be modified, as it's design in a generic way where new protocols can be added by just creating a one class with one method, and change the program options to use it.

### What is a use-case for this library?

Consider if you're running a system that requires high security, such as a cryptocurrency wallet. For management purposes, you need to communicate with this wallet, but not with the full potential. Not everyone needs to have permission to withdraw from this wallet. However, the cryptocurrency gives JsonRPC API access to anyone connected to it. Encryption passwords may help, but someone with 24/7 access to the wallet might find the password, or simply wait until the wallet is unlocked for any withdrawal.

**How do we solve this?**

You place HttpRpcRelay on that server, and you block all communication to the RPC with a firewall except through this HttpRpcRelay. In HttpRpcRelay options, you define the methods that clients are allowed to use. Now, no one can call any undesired methods.


# Dependencies
AsyncJsonRPC depends on two libraries:
1. libjsoncpp
2. boost beast, asio and algorithm
3. spdlog (comes with it as a submodule)
4. gtest for testing (also comes as a submodule)

**Conan as a dependency manager**: Conan retrieves boost for you and compiles it automatically for you. It's not necessary if you want to use your system version of boost. Feel free to change the `CMakeLists.txt` file and remove conan.


### Thread, memory, undefined behavior and other safety checks

For quality assurance, you can build the project with clang-sanitizers enable. Please enable one only at a time. The following are the CMake options to enable:

- `SANITIZE_THREAD`
- `SANITIZE_UNDEFINED`
- `SANITIZE_ADDRESS`
- `SANITIZE_LEAK`

For example, run cmake with `-DSANITIZE_LEAK=ON` to enable leak sanitizer.

### Building
You can build this software by standard cmake compilation steps.

To clone it, simple: `git clone --recursive https://github.com/TheQuantumPhysicist/HttpRpcRelay`

Then, in the cloned directory:
```sh
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j10
./bin/HttpRpcRelay --help  # to see the options of the program
```
