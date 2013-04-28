/*
 * Copyright 2012 Pekka Nikander.  See NOTICE for licensing information.
 */

/*
 * Missing STM32F0 Timer definitions
 */

#define TIM_CCMR1_OC1M_PWM1     ((  TIM_CCMR1_OC1M_2 )|(  TIM_CCMR1_OC1M_1 )|( !TIM_CCMR1_OC1M_0 ))
#define TIM_CCMR1_OC1M_PWM2     ((  TIM_CCMR1_OC1M_2 )|(  TIM_CCMR1_OC1M_1 )|(  TIM_CCMR1_OC1M_0 ))

#define TIM_CCMR1_OC2M_PWM1     ((  TIM_CCMR1_OC2M_2 )|(  TIM_CCMR1_OC2M_1 )|( !TIM_CCMR1_OC2M_0 ))
#define TIM_CCMR1_OC2M_PWM2     ((  TIM_CCMR1_OC2M_2 )|(  TIM_CCMR1_OC2M_1 )|(  TIM_CCMR1_OC2M_0 ))

#define TIM_CCMR2_OC3M_Timing   (( !TIM_CCMR2_OC3M_2 )|( !TIM_CCMR2_OC3M_1 )|( !TIM_CCMR2_OC3M_0 ))
#define TIM_CCMR2_OC3M_Active   (( !TIM_CCMR2_OC3M_2 )|( !TIM_CCMR2_OC3M_1 )|(  TIM_CCMR2_OC3M_0 ))
#define TIM_CCMR2_OC3M_Inactive (( !TIM_CCMR2_OC3M_2 )|(  TIM_CCMR2_OC3M_1 )|( !TIM_CCMR2_OC3M_0 ))
#define TIM_CCMR2_OC3M_Toggle   (( !TIM_CCMR2_OC3M_2 )|(  TIM_CCMR2_OC3M_1 )|(  TIM_CCMR2_OC3M_0 ))

#define TIM_CCMR2_OC3M_PWM1     ((  TIM_CCMR2_OC3M_2 )|(  TIM_CCMR2_OC3M_1 )|( !TIM_CCMR2_OC3M_0 ))
#define TIM_CCMR2_OC3M_PWM2     ((  TIM_CCMR2_OC3M_2 )|(  TIM_CCMR2_OC3M_1 )|(  TIM_CCMR2_OC3M_0 ))

#define TIM_CCMR2_OC4M_Timing   (( !TIM_CCMR2_OC4M_2 )|( !TIM_CCMR2_OC4M_1 )|( !TIM_CCMR2_OC4M_0 ))
#define TIM_CCMR2_OC4M_Active   (( !TIM_CCMR2_OC4M_2 )|( !TIM_CCMR2_OC4M_1 )|(  TIM_CCMR2_OC4M_0 ))
#define TIM_CCMR2_OC4M_Inactive (( !TIM_CCMR2_OC4M_2 )|(  TIM_CCMR2_OC4M_1 )|( !TIM_CCMR2_OC4M_0 ))
#define TIM_CCMR2_OC4M_Toggle   (( !TIM_CCMR2_OC4M_2 )|(  TIM_CCMR2_OC4M_1 )|(  TIM_CCMR2_OC4M_0 ))

#define TIM_CCMR2_OC4M_PWM1     ((  TIM_CCMR2_OC4M_2 )|(  TIM_CCMR2_OC4M_1 )|( !TIM_CCMR2_OC4M_0 ))
#define TIM_CCMR2_OC4M_PWM2     ((  TIM_CCMR2_OC4M_2 )|(  TIM_CCMR2_OC4M_1 )|(  TIM_CCMR2_OC4M_0 ))

#define TIM_CCER_CC1P_Rising    (( !TIM_CCER_CC1NP )|( !TIM_CCER_CC1P ))
#define TIM_CCER_CC1P_Falling   (( !TIM_CCER_CC1NP )|(  TIM_CCER_CC1P ))
#define TIM_CCER_CC1P_Both      ((  TIM_CCER_CC1NP )|(  TIM_CCER_CC1P ))
#define TIM_CCER_CC2P_Rising    (( !TIM_CCER_CC2NP )|( !TIM_CCER_CC2P ))
#define TIM_CCER_CC2P_Falling   (( !TIM_CCER_CC2NP )|(  TIM_CCER_CC2P ))
#define TIM_CCER_CC2P_Both      ((  TIM_CCER_CC2NP )|(  TIM_CCER_CC2P ))
#define TIM_CCER_CC3P_Rising    (( !TIM_CCER_CC3NP )|( !TIM_CCER_CC3P ))
#define TIM_CCER_CC3P_Falling   (( !TIM_CCER_CC3NP )|(  TIM_CCER_CC3P ))
#define TIM_CCER_CC3P_Both      ((  TIM_CCER_CC3NP )|(  TIM_CCER_CC3P ))
#define TIM_CCER_CC4P_Rising    (( !TIM_CCER_CC4NP )|( !TIM_CCER_CC4P ))
#define TIM_CCER_CC4P_Falling   (( !TIM_CCER_CC4NP )|(  TIM_CCER_CC4P ))
#define TIM_CCER_CC4P_Both      ((  TIM_CCER_CC4NP )|(  TIM_CCER_CC4P ))
