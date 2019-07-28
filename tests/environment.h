#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "gtest/gtest.h"

#include "Logging/DefaultLogger.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include <boost/core/ignore_unused.hpp>
#include <boost/shared_ptr.hpp>
#include <fstream>
#include <sstream>
#include <thread>

class Environment : public ::testing::Environment
{
public:
    virtual ~Environment() {}
    virtual void SetUp()
    {
        //        ::testing::GTEST_FLAG(catch_exceptions) = false;
        srand(time(NULL));

        // a file as sink
        LoggerSingleton::get().add_file("test_log.log", b_sev::trace);
        LoggerSingleton::get().getInternalLogger()->flush_on(spdlog::level::critical);
        LoggerSingleton::get().getInternalLogger()->flush_on(spdlog::level::err);

        // stderr as sink
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        LoggerSingleton::get().add_stream(console_sink, b_sev::trace);
    }

    virtual void TearDown() {}
};

::testing::Environment* const env = ::testing::AddGlobalTestEnvironment(new Environment);

template <typename T>
void ignore_it(T)
{
    boost::ignore_unused(env);
}

#endif // ENVIRONMENT_H
