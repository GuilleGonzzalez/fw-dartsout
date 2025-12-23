/**
 * @file dartsout_boardv2.h
 *
 * @brief Board definition for Dartsout board V2
 *
 * @version v1.0.0
 */

#ifndef __DARTSOUT_BOARD2_H
#define __DARTSOUT_BOARD2_H

#ifdef __cplusplus
extern "C" {
#endif

#define BOARD_ID 1

#define PIN_LED1 19 // LED 1
#define PIN_LED2 20 // LED 2

#define BTN_1    10 // POWER
#define BTN_2    9  // START
#define BTN_3    3  // DOUBLE
#define BTN_4    5  // CYBERMATCH
#define BTN_5    6  // PLAYER
#define BTN_6    7  // GAME

#define LED_CONN PIN_LED1
#define LED_SEND PIN_LED2

#define BTN_POWER  BTN_1
#define BTN_PLAYER BTN_5
#define BTN_BACK   BTN_4


#ifdef __cplusplus
}
#endif

#endif // __DARTSOUT_BOARD2_H
