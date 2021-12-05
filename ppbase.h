#ifndef PPBASE_INCLUDED
#define PPBASE_INCLUDED
#include <string>
#include <unordered_map>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cassert>
#include "json.h"
namespace fs = std::filesystem;
using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
using json = nlohmann::json;

inline bool file_exists(const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

bool dir_exists(const fs::path& p, fs::file_status s = fs::file_status{})
{
    if(fs::status_known(s) ? fs::exists(s) : fs::exists(p))
        return true;
    else
        return false;
}

std::string get_stem(const fs::path &p) { return (p.stem().string()); }


namespace pp{
    

    std::string database_name = "";

    class database{
        public:
        std::unordered_map<std::string,json> idb;
        json& operator[](std::string entry){
            if(idb.find(entry) != idb.end()){
                return idb[entry];
            }else{
                if(!file_exists(database_name + "/" + entry)){
                    std::ofstream ne(database_name + "/" + entry);
                    ne<<"{}";
                    idb[entry] = json::parse("{}");
                    return idb[entry];
                }else{
                    std::ifstream in(database_name + "/" + entry);
                    idb[entry] = json::parse(in);
                    return idb[entry];
                }   
            }
        }

        void update_database(){
            for(auto & entry : idb){
                std::ofstream out(database_name + "/" + entry.first);
                out<<entry.second.dump();
            }
        }

        bool has_entry(std::string name){
            return file_exists(database_name + "/" + name);
        }
    };
    
    database * make_database(std::string name){
        //assert(dir_exists(name) == 0);
        if(!dir_exists(name))
            fs::create_directory(name);
        database_name = name;
        database* db = new database();

        for (const auto& entry : recursive_directory_iterator(name))
        {
            std::cout<<entry.path().string().substr(entry.path().string().find('/')+1)<<std::endl;
            std::ifstream in(entry.path().string());
            (*db)[entry.path().string().substr(entry.path().string().find('/')+1)] = json::parse(in);
        }

        return db;
    }
    
}

#endif