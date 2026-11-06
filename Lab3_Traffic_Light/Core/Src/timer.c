/*
 * timer.c
 *
 *  Created on: Lab 3 - CO3009 (HCMUT)
 *  Description: Software Timer Implementation & TIM2 Interrupt Callback
 */

#include "timer.h"
#include "input_reading.h"

/* Mảng lưu số tick đếm ngược của từng kênh Software Timer */
static uint32_t timer_counter[NO_OF_TIMERS]      = {0U, 0U, 0U, 0U};

/* Mảng lưu cờ báo hết giờ (1 = Expired, 0 = Running/Idle) */
static uint8_t  timer_flag[NO_OF_TIMERS]         = {0U, 0U, 0U, 0U};

/* Mảng lưu phần dư mili-giây (Modulo Accumulator) giúp không bị lệch tần số khi TICK_MS = 100ms */
static uint32_t timer_remainder_ms[NO_OF_TIMERS] = {0U, 0U, 0U, 0U};

/**
 * @brief  Cài đặt thời gian cho một kênh Software Timer
 * @param  index: Chỉ số kênh Timer (TimerIndex_t)
 * @param  duration_ms: Thời gian định thời tính bằng mili-giây (ms)
 */
void setTimer(TimerIndex_t index, uint32_t duration_ms) {
    if (index >= NO_OF_TIMERS) {
        return;
    }

    if (duration_ms == 0U) {
        timer_counter[index]      = 0U;
        timer_flag[index]         = 1U;
        timer_remainder_ms[index] = 0U;
        return;
    }

    /* Cộng dồn thời gian yêu cầu với phần dư của chu kỳ trước (Exercise 4) */
    uint32_t total_ms = duration_ms + timer_remainder_ms[index];
    uint32_t ticks    = total_ms / TICK_MS;

    if (ticks > 0U) {
        timer_remainder_ms[index] = total_ms % TICK_MS;
    } else {
        /* Trường hợp duration_ms < TICK_MS (ví dụ quét LED 50ms khi TICK_MS = 100ms),
         * ép tối thiểu 1 tick để hệ thống luôn chạy liên tục không bị treo */
        ticks                     = 1U;
        timer_remainder_ms[index] = 0U;
    }

    timer_counter[index] = ticks;
    timer_flag[index]    = 0U;
}

/**
 * @brief  Kiểm tra xem một kênh Software Timer đã đếm về 0 hay chưa
 * @param  index: Chỉ số kênh Timer
 * @retval 1 nếu đã hết hạn, 0 nếu chưa hết hạn
 */
uint8_t isTimerExpired(TimerIndex_t index) {
    if (index >= NO_OF_TIMERS) {
        return 0U;
    }
    return (timer_flag[index] == 1U) ? 1U : 0U;
}

/**
 * @brief  Xóa cờ, xóa phần dư và dừng một kênh Software Timer
 * @param  index: Chỉ số kênh Timer
 */
void clearTimer(TimerIndex_t index) {
    if (index >= NO_OF_TIMERS) {
        return;
    }
    timer_counter[index]      = 0U;
    timer_flag[index]         = 0U;
    timer_remainder_ms[index] = 0U;
}

/**
 * @brief  Giảm bộ đếm của tất cả các kênh Software Timer đang hoạt động.
 *         Hàm này được gọi định kỳ mỗi TICK_MS bên trong ISR của TIM2.
 */
void timer_run(void) {
    for (uint8_t i = 0U; i < (uint8_t)NO_OF_TIMERS; i++) {
        if (timer_counter[i] > 0U) {
            timer_counter[i]--;
            if (timer_counter[i] == 0U) {
                timer_flag[i] = 1U;
            }
        }
    }
}

/**
 * @brief  Hàm Callback ngắt cứng Timer 2 theo đúng Mục 4.3.3 của tài liệu Lab 3
 * @param  htim: Con trỏ cấu trúc quản lý Timer của thư viện HAL
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        button_reading();
        timer_run();
    }
}
