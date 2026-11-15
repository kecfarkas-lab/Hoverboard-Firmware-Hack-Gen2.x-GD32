#ifndef CONFIG_H
#define CONFIG_H

#ifdef GD32F130
  #define LAYOUT 8
  #define LAYOUT_SUB 0
#endif

#define BLDC_SINE
#define DRIVING_MODE 0

#define BAT_CELLS 10
#define BATTERY_LOW_BEEP
#define BEEP_BACKWARDS

#define MASTER
//#define SLAVE
//#define SINGLE

#if defined(MASTER) || defined(SINGLE)
  #define REMOTE_ADC
  #define SPEED_COEFFICIENT 1
  #define STEER_COEFFICIENT 0
  #define MASTER_OR_SINGLE
#endif

#if defined(MASTER) || defined(SLAVE)
  #define MASTERSLAVE_USART 0
#endif

#ifdef MASTER_OR_SINGLE
  #define INACTIVITY_TIMEOUT 8
  #define CELL_LOW_LVL1 3.5
  #define CELL_LOW_LVL2 3.3
  #define CELL_LOW_DEAD 3.0
#endif

#define DC_CUR_LIMIT 15

#endif // CONFIG_H
