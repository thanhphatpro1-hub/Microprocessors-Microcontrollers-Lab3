/*
 * timer.h
 *
 *  Created on: Lab 3 - CO3009 (HCMUT)
 *  Description: Header for Software Timer Engine (Exercise 4 compliant)
 */

#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#include "main.h"
#include <stdint.h>

/* =========================================================================
 * CẤU HÌNH BƯỚC NHẢY NGẮT PHẦN CỨNG TIM2 (EXERCISE 4)
 * - Mặc định: 10 (tương ứng ngắt TIM2 mỗi 10ms).
 * - Khi đổi cấu hình TIM2 sang 1ms hoặc 100ms, chỉ cần sửa TICK_MS thành
 *   1 hoặc 100 tại đúng dòng này. Toàn bộ hệ thống sẽ tự động thích nghi!
 * ========================================================================= */
#define TICK_MS                     10U

/* Hằng số thời gian chuẩn của các tác vụ trong hệ thống (Đơn vị: ms) */
#define TIME_1S_MS                  1000U   /* Chu kỳ 1 giây đếm ngược đèn giao thông */
#define TIME_BLINK_2HZ_MS           250U    /* Chớp 2Hz (Chu kỳ 500ms = 250ms Sáng + 250ms Tắt) */
#define TIME_7SEG_SCAN_MS           50U     /* Chu kỳ quét mỗi bóng LED 7 đoạn */
#define TIME_AUTO_INC_MS            500U    /* Chu kỳ tự động tăng giá trị mỗi 500ms khi giữ nút > 1s */

/* Định danh các kênh Software Timer độc lập */
typedef enum {
    TIMER_TRAFFIC_1S = 0,   /* Kênh 0: Đếm ngược thời gian đèn giao thông 1s */
    TIMER_BLINK_2HZ,        /* Kênh 1: Chớp tắt LED 2Hz trong Mode 2, 3, 4 */
    TIMER_7SEG_SCAN,        /* Kênh 2: Quét đa hợp 4 LED 7 đoạn */
    TIMER_AUTO_INC,         /* Kênh 3: Tự động tăng mỗi 500ms khi nhấn giữ nút INC > 1s */
    NO_OF_TIMERS            /* Tổng số kênh Software Timer */
} TimerIndex_t;

/* Nguyên mẫu các hàm quản lý Software Timer */
void setTimer(TimerIndex_t index, uint32_t duration_ms);
uint8_t isTimerExpired(TimerIndex_t index);
void clearTimer(TimerIndex_t index);
void timer_run(void);

#endif /* INC_TIMER_H_ */
