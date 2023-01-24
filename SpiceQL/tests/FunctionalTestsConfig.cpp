#include <fstream>

#include <gtest/gtest.h>

#include "Fixtures.h"

#include "config.h"
#include "utils.h"

using namespace std;
using json = nlohmann::json;
using namespace SpiceQL;

TEST_F(TestConfig, FunctionalTestConfigConstruct) {
  json megaConfig = testConfig.globalConf();

  ASSERT_EQ(megaConfig.size(), 33);
}

TEST_F(TestConfig, FunctionalTestConfigEval) {
  fs::path dbPath = getMissionConfigFile("clem1");

  ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(paths);
  mocks.OnCallFunc(getDataDirectory).Return("/isis_data/");

  json config_eval_res = testConfig.get();
  json pointer_eval_res = testConfig.get("/clementine1");

  json::json_pointer pointer = "/clementine1/ck/reconstructed/kernels"_json_pointer;
  int expected_number = 4;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(pointer_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(testConfig[pointer].size(), 1);

  pointer = "/clementine1/ck/smithed/kernels"_json_pointer;
  expected_number = 1;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(pointer_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(testConfig[pointer].size(), expected_number);

  pointer = "/clementine1/spk/reconstructed/kernels"_json_pointer;
  expected_number = 2;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(pointer_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(testConfig[pointer].size(), 1);

  pointer = "/clementine1/fk/kernels"_json_pointer;
  expected_number = 1;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(pointer_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(testConfig[pointer].size(), expected_number);

  pointer = "/clementine1/sclk/kernels"_json_pointer;
  expected_number = 2;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(pointer_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(testConfig[pointer].size(), 1);


  pointer = "/uvvis/ik/kernels"_json_pointer;
  expected_number = 1;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(testConfig[pointer].size(), expected_number);

  pointer = "/uvvis/iak/kernels"_json_pointer;
  expected_number = 2;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(testConfig[pointer].size(), 1);
}


TEST_F(TestConfig, FunctionalTestConfigGlobalEval) {
  fs::path dbPath = getMissionConfigFile("clem1");

  ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(paths);
  mocks.OnCallFunc(getDataDirectory).Return("/isis_data/");

  testConfig.get();
  json config_eval_res = testConfig.globalConf();

  json::json_pointer pointer = "/clementine1/ck/reconstructed/kernels"_json_pointer;
  int expected_number = 1;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);

  pointer = "/clementine1/ck/smithed/kernels"_json_pointer;
  expected_number = 1;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);

  pointer = "/clementine1/spk/reconstructed/kernels"_json_pointer;
  expected_number = 1;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);

  pointer = "/clementine1/fk/kernels"_json_pointer;
  expected_number = 1;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);

  pointer = "/clementine1/sclk/kernels"_json_pointer;
  expected_number = 1;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);

  pointer = "/uvvis/ik/kernels"_json_pointer;
  expected_number = 1;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);

  pointer = "/uvvis/iak/kernels"_json_pointer;
  expected_number = 1;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);
}

TEST_F(TestConfig, FunctionalTestConfigAccessors) {
  Config base = testConfig["base"];
  Config base_pointer = testConfig["/base"];
  
  MockRepository mocks; 
  mocks.OnCallFunc(SpiceQL::ls).Return(paths);

  EXPECT_EQ(base.get()["lsk"]["kernels"].at(0), "/isis_data/base/kernels/sclk/naif0001.tls");
  EXPECT_EQ(base_pointer.get()["lsk"]["kernels"].at(0), "/isis_data/base/kernels/sclk/naif0001.tls");
}

TEST_F(TestConfig, FunctionalTestsConfigKeySearch) {
  vector<json::json_pointer> pointers = {"/my/first/pointer"_json_pointer, "/my/second/pointer"_json_pointer};

  MockRepository mocks;
  mocks.OnCallFunc(SpiceQL::findKeyInJson).Return(pointers);

  vector<string> res_pointers = testConfig.findKey("kernels", true);

  int i = 0;
  for (auto pointer : res_pointers) {
    EXPECT_EQ(pointer, pointers[i++]);
  }
}

TEST_F(TestConfig, FunctionalTestsConfigGetParentPointer) {
  MockRepository mocks;
  mocks.OnCallFunc(SpiceQL::getRootDependency).Return("");

  std::string pointer = "/banana/apple/orange";

  std::string parent = testConfig.getParentPointer(pointer, 0);
  EXPECT_EQ(parent, "");

  parent = testConfig.getParentPointer(pointer, 1);
  EXPECT_EQ(parent, "/banana");

  parent = testConfig.getParentPointer(pointer, 2);
  EXPECT_EQ(parent, "/banana/apple");

  parent = testConfig.getParentPointer(pointer, 3);
  EXPECT_EQ(parent, pointer);
}

TEST_F(TestConfig, FunctionalTestsConfigGetParentPointerDeps) {
  std::string folder = getenv("SPICEROOT");
  folder += "/lro";
  ASSERT_TRUE(fs::create_directory(folder));
  std::string pointer = "/lroc/sclk";

  std::string parent = testConfig.getParentPointer(pointer, 1);
  EXPECT_EQ(parent, "/lro");

  parent = testConfig.getParentPointer(pointer, 2);
  EXPECT_EQ(parent, "/lroc/sclk");
  ASSERT_TRUE(fs::remove(folder));
}