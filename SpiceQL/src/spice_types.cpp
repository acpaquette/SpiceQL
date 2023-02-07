/**
  * @file
  *
  *
 **/

#include <fmt/format.h>
#include <SpiceUsr.h>

#include <nlohmann/json.hpp>

#include <ghc/fs_std.hpp>

#include <spdlog/spdlog.h>

#include "spice_types.h"
#include "query.h"
#include "utils.h"
#include "config.h"

using namespace std;
using json = nlohmann::json;

namespace SpiceQL {

  /**
   * @brief Used here to do reverse lookups of enum stringss
   **/
  template < typename T> pair<bool, int > findInVector(const std::vector<T>  & vecOfElements, const T  & element) {
    pair<bool, int > result;
    auto it = find(vecOfElements.begin(), vecOfElements.end(), element);
    if (it != vecOfElements.end()) {
      result.second = distance(vecOfElements.begin(), it);
      result.first = true;
    }
    else {
      result.first = false;
      result.second = -1;
    }
    return result;
  }


  const std::vector<std::string> Kernel::TYPES =  { "na", "ck", "spk", "tspk",
                                                    "lsk", "mk", "sclk",
                                                    "iak", "ik", "fk",
                                                    "dsk", "pck", "ek"};

  const std::vector<std::string> Kernel::QUALITIES = { "na",
                                                       "predicted",
                                                       "nadir",
                                                       "reconstructed",
                                                       "smithed"};


  string Kernel::translateType(Kernel::Type type) {
    return Kernel::TYPES[static_cast<int>(type)];
  }


  Kernel::Type Kernel::translateType(string type) {
    auto res = findInVector<string>(Kernel::TYPES, type);
    if (res.first) {
      return static_cast<Kernel::Type>(res.second);
    }

    throw invalid_argument(fmt::format("{} is not a valid kernel type", type));
  };


  string Kernel::translateQuality(Kernel::Quality qa) {
    return Kernel::QUALITIES[static_cast<int>(qa)];
  }


  Kernel::Quality Kernel::translateQuality(string qa) {
    auto res = findInVector<string>(Kernel::QUALITIES, qa);
    if (res.first) {
      return static_cast<Kernel::Quality>(res.second);
    }

    throw invalid_argument(fmt::format("{} is not a valid kernel type", qa));
  }


  int Kernel::translateFrame(string frame, string mission) {
    // KernelPool::getInstance();
    
    SpiceInt code;
    SpiceBoolean found;
    // Get FKs 
    // load all latest until we can get smarter about it
    Config c;
    json config = c.globalConf();
    json j;
    if (mission == ""){
      mission = frame;
    }
    if (config.find(mission) != config.end()) {
      vector<string> kernelsToGet = {"fk", "ik", "iak"};
      j = c[mission].get(kernelsToGet);
      json missionKernels = {};
      missionKernels["fk"] = j["fk"];
      missionKernels["ik"] = j["ik"];
      missionKernels["iak"] = j["iak"];
      j = getLatestKernels(missionKernels);
    }
    else {
      spdlog::warn("Could not find mission: \"{}\" in config. \n Either double-check mission variable or manually furnish kernels.", mission);
    }
    KernelSet kset(j);

    checkNaifErrors();
    bodn2c_c(frame.c_str(), &code, &found);
    checkNaifErrors();

    if (!found) {
      string missionKeys = getMissionKeys(config);
      throw invalid_argument(fmt::format("Frame code for frame name \"{}\" not Found. \n Try including frame and mission name. List of available missions: [{}]. ", frame, missionKeys));
    }

    return code;
  }


  string Kernel::translateFrame(int frame) {
    KernelPool::getInstance(); 

    SpiceChar name[128];
    SpiceBoolean found;

    checkNaifErrors();
    bodc2n_c(frame, 128, name, &found);
    checkNaifErrors();

    if(!found) {
      throw invalid_argument(fmt::format("Frame name for code {} not Found", frame));
    }

    return string(name);
  }


  Kernel::Kernel(string path) {
    this->path = path;
    KernelPool::getInstance().load(path, true);
  }


  Kernel::Kernel(Kernel &other) {
    KernelPool::getInstance().load(other.path);
    this->path = other.path;
  }


  Kernel::~Kernel() {
    KernelPool::getInstance().unload(this->path);
  }


  double utcToEt(string utc) {
      // get lsk kernel
      json conf = getMissionConfig("base");
      conf = globKernels(getDataDirectory(), conf, "lsk");
      KernelSet lsk(getLatestKernel(conf.at("base").at("lsk").at("kernels")));

      SpiceDouble et;
      checkNaifErrors();
      utc2et_c(utc.c_str(), &et);
      checkNaifErrors();

      return et;
  }

  double strSclkToEt(int frameCode, string mission, string sclk) {
      // get lsk kernel
      Config missionConf;
      json globalConf = missionConf.globalConf();
      json sclks;
      if (globalConf.find(mission) != globalConf.end()) {
        spdlog::debug("Found {} in config, getting only {} sclks.", mission, mission);
        missionConf = missionConf[mission];
        sclks = missionConf.getLatest("sclk")["sclk"];
      }
      else {
        spdlog::debug("Coudn't find {} in config explicitly, loading all sclk kernels", mission);
        sclks = missionConf.getLatestRecursive("sclk");
      }
      KernelSet sclkSet(sclks);

      SpiceDouble et;
      checkNaifErrors();
      scs2e_c(frameCode, sclk.c_str(), &et);
      checkNaifErrors();
      spdlog::debug("strsclktoet({}, {}, {}) -> {}", frameCode, mission, sclk, et);
      
      return et;
  }


  int KernelPool::load(string path, bool force_refurnsh) {
    spdlog::debug("Furnishing {}, force refurnish? {}.", path, force_refurnsh);

    int refCount; 

    auto it = refCounts.find(path);

    if (it != refCounts.end()) {
      spdlog::debug("{} already furnished.", path);


      // it's been furnished before, increment ref count
      it->second += 1;
      refCount = it->second; 
 
      if (force_refurnsh) {
        checkNaifErrors();
        furnsh_c(path.c_str());
        checkNaifErrors();

      } 
    }
    else { 
      refCount = 1;  
      // load the kernel and register in onto the kernel map 
      checkNaifErrors();
      furnsh_c(path.c_str());
      checkNaifErrors();
      refCounts.emplace(path, 1);
    }


    spdlog::debug("refcout: {}", refCount);
    return refCount;
  }


  int KernelPool::unload(string path) {
    try { 
      int &refcount = refCounts.at(path);
      
      // if the map contains the last copy of the kernel, delete it
      if (refcount == 1) {
        // unfurnsh the kernel
        checkNaifErrors();
        unload_c(path.c_str());
        checkNaifErrors();
        
        refCounts.erase(path);
        return 0;
      }
      else {
        checkNaifErrors();
        unload_c(path.c_str());
        checkNaifErrors();
        
        refcount--;
        
        return refcount;
      }
    }
    catch(out_of_range &e) {
      throw out_of_range(path + " is not a kernel that has been loaded."); 
    }
  }


  unsigned int KernelPool::getRefCount(std::string key) {
    try {
      return refCounts.at(key);
    } catch(out_of_range &e) {
      return 0;
    }
  }


  unordered_map<string, int> KernelPool::getRefCounts() {
    return refCounts;
  }


  KernelPool &KernelPool::getInstance() {
    static KernelPool pool;
    return pool;
  }


  KernelPool::KernelPool() : refCounts() { 
    loadLeapSecondKernel();

    checkNaifErrors();
    // create aliases for spacecrafts 
    boddef_c("mess", -236); // NAIF uses MESSENGER, we use mess for short
    checkNaifErrors();
  
  }


  vector<string> KernelPool::getLoadedKernels() {
    vector<string> res;

    for( const auto& [key, value] : refCounts ) {
      res.emplace_back(key);
    }
    return res;
  }

  void KernelPool::loadClockKernels() { 
    json clocks;

    // if data dir not set, should raise an exception 
    fs::path dataDir = getDataDirectory();

    vector<json> confs = getAvailableConfigs();
    // get SCLKs
    for(auto &j : confs) {
      vector<json::json_pointer> p = findKeyInJson(j, "sclk", true);
      
      if (!p.empty()) {
        json sclks = j[p.at(0)];
        clocks[p.at(0)] = sclks;
      }
    }
  
    clocks = listMissionKernels(dataDir, clocks);
    clocks = getLatestKernels(clocks);

    vector<json::json_pointer> kpointers = findKeyInJson(clocks, "kernels", true);
    for (auto &p : kpointers) {
        json sclks = clocks[p];
        
        for (auto &e : sclks) { 
          load(e.get<string>());
        }
    }
  }


  void KernelPool::loadLeapSecondKernel() {
    // get the distribution's LSK
    fs::path dbPath = getConfigDirectory();
    string lskPath = dbPath / "kernels" / "naif0011.tls";
    load(lskPath);
  }


  KernelSet::KernelSet(json kernels) {
    this->kernels = kernels;

    vector<json::json_pointer> pointers = findKeyInJson(kernels, "kernels", true);

    for(auto &p : pointers) {
      json jkernels = kernels[p];
      vector<SharedKernel> res;
      for(auto & k : jkernels) {
        SharedKernel sk(new Kernel(k));
        res.emplace_back(sk);
      }
      loadedKernels.emplace(p, res);
    }
  }


} 
 