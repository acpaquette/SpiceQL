#include "config.h"
#include "query.h"
#include <time.h>

#include <fstream>
#include <sstream>
#include <spdlog/spdlog.h>

#include <ghc/fs_std.hpp>

using namespace std;
using json = nlohmann::json;

namespace SpiceQL {

  Config::Config() {
    string dbPath = getConfigDirectory(); 
    vector<string> json_paths = glob(dbPath, ".json");

    for(const fs::path &p : json_paths) {
      ifstream i(p);
      json j;
      i >> j;
      for (auto it = j.begin(); it != j.end(); ++it) {
        config[it.key()] = it.value();
      }
    }
  }


  Config::Config(string j) {
    std::ifstream ifs(j);
    config = json::parse(ifs);
  }

  
  Config::Config(json j, string pointer) {
    config = j;
    confPointer = pointer;
  }


  Config Config::operator[](string pointer) {
    if (pointer.at(0) != '/') {
      pointer = "/"+pointer; 
    }

    json::json_pointer p(pointer);
    json::json_pointer pbase(confPointer);
    pointer = (pbase / p).to_string();
    
    Config conf(config, pointer);
    return conf;
  }

  Config Config::operator[](vector<string> pointers) {
    json eval_json;

    for (auto &pointer : pointers) {
      json j = config[pointer];
      eval_json[pointer] = j;
    }

    return Config(eval_json, "");
  }

  string Config::getParentPointer(string searchPointer, int pointerPosition) {
    if (pointerPosition < 0) {
      throw exception();
    }

    json::json_pointer pointer(searchPointer);
    json::json_pointer configPointer(confPointer);

    json::json_pointer pathMod;
    vector<string> deconstructedPointer = {""};

    while (pointer != "") {
      deconstructedPointer.push_back(pointer.back());
      pointer.pop_back();
    }
    std::reverse(deconstructedPointer.begin(), deconstructedPointer.end());
    if (pointerPosition > deconstructedPointer.size()) {
      throw exception();
    }

    for (int i = 0; i < pointerPosition; i++) {
      pathMod /= deconstructedPointer[i];
    }
    json::json_pointer fullPointer = pathMod;

    // If there is some dependency at the pointer requested, return that instead
    string depPath = json::json_pointer(getRootDependency(config, fullPointer.to_string()));
    if (depPath != "") {
      return depPath;
    }

    return pathMod.to_string();
  }


  json Config::evaluateConfig(string pointerToEval) {
    json::json_pointer pointer(confPointer);
    if (pointerToEval != "") {
      pointer = json::json_pointer(pointerToEval);
    }
    json copyConfig(config);

    json::json_pointer parentPointer;
    string dataPath = getDataDirectory();

    json eval_json(copyConfig);
    resolveConfigDependencies(eval_json, config);
    eval_json = eval_json[pointer];

    vector<json::json_pointer> json_to_eval = SpiceQL::findKeyInJson(eval_json, "kernels", true);

    for (auto json_pointer:json_to_eval) {
      json::json_pointer full_pointer = pointer / json_pointer;

      fs::path fsDataPath(dataPath);
      json::json_pointer kernelPath(getParentPointer(full_pointer, 1));
      if (fs::exists((string)fsDataPath + kernelPath.to_string())) {
        fsDataPath += kernelPath.to_string();
        kernelPath = json::json_pointer("/kernels");
        string kernelType = json::json_pointer(getParentPointer(full_pointer, 2)).back();
        kernelPath /= kernelType;
        if (fs::exists((string)fsDataPath + kernelPath.to_string())) {
          fsDataPath += kernelPath.to_string();
        }
      }

      vector<string> res = getPathsFromRegex(fsDataPath, eval_json[json_pointer]);
      eval_json[json_pointer] = res;
    }
    copyConfig[pointer] = eval_json;

    return copyConfig;
  }


  unsigned int Config::size() {
    json::json_pointer cpointer(confPointer);
    return config[cpointer].size();
  }


  json Config::getRecursive(string key) {
    vector<string> pointers = findKey(key, true);
    json res;

    for (auto &pointer : pointers) {
      json::json_pointer p(pointer);
      json::json_pointer cpointer(confPointer);
      json::json_pointer evalPointer = (cpointer / p);
      res[evalPointer] = get(p.to_string())[p];
    }

    return res;
  }


  json Config::getLatestRecursive(string key) {
    json res = getRecursive(key);
    return getLatestKernels(res);
  }


  json Config::get(string pointer) {
    json::json_pointer getConfPointer(confPointer);

    if (pointer != "") {
      if (pointer.at(0) != '/')
      {
        pointer = "/" + pointer;
      }
      json::json_pointer p(pointer);
      json::json_pointer pbase(getConfPointer);
      getConfPointer = (pbase / p);
    }
    json res = evaluateConfig(getConfPointer.to_string());
    return res[json::json_pointer(confPointer)];
  }


  json Config::getLatest(string pointer) {
    json res;
    res[pointer] = get(pointer)[pointer];
    return getLatestKernels(res);
  }


  json Config::get(vector<string> pointers) {
    json eval_json;

    for (auto &pointer : pointers) {
      json j = get(pointer)[pointer];
      eval_json[pointer] = j;
    }
    return eval_json;
  }


  json Config::globalConf() {
    json::json_pointer cpointer(confPointer);
    return config[cpointer];
  }


  vector<string> Config::findKey(string key, bool recursive) {
    json::json_pointer cpointer(confPointer);

    vector<string> pointers;
    json subConf = config[cpointer];
    vector<json::json_pointer> ptrs = SpiceQL::findKeyInJson(subConf, key, recursive);
    for(auto &e : ptrs) {
      pointers.push_back(e.to_string());
    }

    return pointers; 
  }

}
