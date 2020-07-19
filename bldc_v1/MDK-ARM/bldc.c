#include "bldc.h"
#include "tim.h"

#define MOTOR_DIR_CW               1    // 电机方向: 顺时针
#define MOTOR_DIR_CCW              (-1) // 电机方向: 逆时针



__IO uint32_t Lock_Time = 0;  // 堵转时间

__IO int32_t uwStep = 0;      // 6步引脚状态
__IO int32_t Dir = MOTOR_DIR_CCW;       // 旋转方向
__IO float PWM_Duty = 0.15f;  // 占空比15%

void TheFirstSequence(void);
void CCW_Sequence(int32_t  HALL_Step);
void CW_Sequence(int32_t  HALL_Step);


/**
  * 函数功能: 使能无刷电机转动
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
void Enable_BLDC(void)
{
	 /* 使能霍尔传感器接口 和 PWM输出功能 */
  HAL_TIMEx_HallSensor_Start(&htim3); 
  TheFirstSequence();
  HAL_TIM_GenerateEvent(&htim1,TIM_EVENTSOURCE_COM);
  __HAL_TIM_CLEAR_FLAG(&htim1,TIM_FLAG_COM);
  HAL_TIMEx_CommutationCallback(&htim1);

}

/**
  * 函数功能: 停止电机转动
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
void Disable_BLDC(void)
{
  HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
  HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
  
  HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
  HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
  
  HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
  HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);
  
  HAL_TIMEx_HallSensor_Stop(&htim3); 
}


uint32_t HallSensor_GetPinState(void)
{
  __IO static uint32_t State ;
  State  = 0;
  /*霍尔传感器状态获取*/
  if(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_6) != GPIO_PIN_RESET)  
  {
    State |= 0x01U;
  }
  if(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_7) != GPIO_PIN_RESET) 
  {
    State |= 0x02U;
  }
  if(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_8) != GPIO_PIN_RESET) 
  {
    State |= 0x04U;
  }
  return State;
}

void TheFirstSequence()
{
	 /* 获取霍尔传感器引脚状态,作为换相的依据 */
  __IO uint32_t tmp = 0;
  uwStep = HallSensor_GetPinState();
  if(Dir == MOTOR_DIR_CW)
  {
    uwStep = (uint32_t)7 - uwStep; // 根据顺序表的规律 CW = 7 - CCW;
  }
  /*---- 定义定时器OC1为A(U)相 OC2为B(V)相，OC3为C(W)相 ---- */
  /*---- 定义uWStep低3位为霍尔传感器引脚状态,IC1(001,U),IC2(010,V),IC3(100,W) ----*/
  switch(uwStep)
  {
    case 1://C+ A-
      /*  Channel2 configuration  */ 
      HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
      HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
    
      /*  Channel3 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_3,BLDCMOTOR_TIM_PERIOD * PWM_Duty);
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3); 

      /*  Channe1l configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,BLDCMOTOR_TIM_PERIOD +1);
      HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
      break;
    
    case 2: //A+  B-
      /*  Channel3 configuration  */ 
      HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
      HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);
    
      /*  Channel1 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,BLDCMOTOR_TIM_PERIOD * PWM_Duty);
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
      
      /*  Channel2 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,BLDCMOTOR_TIM_PERIOD +1);
      HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
      break;
    
    case 3:// C+ B-
      /*  Channel1 configuration  */ 
      HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
      HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
 
      /*  Channel3 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_3,BLDCMOTOR_TIM_PERIOD * PWM_Duty);
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
  
      /*  Channel2 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,BLDCMOTOR_TIM_PERIOD +1);
      HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
      break;
    
    case 4:// B+ C-
      /*  Channel1 configuration  */ 
      HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
      HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);

      /*  Channel2 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,BLDCMOTOR_TIM_PERIOD * PWM_Duty);
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
      
      /*  Channel3 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_3,BLDCMOTOR_TIM_PERIOD +1);    
      HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
      break;
    
    case 5: // B+ A-
      /*  Channel3 configuration  */       
      HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
      HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);
    
      /*  Channel2 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,BLDCMOTOR_TIM_PERIOD * PWM_Duty);
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
      
      /*  Channel1 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,BLDCMOTOR_TIM_PERIOD +1);
      HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
      break;
    
    case 6: // A+ C-
      /*  Channel2 configuration  */ 
      HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
      HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
    
      /*  Channel1 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,BLDCMOTOR_TIM_PERIOD * PWM_Duty);
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1); 
      
      /*  Channe3l configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_3,BLDCMOTOR_TIM_PERIOD +1);  
      HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
      break;
  }
  Lock_Time = 0;


}


/* 顺时针顺序表*/
void CW_Sequence( int32_t  HALL_Step)
{
  /*---- 定义定时器OC1为A(U)相 OC2为B(V)相，OC3为C(W)相 ---- */
  /*---- 定义uWStep低3位为霍尔传感器引脚状态,IC1(001,U),IC2(010,V),IC3(100,W) ----*/
	switch(HALL_Step)
  {
		    case 1:// A+ B- 设置下一步的通电相序
      /*  Channel3 configuration  */ 
      HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
      HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);
 
      /*  Channel1 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,BLDCMOTOR_TIM_PERIOD * PWM_Duty);
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  
      /*  Channe2 configuration   */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,BLDCMOTOR_TIM_PERIOD +1);
      HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
      break;
    
    case 2: // 下一步是 B+ C-
      /*  Channel1 configuration  */ 
      HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
      HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
    
      /*  Channel2 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,BLDCMOTOR_TIM_PERIOD * PWM_Duty);
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2); 
      
      /*  Channel3 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_3,BLDCMOTOR_TIM_PERIOD +1);  
      HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
      break;
    
    case 3: // 下一步是  A+  C-
      /*  Channel2 configuration  */ 
      HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
      HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
    
      /*  Channel1 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,BLDCMOTOR_TIM_PERIOD * PWM_Duty);
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
      
      /*  Channel3 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_3,BLDCMOTOR_TIM_PERIOD +1);
      HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
      break;
    
    case 4: // 下一步是 C+ A-
      /*  Channel2 configuration  */       
      HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
      HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
    
      /*  Channel3 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_3,BLDCMOTOR_TIM_PERIOD * PWM_Duty);
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
      
      /*  Channel1 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,BLDCMOTOR_TIM_PERIOD +1);
      HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
      break;
    
    case 5:// 下一步是 C+ B-
      /*  Channel1 configuration  */ 
      HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
      HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
    
      /*  Channel3 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_3,BLDCMOTOR_TIM_PERIOD * PWM_Duty);
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3); 

      /*  Channel2 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,BLDCMOTOR_TIM_PERIOD +1);
      HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
      break;
    
    case 6:// 下一步是 B+ A-
      /*  Channel3 configuration  */ 
      HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
      HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);

      /*  Channel2 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,BLDCMOTOR_TIM_PERIOD * PWM_Duty);
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
      
      /*  Channel1 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,BLDCMOTOR_TIM_PERIOD +1);    
      HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
      break;
	
	
	
	}


}





/* 逆时针顺序表*/
void CCW_Sequence( int32_t  HALL_Step)
{
  /*---- 定义定时器OC1为A(U)相 OC2为B(V)相，OC3为C(W)相 ---- */
  /*---- 定义uWStep低3位为霍尔传感器引脚状态,IC1(001,U),IC2(010,V),IC3(100,W) ----*/
    switch(HALL_Step)
  {
    case 1:// C+ B- 设置下一步的通电相序
      /*  Channel1 configuration  */ 
      HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
      HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
 
      /*  Channel3 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_3,BLDCMOTOR_TIM_PERIOD * PWM_Duty);
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
  
      /*  Channe2 configuration   */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,BLDCMOTOR_TIM_PERIOD +1);
      HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
      break;
    
    case 2: // 下一步是 A+ C-
      /*  Channel2 configuration  */ 
      HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
      HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
    
      /*  Channel1 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,BLDCMOTOR_TIM_PERIOD * PWM_Duty);
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1); 
      
      /*  Channel3 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_3,BLDCMOTOR_TIM_PERIOD +1);  
      HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
      break;
    
    case 3: // 下一步是  A+  B-
      /*  Channel3 configuration  */ 
      HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
      HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);
    
      /*  Channel1 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,BLDCMOTOR_TIM_PERIOD * PWM_Duty);
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
      
      /*  Channel2 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,BLDCMOTOR_TIM_PERIOD +1);
      HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
      break;
    
    case 4: // 下一步是 B+ A-
      /*  Channel3 configuration  */       
      HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
      HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);
    
      /*  Channel2 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,BLDCMOTOR_TIM_PERIOD * PWM_Duty);
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
      
      /*  Channel1 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,BLDCMOTOR_TIM_PERIOD +1);
      HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
      break;
    
    case 5:// 下一步是 C+ A-
      /*  Channel2 configuration  */ 
      HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
      HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
    
      /*  Channel3 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_3,BLDCMOTOR_TIM_PERIOD * PWM_Duty);
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3); 

      /*  Channel1 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,BLDCMOTOR_TIM_PERIOD +1);
      HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
      break;
    
    case 6:// 下一步是 B+ C-
      /*  Channel1 configuration  */ 
      HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
      HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);

      /*  Channel2 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,BLDCMOTOR_TIM_PERIOD * PWM_Duty);
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
      
      /*  Channel3 configuration  */
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_3,BLDCMOTOR_TIM_PERIOD +1);    
      HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
      break;
  } 
}


void HAL_TIMEx_CommutationCallback(TIM_HandleTypeDef *htim)
{
  /* 获取霍尔传感器引脚状态,作为换相的依据 */
  __IO uint32_t tmp = 0;
  uwStep = HallSensor_GetPinState();
  if(Dir == MOTOR_DIR_CW)
  {
    CW_Sequence( uwStep );
  }
  else
  {
    CCW_Sequence( uwStep );
  }
  Lock_Time = 0;
}
/******************* (C) COPYRIGHT 2020 COETIC *****END OF FILE****/
