/**
 * Implements an almost transparent disk cache/memoization for C++ functions.
 *
 * Published under three-clause BSD license.
 * Copyright 2014 Hannes Schulz <schulz@ais.uni-bonn.de>
 */
#ifndef memo_h
#define memo_h

#include <map>
#include <fstream>
#include <utility>
#include <functional>
#include <any>

#include <ghc/fs_std.hpp>
#include <spdlog/spdlog.h>

#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>


#define CACHED(cache, func, ...) cache(#func, func, __VA_ARGS__)

namespace memoization {
    template <class T>
    inline size_t _hash_combine(std::size_t& seed, const T& v) {
        spdlog::trace("_hash_combine seed: {}", seed);

        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);

        return seed;
    }

    inline size_t hash_combine(std::size_t& seed) {
        return seed;
    }

    template <typename T, typename... Params>
    inline size_t hash_combine(std::size_t& seed, const T& t, const Params&... params) {
        seed = _hash_combine(seed, t);
        return hash_combine(seed, params...);
    }


    struct disk {
        fs::path m_path;
        disk(std::string path = fs::current_path().string())
        :m_path(fs::path(path) / "cache") {
            fs::create_directories(m_path);
        }

        template<typename Func, typename... Params>
            auto operator()(const Func& f, Params&&... params) -> decltype(f(params...))const{
                return this->operator()("anonymous", f, std::forward<Params>(params)...);
            }
        template<typename Func, typename... Params>
            auto operator()(const std::string& descr, const Func& f, Params&&... params) -> decltype(f(params...))const {
                spdlog::trace("descr: {}", descr);
                std::size_t seed = 0; 
                hash_combine(seed, descr, params...);
                spdlog::debug("seed: ", seed); 
                return this->operator()(descr, seed, f, std::forward<Params>(params)...);
            }
        template<typename Func, typename... Params>
            auto operator()(const std::string& descr, std::size_t seed, const Func& f, Params&&... params) -> decltype(f(params...))const{
                typedef decltype(f(params...)) retval_t;
                std::string fn = descr + "-" + std::to_string(seed);
                spdlog::debug("fn: ", fn);
                
                fn = (m_path / fn).string();
                if(fs::exists(fn)){
                    std::ifstream ifs(fn);
                    cereal::BinaryInputArchive  ia(ifs);
                    retval_t ret;
                    ia >> ret;
                    spdlog::debug("Cached access from file ");
                    return ret;
                }
                retval_t ret = f(std::forward<Params>(params)...);
                spdlog::debug("Non-cached access, file ");
                std::ofstream ofs(fn);
                cereal::BinaryOutputArchive oa(ofs);
                oa << ret;
                return ret;
            }
    };

    struct memory {
        mutable std::map<std::size_t, std::any> m_data;

        template<typename Func, typename... Params>
            auto operator()(const Func& f, Params&&... params) -> decltype(f(params...)) const {
                return (*this)("anonymous", f, std::forward<Params>(params)...);
            }
        template<typename Func, typename... Params>
            auto operator()(std::string descr, const Func& f, Params&&... params) -> decltype(f(params...)) const {
                std::size_t seed = hash_combine(0, descr, params...);
                return (*this)(seed, f, std::forward<Params>(params)...);
            }
        template<typename Func, typename... Params>
            auto operator()(const std::string& descr, std::size_t seed, const Func& f, Params&&... params) -> decltype(f(params...)) const {
                hash_combine(seed, descr);
                return (*this)(seed, f, std::forward<Params>(params)...);
            }
        template<typename Func, typename... Params>
            auto operator()(std::size_t seed, const Func& f, Params&&... params) -> decltype(f(params...)) const {
                typedef decltype(f(params...)) retval_t;
                auto it = m_data.find(seed);
                if(it != m_data.end()){
                    spdlog::debug("Cached access from memory");
                    return std::any_cast<retval_t>(it->second);
                }
                retval_t ret = f(std::forward<Params>(params)...);
                spdlog::debug("Non-cached access");
                m_data[seed] = ret;
                return ret;
            }
    };


    template<typename Cache, typename Function>
    struct memoize{
        const Function& m_func; // we require copying the function object here.
        std::string m_id;
        Cache& m_fc;
        memoize(Cache& fc, std::string id, const Function& f)
            :m_func(f), m_id(id), m_fc(fc){}
        template<typename... Params>
        auto operator()(Params&&... args) 
                -> decltype(std::bind(m_func, args...)()) {
            spdlog::debug("Calling a memo func with ID: {}", m_id);
            return m_fc(m_id, m_func, std::forward<Params>(args)...);
        }
    };
    template<class Cache, class Function>
    struct registry{
        static std::map<Function, std::pair<std::string, Cache*> > data;
    };
    template<class Cache, class Function>
    std::map<Function, std::pair<std::string, Cache*> >
    registry<Cache, Function>::data;
        
    template<typename Cache, typename Function>
    memoize<Cache, Function>
    make_memoized(Cache& fc, const std::string& id, Function f){
        spdlog::debug("making function with ID {}",id);   
        
        typedef registry<Cache,Function> reg_t;
        auto it = reg_t::data.find(f);
        if(it == reg_t::data.end()){
            spdlog::debug("registering {} in registry", id);
            reg_t::data[f] = std::make_pair(id, &fc);
        }
        return memoize<Cache, Function>(fc, id, f);
    }

    template<typename Cache, typename Function, typename...Args>
    auto
    memoized(Function f, Args&&... args) -> decltype(std::bind(f, args...)()){
        typedef registry<Cache,Function> reg_t;
        auto it = reg_t::data.find(f);
        if(it == reg_t::data.end())
            throw std::runtime_error("memoize function is not registered with a cache");
        std::string id;
        Cache* fc;
        std::tie(id,fc) = it->second;
        return memoize<Cache, Function>(*fc, id, f)(std::forward<Args>(args)...);
    }

}

namespace std {
  // literally have no idea why I need to make this specialization... can't compile otherwise
  template <> struct hash<std::vector<std::__cxx11::basic_string<char> >(const std::__cxx11::basic_string<char>&, bool)>
  {
    size_t operator()(const auto & x) const {
      return 0;
    }
  };
}

#endif