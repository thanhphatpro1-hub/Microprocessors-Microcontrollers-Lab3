/*
 * led_display.h
 *
 *  Created on: Lab 3 - CO3009 (HCMUT)
 *  Description: Header for 12 Traffic LEDs & 4 Multiplexed 7-Segment LEDs Driver
 */

#ifndef INC_LED_DISPLAY_H_
#define INC_LED_DISPLAY_H_

#include "main.h"
#include <stdint.h>

/* =========================================================================
 * CẤU HÌNH MỨC LOGIC PHẦN CỨNG TRÊN PROTEUS
 * 1. Đèn giao thông đơn (Active-Low: RESET = Sáng, SET = Tắt)
 * 2. Các đoạn a..g của LED 7 đoạn Anode chung (RESET = Sáng, SET = Tắt)
 * 3. Chân Enable EN0..EN3:
 *    - Nếu qua cổng NOT / Transistor PNP: EN_ON = GPIO_PIN_RESET, EN_OFF = GPIO_PIN_SET
 *    - Nếu nối thẳng vào chân 1..4 của 7SEG-MPX4-CA: Đổi EN_ON = GPIO_PIN_SET, EN_OFF = GPIO_PIN_RESET
 * ========================================================================= */
#define LED_ON                      GPIO_PIN_RESET
#define LED_OFF                     GPIO_PIN_SET

#define SEG_ON                      GPIO_PIN_RESET
#define SEG_OFF                     GPIO_PIN_SET

#define EN_ON                       GPIO_PIN_RESET
#define EN_OFF                      GPIO_PIN_SET

/* Số lượng bóng LED 7 đoạn trong hệ thống */
#define NO_OF_7SEG                  4U

/* Kiểu liệt kê màu đèn giao thông */
typedef enum {
    COLOR_OFF = 0,
    COLOR_RED,
    COLOR_AMBER,
    COLOR_GREEN
} TrafficColor_t;

/* Nguyên mẫu các hàm điều khiển 12 LED giao thông (6 kênh GPIO) */
void clear_all_traffic_leds(void);
void set_traffic_leds(TrafficColor_t road1_color, TrafficColor_t road2_color);
void toggle_traffic_color_2hz(TrafficColor_t color);

/* Nguyên mẫu các hàm điều khiển và quét đa hợp 4 LED 7 đoạn */
void display7SEG(uint8_t num);
void update7SEG(uint8_t index);
void updateClockBuffer(uint8_t road1_or_mode, uint8_t road2_or_value);
void led_7seg_scanning_run(void);

#endif /* INC_LED_DISPLAY_H_ */
