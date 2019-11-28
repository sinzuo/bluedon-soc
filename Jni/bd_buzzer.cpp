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

#include   <sys/io.h>
#include   <unistd.h>

#include   "bd_buzzer.h"


int bd_buzzer_start() {
    int   i;
    unsigned int freq[] = { 330, 392, 330, 294, 330, 392,
                            330, 394, 330, 330, 392, 330,
                            294, 262, 294, 330, 392, 294,
                            262, 262, 220, 196, 196, 220,
                            262, 294, 330, 262, 0
                          };

    unsigned int time[] = { 50, 50, 50 , 25, 25, 50,
                            25, 25, 100, 50, 50, 25,
                            25, 50, 50 , 25, 25, 100,
                            50, 25, 25 , 50, 25, 25,
                            50, 25, 25 , 100
                          };

    unsigned int freq2[] = {
        196, 262, 262, 262, 330, 294, 262, 294, 330, 294, 262,
        330, 394, 440, 440, 394, 330, 330, 262, 294, 262, 294,
        330, 294, 262, 230, 230, 196, 262, 440, 394, 330, 330,
        262, 294, 262, 294, 440, 394, 330, 330, 394, 440, 523,
        394, 330, 330, 262, 294, 262, 294, 330, 294, 262, 230,
        230, 196, 262, 440, 394, 330, 330, 262, 294, 262, 294,
        440, 394, 330, 330, 394, 440, 523, 394, 330, 330, 262,
        294, 262, 294, 330, 294, 262, 230, 230, 196, 262, 0
    };

    unsigned int time2[] = {
        25, 38, 12, 25, 25, 38, 12, 25, 12, 12, 56, 25, 25, 50, 25,
        38, 12, 12, 12, 38, 12, 25, 12, 12, 38, 12, 25, 25, 100, 25,
        38, 12, 12, 12, 38, 12, 25, 25, 38, 12, 25, 25, 100, 25, 38,
        12, 12, 12, 38, 12, 25, 12, 12, 38, 12, 25, 25, 100, 25, 38,
        12, 12, 12, 38, 12, 25, 25, 38, 12, 25, 25, 100, 25, 38, 12,
        12, 12, 38, 12, 25, 12, 12, 38, 12, 25, 25, 100
    };

    unsigned int freq_alert[] = {
        2000, 2400, 0
    };

    unsigned int time_alert[] = {
        50, 60
    };

    //循环5次播放警告音
    for (i = 0; i < 5; i++) {
        play(freq_alert, time_alert);
    }

    //播放歌曲1
    play(freq, time);
    //播放歌曲2
    play(freq2, time2);

    //下面这句用来关闭蜂鸣器,不然老叫，吵死了
    speaker(0, 0);

    return 0;
}


//如果蜂鸣器在鸣叫时程序被ctrl+c或者其他情况意外终止,蜂鸣器就会一直不停的叫
//下面这个函数让蜂鸣器不发声
void bd_buzzer_stop() {
    static int flag = 0;

    if (flag == 0) {
        flag = 1;
        iopl(3);
    }

    outb(0xfc, 0x61);
    return;
}


void play(unsigned int* freq, unsigned int* time) {
    int   i;

    for (i = 0; freq[i] != 0; i++) {
        speaker(freq[i], time[i]);
    }
}


int speaker(unsigned int freq, unsigned int delay) {
    static int flag = 0, bit;

    if (flag == 0) {
        flag = 1;
        iopl(3);
    }

    outb(0xb6, 0x43);
    outb((freq & 0xff), 0x42);
    outb((freq >> 8), 0x42);
    bit = inb(0x61);
    outb(3 | bit, 0x61);
    usleep(10000 * delay);
    outb(0xfc | bit, 0x61);

    return 0;
}

