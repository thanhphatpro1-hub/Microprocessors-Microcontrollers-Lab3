/*
 * input_processing.h
 *
 *  Created on: Lab 3 - CO3009 (HCMUT)
 *  Description: Header for Button FSM, 4-Mode FSM, and Traffic Light Sub-FSM
 */

#ifndef INC_INPUT_PROCESSING_H_
#define INC_INPUT_PROCESSING_H_

#include "main.h"
#include <stdint.h>

/* Giá trị thời gian khởi tạo mặc định (Đơn vị: Giây, thỏa Đỏ = Xanh + Vàng) */
#define DEFAULT_RED_DURATION        5U
#define DEFAULT_AMBER_DURATION      2U
#define DEFAULT_GREEN_DURATION      3U

/* Giới hạn dải chỉnh sửa thời gian theo đặc tả Section 5.1 (1 - 99) */
#define MIN_DURATION                1U
#define MAX_DURATION                99U

/* 1. Trạng thái của từng nút nhấn (Program 1.7 - Trang 19) */
typedef enum {
    BUTTON_RELEASED = 0,
    BUTTON_PRESSED,
    BUTTON_PRESSED_MORE_THAN_1_SECOND
} ButtonState_t;

/* 2. Trạng thái 4 chế độ hoạt động của hệ thống (Section 5.1 - Trang 21) */
typedef enum {
    MODE_INIT = 0,
    MODE_1_NORMAL,          /* Mode 1: Chạy ứng dụng đèn giao thông bình thường */
    MODE_2_MOD_RED,         /* Mode 2: Chỉnh thời gian đèn Đỏ (Chớp 4 LED Đỏ 2Hz) */
    MODE_3_MOD_AMBER,       /* Mode 3: Chỉnh thời gian đèn Vàng (Chớp 4 LED Vàng 2Hz) */
    MODE_4_MOD_GREEN        /* Mode 4: Chỉnh thời gian đèn Xanh (Chớp 4 LED Xanh 2Hz) */
} SystemMode_t;

/* 3. Trạng thái 4 pha điều khiển đèn giao thông tại ngã tư trong Mode 1 */
typedef enum {
    TRAFFIC_INIT = 0,
    PHASE_RED1_GREEN2,      /* Đường 1 Đỏ   - Đường 2 Xanh */
    PHASE_RED1_AMBER2,      /* Đường 1 Đỏ   - Đường 2 Vàng */
    PHASE_GREEN1_RED2,      /* Đường 1 Xanh - Đường 2 Đỏ   */
    PHASE_AMBER1_RED2       /* Đường 1 Vàng - Đường 2 Đỏ   */
} TrafficPhase_t;

/* Nguyên mẫu hàm chính gọi trong vòng lặp while(1) của main.c (Program 1.8) */
void fsm_for_input_processing(void);

#endif /* INC_INPUT_PROCESSING_H_ */
