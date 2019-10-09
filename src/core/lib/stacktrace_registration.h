//
// Created by Jan on 17.08.2019.
//

#ifndef RASPITOLIGHT_SRC_CORE_LIB_STACKTRACE_REGISTRATION_H_
#define RASPITOLIGHT_SRC_CORE_LIB_STACKTRACE_REGISTRATION_H_

#include <signal.h>
#include <iostream>
#include <spdlog/fmt/bundled/format.h>

/*
 * NOTE:
 * stacktrace_registration.h has to be included somewhere in the application and register_signal_handlers() has to be called once.
 *
 * Internally uses boost::stacktrace and this only works in UNIX Environments at the moment
 *
 * Example:
 * See crash_demo.cpp for example
 */

void print_stack_trace();

void register_signal_handlers();
std::string status_string(bool status);
std::string signal_handler_registration_report();

#endif //RASPITOLIGHT_SRC_CORE_LIB_STACKTRACE_REGISTRATION_H_
