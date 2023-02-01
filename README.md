# SpiceQL
[![Documentation Status](https://readthedocs.org/projects/sugar-spice/badge/?version=latest)](http://sugar-spice.readthedocs.io/?badge=latest) [![CMake](https://github.com/USGS-Astrogeology/SpiceQL/actions/workflows/ctests.yml/badge.svg)](https://github.com/USGS-Astrogeology/SpiceQL/actions/workflows/ctests.yml)

This Library provides a C++ interface querying, reading and writing Naif SPICE kernels. Built on the [Naif Toolkit](https://naif.jpl.nasa.gov/naif/toolkit.html).


## Building The Library

The library leverages anaconda to maintain all of it's dependencies. So in order to build SpiceQL, you'll need to have Anaconda installed.

> **NOTE**:If you already have Anaconda installed, skip to step 3.

1. Download either the Anaconda or Miniconda installation script for your OS platform. Anaconda is a much larger distribtion of packages supporting scientific python, while Miniconda is a minimal installation and not as large: Anaconda installer, Miniconda installer
1. If you are running on some variant of Linux, open a terminal window in the directory where you downloaded the script, and run the following commands. In this example, we chose to do a full install of Anaconda, and our OS is Linux-based. Your file name may be different depending on your environment.
   * If you are running Mac OS X, a pkg file (which looks similar to Anaconda3-5.3.0-MacOSX-x86_64.pkg) will be downloaded. Double-click on the file to start the installation process.
1. Open a Command line prompt and run the following commands:

```bash
# Clone the Github repo, note the recursive flag, this library depends on
# submodules that also need to be cloned. --recurse-submodules enables this and
# the -j8 flag parallelizes the cloning process.
git clone --recurse-submodules -j8 https://github.com/USGS-Astrogeology/SpiceQL.git

# cd into repo dir
cd SpiceQL

# Create new environment from the provided dependency file, the -n flag is
# proceded by the name of the new environment, change this to whatever works for you
conda env create -f environment.yml -n ssdev

# activate the new env
conda activate ssdev

# make and cd into the build directory. This can be placed anywhere, but here, we make
# it in the repo (build is in .gitingore, so no issues there)
mkdir build
cd build

# Configure the project, install directory can be anything, here, it's the conda env
cmake .. -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX

# Optional: DB files are installed by default in $CONDA_PREFIX/etc/SpiceQL/db to 
# use files that are included within the repo, you must create and define 
# an environment variable named SSPICE_DEBUG. 
# note SSPICE_DEBUG can be set to anything as long as it is defined
export SSPICE_DEBUG=True

# Set the environment variable(s) to point to your kernel install 
# The following environment variables are used by default in order of priority: 
# $SPICEROOT, $ALESPICEROOT, $ISISDATA. 
# SPICEROOT is unique to this lib, while ALESPICEROOT, and ISISDATA are used 
# by both ALE and ISIS respectively. 
# note you can set each of these environment variables path to point to the
# correspoding kernels downloaded location, ie 
SPICEROOT=~/spiceQL/Kernals/spiceRootKernel
ALESPICEROOT=~/spiceQL/Kernals/aleSpiceRootKernel
ISISDATA=~/spiceQL/Kernals/isisData

# build and install project
make install

# Optional, Run tests
ctest -j8
```

You can disable different components of the build by setting the CMAKE variables `SPICEQL_BUILD_DOCS`, `SPICEQL_BUILD_TESTS`, `SPICEQL_BUILD_BINDINGS`, or `SPICEQL_BUILD_LIB` to `OFF`. For example, the following cmake configuration command will not build the documentation or the tests:

```
cmake .. -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX -DSPICEQL_BUILD_DOCS=OFF -DSPICEQL_BUILD_TESTS=OFF
```

## Bindings

The SpiceQL API is available via Python bindings in the module `pyspiceql`. The bindings are built using SWIG and are on by default. You can disable the bindings in your build by setting `SPICEQL_BUILD_BINDINGS` to `OFF` when configuring your build.

## Memoization Header Library 

SpiceQL has a simple memoization header only library at `Spiceql/include/memo.h`. This can cache function results on disk using a binary archive format mapped using a combined hash of a function ID and it's input parameters. 

TLDR 
```C++
#include "memo.h"

int func(int) { ... }
memoization::disk c("cache_path");

// use case 1: wrap function call
// (function ID, the function to wrap and then params
int result1 = c("func_id", func, 3);

// use case 2: wrap function
// (cache object, function ID, function)
auto func_memoed = memoization::make_memoized(c, "func_id", func);
int result2 = func_memoed(3);

assert(result1 == result2);
```


## Building and Testing Lambda Locally

1. Build your image locally using the `docker build` command.
`docker build -f aws/Dockerfile -t spiceql .`

2. Run your container image locally using the `docker run` command.
`docker run -p 9000:8080 -it spiceql`

3. From a new terminal window, post an event to the following endpoint using a `curl` command:
`curl -XGET "http://localhost:9000/2015-03-31/functions/function/invocations" -d '{"func" : "getMissionConfigFile", "mission" : "mro"}'`

## Interacting with Public(VPN) AWS Lambda Function

The URL for interacting with the lambda function is:
`https://spiceql-dev.prod-asc.chs.usgs.gov/v1/spiceql`

Right now there are two query string parameters:

   1. func: The name of the function
   2. mission: the name of the mission

To test these parameters run the following URL in your browser: 
`https://spiceql-dev.prod-asc.chs.usgs.gov/v1/spiceql?func=getMissionConfigFile&mission=mro`

This should return response with the associated config file path.

## Adding/Updating New Query String Parameters in CloudFormation

To add new query string parameters in the CF:

1. Update the following `RequestParameters` under `AWS::ApiGateway::Method`:

   - `method.request.querystring.<ParameterName>: <RequiredBoolean>`

2. Then add the following to `RequestParameters` under `Integration` under `AWS::ApiGateway::Method`:

   - `integration.request.querystring.<ParameterName>: method.request.querystring.<ParameterName>`

 *Please note that updating the CF in AWS does not always update these parameters. If you make changes and are unable to query the new parameters, you may need to delete the CF stack and create a new one with your updated template.*

 3. The lambda handler takes in an "event" that needs to be formatted in json. The API Gateway takes in these query string parameter results and formats them in json in order to be digested into the lambda function. 

   - To add in new parameters into the json, update the `RequestTemplate` under `Integrations` under `AWS::ApiGateway::Method`:

   ```
   "application/json": "{"func": "$input.param('func')", 
                         "mission": "$input.params('mission')", 
                         "<ParameterName>": "$input.params('<ParameterName>')" }"
   ```


