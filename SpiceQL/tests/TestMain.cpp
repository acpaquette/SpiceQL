#include <gtest/gtest.h>
#include "Fixtures.h"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include <spdlog/spdlog.h>

int main(int argc, char **argv) {
   spdlog::set_level(spdlog::level::trace);
   spdlog::set_pattern("SpiceQL-Tests [%H:%M:%S %z] %v"); 
   
   testing::Environment* const spiceql_env = testing::AddGlobalTestEnvironment(new TempTestingFiles);

   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}