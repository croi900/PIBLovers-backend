#include "rest.h"

// HTTP
httplib::Client cli("0.0.0.0:8080");

// HTTPS
#include <iostream>
#include <fstream>

int main(){

    std::ifstream in("test/coaie");
    std::stringstream buffer;
    buffer << in.rdbuf();
    httplib::Params params{
        { "name", "coaie" },
        {"amount", "1923"}
       // { "json", buffer.str() }
    };
    
    auto res = cli.Post("/addPoints", params);
    std::cout<<res->body;

}