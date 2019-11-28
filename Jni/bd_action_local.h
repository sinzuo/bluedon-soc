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

#ifndef BD_EXECUTION_LOCAL_H
#define BD_EXECUTION_LOCAL_H

#ifdef WIN32
#pragma warning(disable:4996)
#ifdef BD_ACTION_EXPORTS
#define BD_ACTIONLOCAL_EXPORTS __declspec(dllexport)
#else
#define BD_ACTIONLOCAL_EXPORTS __declspec(dllimport)
#endif // BD_ACTION_EXPORTS
#else
#define BD_ACTIONLOCAL_EXPORTS
#endif // WIN32

#include <string>

class BD_ACTIONLOCAL_EXPORTS bd_action_local {
public:
    bd_action_local();
    bd_action_local(const std::string& command, const std::string& argument);
    ~bd_action_local();

    /*
     * 执行用户上传命令/SOC扩充程序
     */
    int execute();

private:
    std::string m_command;
    std::string m_argument;
};

#endif