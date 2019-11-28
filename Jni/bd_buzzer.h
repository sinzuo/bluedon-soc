/**************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2017,      BLUEDON
 * All rights reserved.
 *
 * @file    bd_buzzer.h
 * @brief   ���Ʒ�������������
 *
 *
 * @version 1.0
 * @author  wb
 * @date    2017��8��2��
 *
 * �޶�˵��������汾
 **************************************************/

#ifndef BD_BUZZER_H_
#define BD_BUZZER_H_

//#include <string>


#ifdef __cplusplus
extern "C"
{
#endif

/*
*  ���Ʒ������ķ���(һ��ʱ�����Զ�ֹͣ)
*  @return             0: �ɹ� -1:ʧ��
*/
int bd_buzzer_start();


/*
*  ֹͣ�������ķ���(������ֹʱ,�������ͻ�һֱ��ͣ�Ľ�)
*/
void bd_buzzer_stop();


void play(unsigned int* freq, unsigned int* delay);
int  speaker(unsigned int freq, unsigned int delay);


#ifdef __cplusplus
}
#endif

#endif // BD_BUZZER_H_