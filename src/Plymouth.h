/*
 * Plymouth.h
 *
 *  Created on: 29.05.2014
 *      Author: janis
 */

#ifndef PLYMOUTH_H_
#define PLYMOUTH_H_

#include <string>
#include <functional>

extern "C" {
#include <ply-boot-client.h>
#include <ply-event-loop.h>
}

namespace jd {

class Plymouth {
    ply_boot_client_t * _ply_client;
    ply_event_loop_t * _ply_event_loop;
public:
    enum answer_t {
	OK = 0,
	CANCELED = 1,
	FAILED = 2,
    };
    Plymouth();
    virtual ~Plymouth();

    answer_t askPassword(const std::string & promt, std::string & answer);
    answer_t askQuestion(const std::string & question, std::string & answer);
    answer_t displayMessage(const std::string & message);
    answer_t whatchKey(const std::string & keys, std::string & result);
    answer_t pause(bool do_pause = true);
    answer_t unpause();
private:
    void handleDisconnect();
};

} /* namespace jd */

#endif /* PLYMOUTH_H_ */
