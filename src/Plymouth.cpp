/*
 * Plymouth.cpp
 *
 *  Created on: 29.05.2014
 *      Author: janis
 */

#include "Plymouth.h"
#include <stdexcept>
#include "globals.h"
extern "C" {
#include <ply-boot-client.h>
#include <ply-event-loop.h>
}

namespace jd {

Plymouth::Plymouth() {
    _ply_client = ply_boot_client_new();
    if(_ply_client == 0){
	throw std::runtime_error("failed to create ply_boot_client");
    }
    auto disconnect_handler = [] (void* data, ply_boot_client_t *) -> void {
	reinterpret_cast<Plymouth*>(data)->handleDisconnect();
    };
    if (!ply_boot_client_connect(_ply_client, disconnect_handler, this)){
	ply_boot_client_free(_ply_client);
	throw std::runtime_error("ply_boot_client_connect failed");
    }
    _ply_event_loop = ply_event_loop_new();
    if(_ply_event_loop == 0){
	ply_boot_client_free(_ply_client);
	throw std::runtime_error("failed to create ply_event_loop");
    }
}

Plymouth::~Plymouth() {
    ply_boot_client_free(_ply_client);
    ply_event_loop_free(_ply_event_loop);
}

#define KEY_CTRL_C ('\100' ^ 'C')
#define KEY_ENTER  ((char)0x0d)

Plymouth::answer_t Plymouth::askPassword(const std::string & prompt, std::string & answer){
    ply_boot_client_attach_to_event_loop(_ply_client, _ply_event_loop);

    auto answer_helper = [this, &answer] (const char * _answer, int exit_status) -> void {
	if(_answer == 0 || _answer[0] == KEY_CTRL_C){
	    exit_status = CANCELED;
	}
	if(exit_status == OK){
	    answer.assign(_answer);
	}
	ply_event_loop_exit(_ply_event_loop, exit_status);
    };
    typedef decltype(answer_helper) answer_helper_t;
    auto answer_hdl = [] (void * data, const char * answer, ply_boot_client_t *) -> void{
	(*reinterpret_cast<answer_helper_t*>(data))(answer, OK);
    };
    auto fail_hdl = [] (void *data, ply_boot_client_t*) -> void{
	(*reinterpret_cast<answer_helper_t*>(data))("", CANCELED);
    };
    ply_boot_client_ask_daemon_for_password(_ply_client, prompt.c_str(), answer_hdl, fail_hdl, &answer_helper);

    return static_cast<answer_t>(ply_event_loop_run(_ply_event_loop));
}

Plymouth::answer_t Plymouth::askQuestion(const std::string & prompt, std::string & answer){
    ply_boot_client_attach_to_event_loop(_ply_client, _ply_event_loop);

    auto answer_helper = [this, &answer] (const char * _answer, int exit_status) -> void {
	if(_answer == 0 || _answer[0] == KEY_CTRL_C){
	    exit_status = CANCELED;
	}
	if(exit_status == OK){
	    answer.assign(_answer);
	}
	ply_event_loop_exit(_ply_event_loop, exit_status);
    };
    typedef decltype(answer_helper) answer_helper_t;
    auto answer_hdl = [] (void * data, const char * answer, ply_boot_client_t *) -> void{
	(*reinterpret_cast<answer_helper_t*>(data))(answer, OK);
    };
    auto fail_hdl = [] (void *data, ply_boot_client_t*) -> void{
	(*reinterpret_cast<answer_helper_t*>(data))("", FAILED);
    };
    ply_boot_client_ask_daemon_question(_ply_client, prompt.c_str(), answer_hdl, fail_hdl, &answer_helper);

    return static_cast<answer_t>(ply_event_loop_run(_ply_event_loop));
}

Plymouth::answer_t Plymouth::displayMessage(const std::string & message){
    ply_boot_client_attach_to_event_loop(_ply_client, _ply_event_loop);

    auto response_helper = [this] (int exit_status) -> void {
	ply_event_loop_exit(_ply_event_loop, exit_status);
    };
    typedef decltype(response_helper) answer_helper_t;
    auto response_hdl = [] (void * data, ply_boot_client_t *) -> void{
	(*reinterpret_cast<answer_helper_t*>(data))(OK);
    };
    auto fail_hdl = [] (void *data, ply_boot_client_t*) -> void{
	(*reinterpret_cast<answer_helper_t*>(data))(FAILED);
    };
    ply_boot_client_tell_daemon_to_display_message(_ply_client, message.c_str(), response_hdl, fail_hdl, &response_helper);

    return static_cast<answer_t>(ply_event_loop_run(_ply_event_loop));
}

Plymouth::answer_t Plymouth::whatchKey(const std::string & keys, std::string & result){
    ply_boot_client_attach_to_event_loop(_ply_client, _ply_event_loop);

    auto answer_helper = [this, &result] (const char * _answer, int exit_status) -> void {
	if(_answer == 0){
	    exit_status = CANCELED;
	}
	if(exit_status == OK){
	    result.assign(_answer);
	}
	ply_event_loop_exit(_ply_event_loop, exit_status);
    };
    typedef decltype(answer_helper) answer_helper_t;
    auto answer_hdl = [] (void * data, const char * answer, ply_boot_client_t *) -> void{
	(*reinterpret_cast<answer_helper_t*>(data))(answer, OK);
    };
    auto fail_hdl = [] (void *data, ply_boot_client_t*) -> void{
	(*reinterpret_cast<answer_helper_t*>(data))("", FAILED);
    };

    ply_boot_client_ask_daemon_to_watch_for_keystroke(_ply_client, keys == "" ? 0 : keys.c_str(), answer_hdl, fail_hdl, &answer_helper);

    return static_cast<answer_t>(ply_event_loop_run(_ply_event_loop));
}
Plymouth::answer_t Plymouth::pause(bool do_pause){
    ply_boot_client_attach_to_event_loop(_ply_client, _ply_event_loop);

    auto response_helper = [this] (int exit_status) -> void {
	ply_event_loop_exit(_ply_event_loop, exit_status);
    };
    typedef decltype(response_helper) answer_helper_t;
    auto success_hdl = [] (void * data, ply_boot_client_t *) -> void{
	(*reinterpret_cast<answer_helper_t*>(data))(OK);
    };
    auto fail_hdl = [] (void *data, ply_boot_client_t*) -> void{
	(*reinterpret_cast<answer_helper_t*>(data))(FAILED);
    };

    if (do_pause){
	ply_boot_client_tell_daemon_to_progress_pause(_ply_client, success_hdl, fail_hdl, &response_helper);
    } else {
	ply_boot_client_tell_daemon_to_progress_unpause(_ply_client, success_hdl, fail_hdl, &response_helper);
    }

    return static_cast<answer_t>(ply_event_loop_run(_ply_event_loop));
}
Plymouth::answer_t Plymouth::unpause(){
    return pause(false);
}


void Plymouth::handleDisconnect(){
    log << "ply disconnected" << std::endl;
}

} /* namespace jd */
