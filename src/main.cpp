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
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "Plymouth.h"
#include "globals.h"

//static std::ofstream _log("/tmp/plymentry.log", std::ios_base::out | std::ios_base::app);

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

enum pe_errors {
    UNKNOWN_COMMAND = 0x5000113,
    CANCELED =        0x5000063,
    NOT_CONFIRMED =   0x5000072,
    SERVER_FAULT =    0x5000000 | 80,
};

template <pe_command CMD>
void handle_pe_command(const std::string & arg, std::ostream & out){
    log << "--> " << get_cmd_str<CMD>::str() << " " << arg << std::endl;
    out << "OK\n";
}

#define DEFINE_PE_CMD(name) \
case name: return get_cmd_str<name>::str();
const std::string & getCmdString(const pe_command cmd){
    static const std::string dummy("");
    switch(cmd){
	#include "pe_commands.h"
    default: return dummy;
    }
}
#undef DEFINE_PE_CMD

static std::string decode0x25(const std::string & str){
    std::stringstream s;
    for (auto c = str.begin(); c != str.end(); ++c ){
	if (*c == '%'){
	    std::stringstream parse;
	    unsigned x;
	    parse << *(++c); parse << *(++c);
	    parse >> std::hex >> x;
	    // drop if no ascii char
	    if(x & 0x80) continue;
	    s << (char)x;
	} else {
	    s << *c;
	}
    }
    return s.str();
}

class PEPipeServer{
public:
    typedef std::function<void(const std::string &, std::ostream &)> handler_t;
private:
    typedef std::map<const std::string, handler_t> handler_map_t;
    handler_map_t _handler_map;
public:
    #define DEFINE_PE_CMD(name) \
    _handler_map.insert(handler_map_t::value_type(get_cmd_str<name>::str(), handle_pe_command<name>));
    PEPipeServer(){
	#include "pe_commands.h"
    }
    #undef DEFINE_PE_CMD
    void registerHandler(const std::string & command, handler_t && handler){
	_handler_map[command] = handler;
    }
    void registerHandler(const pe_command cmd, handler_t && handler){
	registerHandler(getCmdString(cmd), std::move(handler));
    }
    void enterLoop (std::istream & in, std::ostream & out){
	out << "OK Your orders please\n";
	std::string line;
	std::string arg;
	while(!in.eof()){
	    std::getline(in, line);
	    if(!line.size()) continue;
	    auto pos = line.find_first_of(' ');
	    auto cmd = line.substr(0, pos);
	    if(pos != std::string::npos) {
		arg = line.substr(pos + 1);
	    } else {
		arg = "";
	    }
	    auto handler = _handler_map.find(cmd);
	    if (handler == _handler_map.end()){
		out << "ERR " << UNKNOWN_COMMAND << " unknown command" << std::endl;
	    } else {
		try{
		    handler->second(decode0x25(arg), std::cout);
		} catch (std::runtime_error & e){
		    log << e.what() << std::endl;
		    out << "ERR " << SERVER_FAULT << " server fault" << std::endl;
		}
	    }
	}
    }
};

inline static jd::Plymouth::answer_t plym_check(jd::Plymouth::answer_t err){
    if(err == jd::Plymouth::FAILED){
	throw std::runtime_error("fail");
    }
    return err;
}

class PlymEntry: public PEPipeServer{
    jd::Plymouth _ply;
    std::string _desc;
    std::string _prompt;
    std::string _title;
    std::string _ok;
    std::string _cancel;
public:
    PlymEntry()
    : _desc("")
    , _prompt("Passphrase:")
    , _title("")
    , _ok("OK")
    , _cancel("Cancel")
    {
	registerHandler(SETDESC,
		[this] (const std::string & arg, std::ostream & out) -> void{
		    _desc = arg;
		    out << "OK\n";
		});
	registerHandler(SETPROMPT,
		[this] (const std::string & arg, std::ostream & out) -> void{
		    _prompt = arg;
		    out << "OK\n";
		});
	registerHandler(SETTITLE,
		[this] (const std::string & arg, std::ostream & out) -> void{
		    _title = arg;
		    out << "OK\n";
		});
	registerHandler(SETOK,
		[this] (const std::string & arg, std::ostream & out) -> void{
		    _ok = arg;
		    out << "OK\n";
		});
	registerHandler(SETCANCEL,
		[this] (const std::string & arg, std::ostream & out) -> void{
		    _cancel = arg;
		    out << "OK\n";
		});
	registerHandler(GETPIN,
		[this] (const std::string & arg, std::ostream & out) -> void{
		    std::string password;
		    _ply.pause();
		    jd::Plymouth::answer_t result = plym_check(_ply.askPassword(_prompt, password));
		    _ply.unpause();
		    if(result == jd::Plymouth::OK){
			out << "D " << password << "\n";
			out << "OK\n";
			return;
		    }
		    out << "ERR " << CANCELED << " canceled\n";
		});
	auto message_hdl = [this] (const std::string & arg, std::ostream & out) -> void{
	    _ply.pause();
	    std::stringstream message;
	    message << _desc << "\n\nPress any key to continue";
	    jd::Plymouth::answer_t result = plym_check(_ply.displayMessage(message.str()));
	    std::string keys("nothing assigned");
	    result = _ply.whatchKey("", keys);
	    _ply.displayMessage("");
	    _ply.unpause();
	    out << "OK\n";
	};
	registerHandler(MESSAGE, message_hdl);
	registerHandler(CONFIRM,
		[this, message_hdl] (const std::string & arg, std::ostream & out) -> void{
		    if (arg.find("--one-button") != std::string::npos){
			message_hdl(arg, out);
		    } else {
			_ply.pause();
			std::stringstream message;
			message << _desc << "\n\nPress y to confirm or n to cancel";
			jd::Plymouth::answer_t result = plym_check(_ply.displayMessage(message.str()));
			std::string keys("nothing assigned");
			do{
			    result = plym_check(_ply.whatchKey("", keys));
			    if(keys == "n" || keys == "N"){
				out << "ERR " << CANCELED << " canceled\n";
				break;
			    } else if(keys == "y" || keys == "Y"){
				out << "OK\n";
				break;
			    }
			} while(!result);
			_ply.displayMessage("");
			_ply.unpause();
		    }
		});
    }
};

int main(int argc, char *args[]){
    log << "plymentry sais hi!" << std::endl;
    try{
	PlymEntry plyme;
	plyme.enterLoop(std::cin, std::cout);
    } catch (std::runtime_error & e){
	log << e.what() << std::endl;
	return 2;
    }
    return 0;
}


