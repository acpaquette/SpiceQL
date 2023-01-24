#include <fstream>
#include <gtest/gtest.h>

#include <cstdlib>

#include "Fixtures.h"

#include "query.h"
#include "utils.h"

using namespace std;
using namespace SpiceQL;


TEST_F(LroKernelSet, FunctionalTestSearchMissionKernels) {
  setenv("SPICEROOT", root.c_str(), true);

  // load all available kernels
  nlohmann::json kernels = listMissionKernels(root, conf);

  // do a time query
  kernels = searchMissionKernels(kernels, {110000000, 120000001}, false);
  kernels = getLatestKernels(kernels);

  ASSERT_EQ(fs::path(kernels["moc"]["spk"]["smithed"]["kernels"][0].get<string>()).filename(), "LRO_TEST_GRGM660MAT270.bsp" );
  ASSERT_EQ(fs::path(kernels["moc"]["ck"]["reconstructed"]["kernels"][0].get<string>()).filename(), "soc31.0001.bc" );
  ASSERT_EQ(fs::path(kernels["moc"]["ik"]["kernels"][0].get<string>()).filename(), "lro_instruments_v11.ti");
  ASSERT_EQ(fs::path(kernels["moc"]["fk"]["kernels"][0].get<string>()).filename(), "lro_frames_1111111_v01.tf");
  ASSERT_EQ(fs::path(kernels["moc"]["sclk"]["kernels"][0].get<string>()).filename(), "lro_clkcor_2020184_v00.tsc");
}