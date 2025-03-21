#include <fstream>
#include <algorithm>

#include <HippoMocks/hippomocks.h>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include "Fixtures.h"

#include "query.h"
#include "utils.h"


using namespace std;
using namespace SpiceQL;


TEST(QueryTests, UnitTestGetLatestKernel) {
  vector<string> kernels = {
    "iak.0001.ti",
    "iak.0003.ti",
    "different/place/iak.0002.ti",
    "test/iak.0004.ti"
  };

  EXPECT_EQ(getLatestKernel(kernels)[0],  "test/iak.0004.ti");
}

TEST(QueryTests, getKernelStringValue){
  unique_ptr<Kernel> k(new Kernel("data/msgr_mdis_v010.ti"));
  // INS-236810_CCD_CENTER        =  (  511.5, 511.5 )
  string res = getKernelStringValue("INS-236810_FOV_SHAPE");
  EXPECT_EQ(res, "RECTANGLE");

  res = getKernelStringValue("INS-236810_FOV_REF_ANGLE");
  EXPECT_EQ(res, "0.7465");

  try {
    getKernelStringValue("aKeyThatWillNotBeInTheResults");
    FAIL() << "Expected std::invalid_argument";
  }
  catch(std::invalid_argument const & err) {
      EXPECT_EQ(err.what(),std::string("key not in results"));
  }
  catch(...) {
      FAIL() << "Expected std::invalid_argument";
  }
}


TEST(QueryTests, getKernelVectorValue){
  unique_ptr<Kernel> k(new Kernel("data/msgr_mdis_v010.ti"));

  vector<string> actualResultsOne = getKernelVectorValue("INS-236810_CCD_CENTER");
  std::vector<string> expectedResultsOne{"511.5", "511.5"};

  vector<string> actualResultsTwo = getKernelVectorValue("INS-236800_FOV_REF_VECTOR");
  std::vector<string> expectedResultsTwo{"1.0", "0.0", "0.0"};

  for (int i = 0; i < actualResultsOne.size(); ++i) {
    EXPECT_EQ(actualResultsOne[i], expectedResultsOne[i]) << "Vectors x and y differ at index " << i;
  }

  for (int j = 0; j < actualResultsTwo.size(); ++j) {
    EXPECT_EQ(actualResultsTwo[j], expectedResultsTwo[j]) << "Vectors x and y differ at index " << j;
  }

   try {
        getKernelVectorValue("aKeyThatWillNotBeInTheResults");
        FAIL() << "Expected std::invalid_argument";
    }
    catch(std::invalid_argument const & err) {
        EXPECT_EQ(err.what(),std::string("key not in results"));
    }
    catch(...) {
        FAIL() << "Expected std::invalid_argument";
    }
}

TEST(QueryTests, UnitTestGetLatestKernelError) {
  vector<string> kernels = {
    "iak.0001.ti",
    "iak.0003.ti",
    "different/place/iak.0002.ti",
    "test/iak.4.ti",
    // different extension means different filetype and therefore error
    "test/error.tf"
  };

  try {
    getLatestKernel(kernels);
    FAIL() << "expected invalid argument error";
  }
  catch(invalid_argument &e) {
    SUCCEED();
  }
}


TEST_F(KernelDataDirectories, FunctionalTestListMissionKernelsAllMess) {
  string dbPath = getMissionConfigFile("mess");

  ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(paths);

  nlohmann::json res = listMissionKernels("/isis_data/", conf);

  SPDLOG_INFO("res: {}", res.dump());
  vector<string> s = SpiceQL::getKernelsAsVector(res["mdis"]["ck"]["reconstructed"]["kernels"]);
  SPDLOG_INFO("CK Reconstructed: {}", fmt::join(s, ", "));

  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["ck"]["reconstructed"]["kernels"]).size(), 4);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["ck"]["smithed"]["kernels"]).size(), 4);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["spk"]["reconstructed"]["kernels"]).size(), 2);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["tspk"]["kernels"]).size(), 1);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["fk"]["kernels"]).size(), 2);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["ik"]["kernels"]).size(), 2);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["iak"]["kernels"]).size(), 2);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["pck"]["na"]["kernels"]).size(), 2);

  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis_att"]["ck"]["reconstructed"]["kernels"]).size(), 4);

  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["messenger"]["ck"]["reconstructed"]["kernels"]).size(), 5);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["messenger"]["sclk"]["kernels"]).size(), 2);
}


TEST_F(KernelDataDirectories, FunctionalTestListMissionKernelsClem1) {
  fs::path dbPath = getMissionConfigFile("clem1");

  ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(paths);

  nlohmann::json res = listMissionKernels("/isis_data/", conf);

  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["clementine1"]["ck"]["reconstructed"]["kernels"]).size(), 4);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["clementine1"]["ck"]["smithed"]["kernels"]).size(), 1);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["clementine1"]["spk"]["reconstructed"]["kernels"]).size(), 2);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["clementine1"]["fk"]["kernels"]).size(), 1);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["clementine1"]["sclk"]["kernels"]).size(), 2);

  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["uvvis"]["ik"]["kernels"]).size(), 1);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["uvvis"]["iak"]["kernels"]).size(), 2);

  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["uvvis"]["iak"]["kernels"]).size(), 2);
}


TEST_F(KernelDataDirectories, FunctionalTestListMissionKernelsGalileo) {
  fs::path dbPath = getMissionConfigFile("galileo");

  ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(paths);

  nlohmann::json res = listMissionKernels("/isis_data/", conf);

  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["galileo"]["ck"]["reconstructed"]["kernels"]).size(), 4);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["galileo"]["ck"]["smithed"]["kernels"]).size(), 3);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["galileo"]["spk"]["reconstructed"]["kernels"]).size(), 2);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["galileo"]["iak"]["kernels"]).size(), 1);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["galileo"]["pck"]["smithed"]["kernels"]).size(), 2);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["galileo"]["pck"]["na"]["kernels"]).size(), 1);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["galileo"]["sclk"]["kernels"]).size(), 1);
}



TEST_F(KernelDataDirectories, FunctionalTestListMissionKernelsCassini) {
  fs::path dbPath = getMissionConfigFile("cassini");
  ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;
  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(paths);

  nlohmann::json res = listMissionKernels("/isis_data/", conf);

  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["ck"]["reconstructed"]["kernels"]).size(), 2);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["ck"]["smithed"]["kernels"]).size(), 2);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["fk"]["kernels"]).size(), 2);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["iak"]["kernels"]).size(), 3);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["pck"]["kernels"]).size(), 3);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["pck"]["smithed"]["kernels"]).size(), 1);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["sclk"]["kernels"]).size(), 1);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["spk"]["kernels"]).size(), 3);
}



// test for apollo 17 kernels 
TEST_F(IsisDataDirectory, FunctionalTestApollo17Conf) {
  fs::path dbPath = getMissionConfigFile("apollo17");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);
  nlohmann::json res = listMissionKernels("doesn't matter", conf);

  set<string> kernels = getKernelsAsSet(res);
  set<string> expectedKernels = missionMap.at("apollo17");
  set<string> diff; 
  
  set_difference(expectedKernels.begin(), expectedKernels.end(), kernels.begin(), kernels.end(), inserter(diff, diff.begin()));
  
  if (diff.size() != 0) {
    FAIL() << "Kernel sets are not equal, diff: " << fmt::format("{}", fmt::join(diff, " ")) << endl;
  }
}


TEST_F(IsisDataDirectory, FunctionalTestLroConf) {
  compareKernelSets("lro");

  nlohmann::json conf = getMissionConfig("lro");
  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);
  
  nlohmann::json res = listMissionKernels("doesn't matter", conf);
  
  // check a kernel from each regex exists in their quality groups
  vector<string> kernelToCheck =  SpiceQL::getKernelsAsVector(res.at("moc").at("ck").at("reconstructed").at("kernels"));
  vector<string> expected = {"moc42r_2016305_2016336_v01.bc"};
  
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      FAIL() << e << " was not found in the kernel results";
    }
  }
  
  kernelToCheck = getKernelsAsVector(res.at("moc").at("spk").at("reconstructed")); 
  expected = {"fdf29r_2018305_2018335_v01.bsp", "fdf29_2021327_2021328_b01.bsp"};
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      FAIL() << e << " was not found in the kernel results";
    }
  }


  kernelToCheck = getKernelsAsVector(res.at("moc").at("spk").at("smithed")); 
  expected = {"LRO_ES_05_201308_GRGM660PRIMAT270.bsp", "LRO_ES_16_201406_GRGM900C_L600.BSP"};
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      FAIL() << e << " was not found in the kernel results";
    }
  }
}


TEST_F(IsisDataDirectory, FunctionalTestJunoConf) {
  set<string> expectedDiff = {"jup260.bsp",
                              "jup310.bsp",
                              "jup329.bsp",
                              "vgr1_jup230.bsp",
                              "vgr2_jup204.bsp",
                              "vgr2_jup230.bsp"};
  compareKernelSets("juno", expectedDiff);
} 


TEST_F(IsisDataDirectory, FunctionalTestMroConf) {
  compareKernelSets("mro");

  nlohmann::json conf = getMissionConfig("mro");
  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);
  
  nlohmann::json res = listMissionKernels("doesn't matter", conf);

  // check a kernel from each regex exists in there quality groups
  vector<string> kernelToCheck =  getKernelsAsVector(res.at("mro").at("spk").at("reconstructed").at("kernels"));
  vector<string> expected = {"mro_cruise.bsp", "mro_ab.bsp", "mro_psp_rec.bsp", 
                             "mro_psp1.bsp", "mro_psp10.bsp", "mro_psp_rec.bsp", 
                             "mro_psp1_ssd_mro95a.bsp", "mro_psp27_ssd_mro110c.bsp"};
  
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    EXPECT_TRUE(it != kernelToCheck.end());
  }

  kernelToCheck = getKernelsAsVector(res.at("mro").at("spk").at("predicted")); 
  expected = {"mro_psp.bsp"};
  EXPECT_EQ(kernelToCheck, expected);

  kernelToCheck = getKernelsAsVector(res.at("mro").at("ck").at("reconstructed"));
  expected = {"mro_sc_psp_160719_160725.bc", "mro_sc_cru_060301_060310.bc", 
              "mro_sc_ab_060801_060831.bc", "mro_sc_psp_150324_150330_v2.bc"};
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    EXPECT_TRUE(it != kernelToCheck.end());
  }
}


TEST_F(IsisDataDirectory, FunctionalTestViking1Conf) {
  compareKernelSets("viking1");
  // skip specific tests since viking images are mostly literals and without mixed qualities
}


TEST_F(IsisDataDirectory, FunctionalTestViking2Conf) {
  compareKernelSets("viking2");
}


TEST_F(IsisDataDirectory, FunctionalTestMgsConf) {
  
  fs::path dbPath = getMissionConfigFile("mgs");
  
  compareKernelSets("mgs");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);

  nlohmann::json res = listMissionKernels("doesn't matter", conf);

  set<string> kernels = getKernelsAsSet(res);
  set<string> mission = missionMap.at("mgs");
 
  // check a kernel from each regex exists in their quality groups
  vector<string> kernelToCheck =  getKernelsAsVector(res.at("mgs").at("ck").at("reconstructed"));
  vector<string> expected = {"mgs_sc_ab1.bc"};
  
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      FAIL() << e << " was not found in the kernel results";
    }
  }
  
  kernelToCheck = getKernelsAsVector(res.at("mgs").at("iak")); 
  expected = {"mocAddendum001.ti"};
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      FAIL() << e << " was not found in the kernel results";
    }
  }


  kernelToCheck = getKernelsAsVector(res.at("mgs").at("ik")); 
  expected = {"moc20.ti", "tes12.ti", "mola25.ti"};
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      FAIL() << e << " was not found in the kernel results";
    }
  }

  kernelToCheck = getKernelsAsVector(res.at("mgs").at("sclk")); 
  expected = {"MGS_SCLKSCET.00032.tsc"};
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      FAIL() << e << " was not found in the kernel results";
    }
  }

  kernelToCheck = getKernelsAsVector(res.at("mgs").at("spk").at("reconstructed")); 
  expected = {"mgs_ext24.bsp"};
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      FAIL() << e << " was not found in the kernel results";
    }
  }
}

TEST_F(IsisDataDirectory, FunctionalTestOdysseyConf) {
  fs::path dbPath = getMissionConfigFile("odyssey");
  
  compareKernelSets("odyssey");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);

  nlohmann::json res = listMissionKernels("doesn't matter", conf);

  set<string> kernels = getKernelsAsSet(res);
  set<string> mission = missionMap.at("odyssey");

  vector<string> expected = {"m01_sc_ab0110.bc", 
                             "m01_sc_map3_rec_nadir.bc", 
                             "m01_sc_map10_rec_nadir.bc", 
                             "m01_sc_map1_v2.bc",
                             "m01_sc_map10.bc",
                             "m01_sc_ext7_rec_nadir.bc",
                             "m01_sc_ext36_rec_nadir.bc",
                             "m01_sc_ext22_rec_roto_v2.bc",
                             "m01_sc_ext7.bc",
                             "m01_sc_ext42.bc"};
  CompareKernelSets(getKernelsAsVector(res.at("odyssey").at("ck").at("reconstructed")), expected); 

  expected = {"themis_nightir_merged_2018Mar02_ck.bc", 
              "themis_dayir_merged_2018Jul13_ck.bc"};
  CompareKernelSets(getKernelsAsVector(res.at("odyssey").at("ck").at("smithed")), expected); 
  
  expected = {"m01_map.bsp"};
  CompareKernelSets(getKernelsAsVector(res.at("odyssey").at("spk").at("predicted")), expected);  
  
  expected = {"m01_ab_v2.bsp", 
              "m01_map1_v2.bsp",
              "m01_map2.bsp",
              "m01_ext8.bsp",
              "m01_ext23.bsp",
              "m01_map_rec.bsp"};
  CompareKernelSets(getKernelsAsVector(res.at("odyssey").at("spk").at("reconstructed")), expected); 

  expected = {"themis_nightir_merged_2018Mar02_spk.bsp", 
              "themis_dayir_merged_2018Jul13_spk.bsp"};
  CompareKernelSets(getKernelsAsVector(res.at("odyssey").at("spk").at("smithed")), expected);  
}

TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsKaguya) {
  fs::path dbPath = getMissionConfigFile("kaguya");
  
  compareKernelSets("kaguya");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);

  nlohmann::json res = listMissionKernels("doesn't matter", conf);

  set<string> kernels = getKernelsAsSet(res);
  set<string> mission = missionMap.at("kaguya");
  
  vector<string> expected = {"SEL_M_ALL_D_V02.BC"};
  CompareKernelSets(getKernelsAsVector(res.at("kaguya").at("ck").at("reconstructed")), expected); 

  expected = {"SEL_M_071020_081226_SGMI_05.BSP",
              "SELMAINGRGM900CL660DIRALT2008103020090610.bsp",
              "SEL_M_071020_090610_SGMH_02.BSP"};
  CompareKernelSets(getKernelsAsVector(res.at("kaguya").at("spk").at("smithed")), expected); 
}


TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsTgo) {
  fs::path dbPath = getMissionConfigFile("tgo");
  
  compareKernelSets("tgo");
  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);

  nlohmann::json res = listMissionKernels("doesn't matter", conf);

  set<string> kernels = getKernelsAsSet(res);

  set<string> mission = missionMap.at("tgo");
  
  vector<string> expected = {"em16_tgo_sc_fmp_026_01_20200321_20200418_f20180215_v01.bc"};
  CompareKernelSets(getKernelsAsVector(res.at("tgo").at("ck").at("predicted")), expected); 

  expected = {"em16_tgo_sc_ssm_20190210_20190303_s20190208_v01.bc"};
  CompareKernelSets(getKernelsAsVector(res.at("tgo").at("ck").at("reconstructed")), expected); 

  expected = {"em16_tgo_fap_167_01_20160314_20180203_v01.bsp"};
  CompareKernelSets(getKernelsAsVector(res.at("tgo").at("spk").at("predicted")), expected); 

  expected = {"em16_tgo_cassis_ipp_tel_20160407_20170309_s20170116_v01.bc"};
  CompareKernelSets(getKernelsAsVector(res.at("cassis").at("ck").at("predicted")), expected); 

  expected = {"cassis_ck_p_160312_181231_180609.bc",
              "em16_tgo_cassis_tel_20160407_20201231_s20200803_v04.bc"};
  CompareKernelSets(getKernelsAsVector(res.at("cassis").at("ck").at("reconstructed")), expected); 
}


TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsMex) {
  fs::path dbPath = getMissionConfigFile("mex");
  
  compareKernelSets("mex");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);

  nlohmann::json res = listMissionKernels("doesn't matter", conf);

  set<string> kernels = getKernelsAsSet(res);set<string> mission = missionMap.at("mex");
  
  vector<string> expected = {"ATNM_P060401000000_00780.BC"};
  CompareKernelSets(getKernelsAsVector(res.at("mex").at("ck").at("predicted")), expected); 

  expected = {"ATNM_MEASURED_030602_040101_V03.BC",
              "ATNM_RECONSTITUTED_00004.BC"};
  CompareKernelSets(getKernelsAsVector(res.at("mex").at("ck").at("reconstructed")), expected);

  expected = {"ATNM_P060401000000_00780.BC"};
  CompareKernelSets(getKernelsAsVector(res.at("mex").at("ck").at("predicted")), expected); 

  expected = {"ORMF_______________00720.BSP"};
  CompareKernelSets(getKernelsAsVector(res.at("mex").at("spk").at("predicted")), expected);

  expected = {"ORHM_______________00038.BSP"};
  CompareKernelSets(getKernelsAsVector(res.at("mex").at("spk").at("reconstructed")), expected);  

}