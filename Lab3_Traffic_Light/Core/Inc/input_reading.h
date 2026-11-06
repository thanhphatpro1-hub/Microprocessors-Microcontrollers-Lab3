/*
 * input_reading.h
 *
 *  Created on: Lab 3 - CO3009 (HCMUT)
 *  Description: Header for Button Reading and 3-Stage Debouncing Driver
 */

#ifndef INC_INPUT_READING_H_
#define INC_INPUT_READING_H_

#include "main.h"
#include "timer.h"
#include <stdint.h>

/* Số lượng nút nhấn trong hệ thống (Mode, Inc, Set) */
#define NO_OF_BUTTONS                       3U

/* Chỉ số định danh cho từng nút nhấn */
#define BUTTON_MODE                         0U  /* Nút 1 (PB0): Chuyển đổi 4 Mode */
#define BUTTON_INC                          1U  /* Nút 2 (PB1): Tăng giá trị (1..99) */
#define BUTTON_SET                          2U  /* Nút 3 (PB2): Lưu giá trị mới */

/* Mức logic phần cứng của nút nhấn Active-Low (Có trở kéo lên 10k) */
#define BUTTON_IS_PRESSED                   GPIO_PIN_RESET
#define BUTTON_IS_RELEASED                  GPIO_PIN_SET

/* Thời gian nhấn giữ 1 giây (1000ms) được tính tự động theo TICK_MS (Exercise 4) */
#define DURATION_FOR_AUTO_INCREASING_MS     1000U
#define DURATION_FOR_AUTO_INCREASING        ((DURATION_FOR_AUTO_INCREASING_MS >= TICK_MS) ? \
                                             (DURATION_FOR_AUTO_INCREASING_MS / TICK_MS) : 1U)

/* Nguyên mẫu hàm chuẩn theo Mục 4.3.2 của tài liệu Lab 3 */
void button_reading(void);
unsigned char is_button_pressed(unsigned char index);
unsigned char is_button_pressed_1s(unsigned char index);

#endif /* INC_INPUT_READING_H_ */
