/*
 * pe_commands.h
 *
 *  Created on: 29.05.2014
 *      Author: janis
 */

// no include guard because this header is included multiple times intentionally!
#ifndef DEFINE_PE_CMD
#error pe_commands.h included but makro DEFINE_PE_CMD undefined
#define DEFINE_PE_CMD(name)
#endif

DEFINE_PE_CMD(SETDESC)
DEFINE_PE_CMD(SETPROMPT)
DEFINE_PE_CMD(SETTITLE)
DEFINE_PE_CMD(SETOK)
DEFINE_PE_CMD(SETCANCEL)
DEFINE_PE_CMD(SETERROR)
DEFINE_PE_CMD(SETQUALITYBAR)
DEFINE_PE_CMD(SETQUALITYBAR_TT)
DEFINE_PE_CMD(GETPIN)
DEFINE_PE_CMD(CONFIRM)
DEFINE_PE_CMD(MESSAGE)
DEFINE_PE_CMD(OPTION)


