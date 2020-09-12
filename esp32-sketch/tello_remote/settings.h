#ifndef SETTINGS_H
#define SETTINGS_H

#define SW_ARM 32
#define SW_FLY 33
#define SW_FAST 17 

#define SW_PLANET_0 2
#define SW_PLANET_1 4
#define SW_PLANET_2 5

#define JOY_LEFT_RIGHT 34
#define JOY_FORW_BACK 36
#define JOY_UP_DOWN 39
#define JOY_YAW 35

#define LED_ARM 16 
#define LED_DISARM 12 
#define LED_FAST 18 
#define LED_SLOW 19 
#define LED_FLY 21
#define LED_LAND 22 
#define LED_PLANET_0 25 
#define LED_PLANET_1 26 
#define LED_PLANET_2 27 
#define LED_POWER 13

// joystick calibration values
#define MIN_LEFT_RIGHT 1636
#define MAX_LEFT_RIGHT 3153
#define MIN_FORW_BACK 2526 
#define MAX_FORW_BACK 3947
#define MIN_UP_DOWN 2457
#define MAX_UP_DOWN 3931
#define MIN_YAW 1575
#define MAX_YAW 2960

// until what value do we ignore the joystick
#define JOY_THRESHOLD 22

#endif
