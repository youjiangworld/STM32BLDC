#ifndef __EXIT_H
#define __EXIT_H

#include "stm32h7xx.h" //STM32H7所有寄存器的映射在这个文件中

// 定义定时器周期，当定时器开始计数到BLDCMOTOR_TIMx_PERIOD值并且重复计数寄存器为0时更新定时器并生成对应事件和中断
#define BLDCMOTOR_TIM_PERIOD     1000   





#endif /*__EXIT_H*/
