/**************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2017,      BLUEDON
 * All rights reserved.
 *
 * @file    bd_buzzer.h
 * @brief   控制蜂鸣器发报警声
 *
 *
 * @version 1.0
 * @author  wb
 * @date    2017年8月2日
 *
 * 修订说明：最初版本
 **************************************************/

#ifndef BD_BUZZER_H_
#define BD_BUZZER_H_

//#include <string>


#ifdef __cplusplus
extern "C"
{
#endif

/*
*  控制蜂鸣器的发声(一段时间后会自动停止)
*  @return             0: 成功 -1:失败
*/
int bd_buzzer_start();


/*
*  停止蜂鸣器的发声(意外终止时,蜂鸣器就会一直不停的叫)
*/
void bd_buzzer_stop();


void play(unsigned int* freq, unsigned int* delay);
int  speaker(unsigned int freq, unsigned int delay);


#ifdef __cplusplus
}
#endif

#endif // BD_BUZZER_H_