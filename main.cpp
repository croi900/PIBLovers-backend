

#include "ppbase.h"
#include "rest.h"
#include <iostream>
#include <unordered_map>
#include <thread>
// HTTP

using namespace httplib;

pp::database* db;
#define dbase (*db)

inline unsigned char from_hex (unsigned char ch) 
{
    if (ch <= '9' && ch >= '0')
        ch -= '0';
    else if (ch <= 'f' && ch >= 'a')
        ch -= 'a' - 10;
    else if (ch <= 'F' && ch >= 'A')
        ch -= 'A' - 10;
    else 
        ch = 0;
    return ch;
}

const std::string urldecode (const std::string& str) 
{
    using namespace std;
    string result;
    string::size_type i;
    for (i = 0; i < str.size(); ++i)
    {
        if (str[i] == '+')
        {
            result += ' ';
        }
        else if (str[i] == '%' && str.size() > i+2)
        {
            const unsigned char ch1 = from_hex(str[i+1]);
            const unsigned char ch2 = from_hex(str[i+2]);
            const unsigned char ch = (ch1 << 4) | ch2;
            result += ch;
            i += 2;
        }
        else
        {
            result += str[i];
        }
    }
    return result;
}

using namespace std;
string url_decode(string text) {
    char h;
    ostringstream escaped;
    escaped.fill('0');

    for (auto i = text.begin(), n = text.end(); i != n; ++i) {
        string::value_type c = (*i);

        if (c == '%') {
            if (i[1] && i[2]) {
                h = from_hex(i[1]) << 4 | from_hex(i[2]);
                escaped << h;
                i += 2;
            }
        } else if (c == '+') {
            escaped << ' ';
        } else {
            escaped << c;
        }
    }

    return escaped.str();
}

void process_body_add(std::string body){
        std::cout<<body<<" "<<std::endl;

    std::string name = body.substr (body.find('&'));
    std::string json = body.substr (0, body.find('&'));
    name = name.substr (name.find('=')+1);
    json = json.substr (json.find('=')+1);
    // std::cout << body << "\n~~~~~\n";

    // std::cout << name << "\n~~~~~\n";

    // std::cout << json << "\n~~~~~\n";
    // std::cout << urldecode(json) << "\n~~~~~\n";
    // std::cout << url_decode(json) << "\n~~~~~\n";

    dbase[name] = json::parse(urldecode(json));

}

std::string process_body_find(std::string body){
    std::string name = body.substr (body.find('=')+1);

    if(dbase.has_entry(name))
        return std::string("{\"exists\": 1}");
    else
        return std::string("{\"exists\": 0}");
}

std::string process_body_get(std::string body){
    std::string name = body.substr (body.find('=')+1);

    if(dbase.has_entry(name))
        return dbase[name].dump();
    else
        return std::string("{}");
}

std::string process_body_addPoints(std::string body){

    std::cout << body << std::endl;
    std::string name = body.substr(body.find('&'));
    std::string amount_s = body.substr(0, body.find('&'));
    name = name.substr(name.find('=')+1);
    amount_s = amount_s.substr(amount_s.find('=')+1);
    auto amount = std::stol(amount_s);

    if(dbase.has_entry(name + ".points")){
        if(dbase[name + ".points"]["points"].is_null() == false){
            if(dbase[name + ".points"]["points"].is_number_integer() == true){
                // std::cout<<name + ".points"<<" " <<  dbase[name + ".points"]["points"] << std::endl;
                dbase[name + ".points"]["points"] = dbase[name + ".points"].value("points", 0) + amount;
                return std::string(std::to_string(dbase[name + ".points"].value("points", 0)) + "success");
            }else{
                return std::string("222: points field is not integer");
            }
        }else{
            return std::string("111: no points field in entry");
        }
    }else{
        return std::string("000: no entry in db");
    }
}

void startserver(){
    httplib::Server svr;

    svr.Get("/database", [](const httplib::Request & req, httplib::Response &res) {
                res.set_header("Access-Control-Allow-Origin", "*");

        res.set_content("Hello World!", "text/plain");
    });

    svr.Post("/get",
    [&](const Request &req, Response &res, const ContentReader &content_reader) {
        if (req.is_multipart_form_data()) {
        // NOTE: `content_reader` is blocking until every form data field is read
        MultipartFormDataItems files;
        content_reader(
            [&](const MultipartFormData &file) {
            files.push_back(file);
            return true;
            },
            [&](const char *data, size_t data_length) {
            files.back().content.append(data, data_length);
            std::cout<<data;
            return true;
            });
        } else {
        std::string body;
        content_reader([&](const char *data, size_t data_length) {
            body.append(data, data_length);
            return true;
        });
        res.set_header("Access-Control-Allow-Origin", "*");
            res.set_content(process_body_get(body),"json/application");
        }
    });

    svr.Post("/addPoints",
    [&](const Request &req, Response &res, const ContentReader &content_reader) {
        if (req.is_multipart_form_data()) {
        // NOTE: `content_reader` is blocking until every form data field is read
        MultipartFormDataItems files;
        content_reader(
            [&](const MultipartFormData &file) {
            files.push_back(file);
            return true;
            },
            [&](const char *data, size_t data_length) {
            files.back().content.append(data, data_length);
            std::cout<<data;
            return true;
            });
        } else {
        std::string body;
        content_reader([&](const char *data, size_t data_length) {
            body.append(data, data_length);
            return true;
        });
        res.set_header("Access-Control-Allow-Origin", "*");
            res.set_content(process_body_addPoints(body),"text/plain");
            dbase.update_database();
        }
    });

    svr.Post("/find",
    [&](const Request &req, Response &res, const ContentReader &content_reader) {
        if (req.is_multipart_form_data()) {
        // NOTE: `content_reader` is blocking until every form data field is read
        MultipartFormDataItems files;
        content_reader(
            [&](const MultipartFormData &file) {
            files.push_back(file);
            return true;
            },
            [&](const char *data, size_t data_length) {
            files.back().content.append(data, data_length);
            std::cout<<data;
            return true;
            });
        } else {
        std::string body;
        content_reader([&](const char *data, size_t data_length) {
            body.append(data, data_length);
            return true;
        });
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(process_body_find(body),"text/plain");
        dbase.update_database();
        }
    });

    svr.Post("/add",
    [&](const Request &req, Response &res, const ContentReader &content_reader) {
        if (req.is_multipart_form_data()) {
        // NOTE: `content_reader` is blocking until every form data field is read
        MultipartFormDataItems files;
        content_reader(
            [&](const MultipartFormData &file) {
            files.push_back(file);
            return true;
            },
            [&](const char *data, size_t data_length) {
            files.back().content.append(data, data_length);
            std::cout<<data;
            return true;
            });
        } else {
        std::string body;
        content_reader([&](const char *data, size_t data_length) {
            body.append(data, data_length);
            return true;
        });
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(body,"text/plain");
        process_body_add(body);
        dbase.update_database();

        }
    });

    svr.listen("0.0.0.0", 8081);
}



int main(){
    db = pp::make_database("database");

    //std::cout << dbase["coaie"];

    startserver();
}