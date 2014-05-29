/*
 * main.cpp
 *
 *  Created on: 29.05.2014
 *      Author: janis
 */

#include <string>
#include <map>
#include <functional>
#include <ostream>
#include <iostream>

// Commands Enum
#define DEFINE_PE_CMD(name) name,
enum pe_command{
#include "pe_commands.h"
};
#undef DEFINE_PE_CMD


#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

// Commands traits
template <pe_command cmd>
struct get_cmd_str {};

#define DEFINE_PE_CMD(name) \
    template <> \
    struct get_cmd_str<name> { \
	static const std::string & str(){ \
	    static std::string dummy(#name); \
	    return dummy; \
	} \
    };

#include "pe_commands.h"
#undef DEFINE_PE_CMD


template <pe_command CMD>
void handle_pe_command(const std::string & arg, std::ostream & out){
    out << "OK\n";
}

//struct PinentryCmdHandler{
//    virtual void handle(const std::string & arg, std::ostream & out) = 0;
//};
//
//template <pe_command CMD>
//struct TPinentryCmdHandler : public PinentryCmdHandler{
//    virtual void handle(const std::string & arg, std::ostream & out){
//	out << "OK\n";
//    }
//};
//
//template <>
//struct TPinentryCmdHandler<> : public Pinentry{
//    virtual void handle(const std::string & arg, std::ostream & out){
//	out << "OK\n";
//    }
//};

typedef std::map<const std::string, std::function<void(const std::string &, std::ostream &)> > handler_map_t;
static handler_map_t pe_handler_map;

// init handler map
#define DEFINE_PE_CMD(name) \
    pe_handler_map.insert(handler_map_t::value_type(get_cmd_str<name>::str(), handle_pe_command<name>));

void init_handlers(){
#include "pe_commands.h"
}
#undef DEFINE_PE_CMD

void main_loop(){
    std::string line;
    while(!std::cin.eof()){
	std::getline(std::cin, line);
	if(!line.size()) continue;
	auto pos = line.find_first_of(' ');
	auto cmd = line.substr(0, pos);
	auto arg = line.substr(pos + 1);
	auto handler = pe_handler_map.find(cmd);
	if (handler == pe_handler_map.end()){
	    std::cout << "ERR 83886355 unknown command" << std::endl;
	} else {
	    handler->second(arg, std::cout);
	}
    }
}

int main(int argc, char *args[]){
    init_handlers();
    main_loop();
    return 0;
}


