/**************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2011,      BLUEDON
 * All rights reserved.
 *
 * @file    xxx.h
 * @brief   xxx
 *
 *
 * @version 1.0
 * @author  xxx
 * @date    2011年09月13日
 *
 * 修订说明：最初版本
 **************************************************/

#include "bd_action_local.h"
#include <stdlib.h>
#include <string>
#include "ace/OS.h"
#include "utils/bd_common.h"
//#include "bd_log_macros.h"


bd_action_local::bd_action_local() {
}

bd_action_local::bd_action_local(const std::string& command, const std::string& argument)
    : m_command(command), m_argument(argument) {
}

bd_action_local::~bd_action_local() {
}

int bd_action_local::execute() {
    //BD_TRACE( bd_action_local, execute );

    std::string cmd;
    cmd.append(m_command);
    cmd.append(" ");
    cmd.append(m_argument);
    bd_trim(&cmd);
    return system(cmd.c_str());
}

