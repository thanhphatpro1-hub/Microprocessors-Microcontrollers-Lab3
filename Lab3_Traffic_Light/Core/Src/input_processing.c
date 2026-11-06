/*
 * input_processing.c
 *
 *  Created on: Lab 3 - CO3009 (HCMUT)
 *  Description: Full FSM Implementation for Exercises 5, 6, 7, 8, 9, and 10
 */

#include "input_processing.h"
#include "input_reading.h"
#include "led_display.h"
#include "timer.h"

/* Mảng lưu trạng thái FSM của 3 nút nhấn */
static ButtonState_t buttonState[NO_OF_BUTTONS] = {
    BUTTON_RELEASED,
    BUTTON_RELEASED,
    BUTTON_RELEASED
};

/* Cờ sự kiện kích hoạt theo sườn nhấn (Edge-triggered event flags) cho 3 nút */
static uint8_t buttonEventFlag[NO_OF_BUTTONS] = {0U, 0U, 0U};

/* Biến lưu trạng thái hiện tại của Hệ thống (4 Modes) và Pha Giao thông */
static SystemMode_t   currentMode  = MODE_INIT;
static TrafficPhase_t trafficPhase = TRAFFIC_INIT;

/* Biến lưu trữ thời gian chính thức của 3 màu đèn (1..99 giây) */
static uint8_t time_red   = DEFAULT_RED_DURATION;
static uint8_t time_amber = DEFAULT_AMBER_DURATION;
static uint8_t time_green = DEFAULT_GREEN_DURATION;

/* Biến nháp dùng để tăng/hiển thị giá trị trong các Mode 2, 3, 4 trước khi nhấn SET */
static uint8_t temp_value = DEFAULT_RED_DURATION;

/* Bộ đếm ngược số giây hiển thị của Tuyến đường 1 và Tuyến đường 2 trong Mode 1 */
static uint8_t count_road1 = DEFAULT_RED_DURATION;
static uint8_t count_road2 = DEFAULT_GREEN_DURATION;

/**
 * @brief  Tăng giá trị biến nháp temp_value thêm 1 đơn vị trong phạm vi [1..99]
 */
static void increment_temp_value(void) {
    temp_value++;
    if (temp_value > MAX_DURATION) {
        temp_value = MIN_DURATION;
    }
}

/**
 * @brief  Đồng bộ hóa thời gian giữa 3 màu đèn sau khi người dùng nhấn nút SET
 *         nhằm đảm bảo quy luật bất biến: Thời gian Đỏ = Thời gian Xanh + Thời gian Vàng
 * @param  modified_mode: Chế độ vừa thực hiện lệnh SET
 */
static void synchronize_traffic_durations(SystemMode_t modified_mode) {
    if (modified_mode == MODE_2_MOD_RED) {
        /* Thời gian Đỏ tối thiểu phải bằng 2s để Xanh >= 1s và Vàng >= 1s */
        if (time_red < 2U) {
            time_red = 2U;
        }
        if (time_red > time_amber) {
            time_green = time_red - time_amber;
        } else {
            time_amber = 1U;
            time_green = time_red - time_amber;
        }
    } else if ((modified_mode == MODE_3_MOD_AMBER) || (modified_mode == MODE_4_MOD_GREEN)) {
        uint16_t total_red = (uint16_t)time_green + (uint16_t)time_amber;
        if (total_red > MAX_DURATION) {
            time_red = MAX_DURATION;
            if (modified_mode == MODE_3_MOD_AMBER) {
                if (time_amber >= MAX_DURATION) {
                    time_amber = MAX_DURATION - 1U;
                }
                time_green = time_red - time_amber;
            } else {
                if (time_green >= MAX_DURATION) {
                    time_green = MAX_DURATION - 1U;
                }
                time_amber = time_red - time_green;
            }
        } else {
            time_red = (uint8_t)total_red;
        }
    }
}

/**
 * @brief  Thực thi FSM 3 trạng thái cho cả 3 nút nhấn (theo Hình 1.9 & Program 1.7):
 *         - Phát hiện sườn nhấn xuống để sinh đúng 1 sự kiện nhấn đơn.
 *         - Nếu giữ nút INC > 1 giây, tự động sinh sự kiện tăng mỗi 500ms.
 */
static void fsm_button_state_update(void) {
    for (uint8_t i = 0U; i < NO_OF_BUTTONS; i++) {
        switch (buttonState[i]) {
            case BUTTON_RELEASED:
                if (is_button_pressed(i)) {
                    buttonState[i]     = BUTTON_PRESSED;
                    buttonEventFlag[i] = 1U;
                }
                break;

            case BUTTON_PRESSED:
                if (!is_button_pressed(i)) {
                    buttonState[i] = BUTTON_RELEASED;
                } else {
                    if (is_button_pressed_1s(i)) {
                        buttonState[i] = BUTTON_PRESSED_MORE_THAN_1_SECOND;
                        if (i == BUTTON_INC) {
                            setTimer(TIMER_AUTO_INC, TIME_AUTO_INC_MS);
                        }
                    }
                }
                break;

            case BUTTON_PRESSED_MORE_THAN_1_SECOND:
                if (!is_button_pressed(i)) {
                    buttonState[i] = BUTTON_RELEASED;
                    if (i == BUTTON_INC) {
                        clearTimer(TIMER_AUTO_INC);
                    }
                } else {
                    if ((i == BUTTON_INC) && isTimerExpired(TIMER_AUTO_INC)) {
                        setTimer(TIMER_AUTO_INC, TIME_AUTO_INC_MS);
                        buttonEventFlag[BUTTON_INC] = 1U;
                    }
                }
                break;

            default:
                buttonState[i] = BUTTON_RELEASED;
                break;
        }
    }
}

/**
 * @brief  Đọc và xóa cờ sự kiện của nút nhấn (Đảm bảo mỗi lần nhấn chỉ xử lý 1 lần)
 * @param  index: Chỉ số nút nhấn (BUTTON_MODE, BUTTON_INC, BUTTON_SET)
 * @retval 1 nếu có sự kiện nhấn mới, 0 nếu không có
 */
static uint8_t check_and_clear_button_event(uint8_t index) {
    if (index >= NO_OF_BUTTONS) {
        return 0U;
    }
    if (buttonEventFlag[index] == 1U) {
        buttonEventFlag[index] = 0U;
        return 1U;
    }
    return 0U;
}

/**
 * @brief  Thực thi FSM điều khiển 4 pha đèn giao thông ngã tư khi ở Mode 1 (Normal Mode)
 */
static void fsm_traffic_light_normal(void) {
    switch (trafficPhase) {
        case TRAFFIC_INIT:
            count_road1  = time_red;
            count_road2  = time_green;
            set_traffic_leds(COLOR_RED, COLOR_GREEN);
            updateClockBuffer(count_road1, count_road2);
            setTimer(TIMER_TRAFFIC_1S, TIME_1S_MS);
            trafficPhase = PHASE_RED1_GREEN2;
            break;

        case PHASE_RED1_GREEN2:
            set_traffic_leds(COLOR_RED, COLOR_GREEN);
            if (isTimerExpired(TIMER_TRAFFIC_1S)) {
                setTimer(TIMER_TRAFFIC_1S, TIME_1S_MS);
                if (count_road1 > 1U) {
                    count_road1--;
                }
                if (count_road2 > 1U) {
                    count_road2--;
                } else {
                    /* Hết đèn Xanh Đường 2 -> Chuyển sang đèn Vàng Đường 2 */
                    count_road2  = time_amber;
                    trafficPhase = PHASE_RED1_AMBER2;
                }
                updateClockBuffer(count_road1, count_road2);
            }
            break;

        case PHASE_RED1_AMBER2:
            set_traffic_leds(COLOR_RED, COLOR_AMBER);
            if (isTimerExpired(TIMER_TRAFFIC_1S)) {
                setTimer(TIMER_TRAFFIC_1S, TIME_1S_MS);
                if (count_road2 > 1U) {
                    if (count_road1 > 1U) {
                        count_road1--;
                    }
                    count_road2--;
                } else {
                    /* Hết đèn Vàng Đường 2 -> Đổi chiều: Đường 1 Xanh, Đường 2 Đỏ */
                    count_road1  = time_green;
                    count_road2  = time_red;
                    trafficPhase = PHASE_GREEN1_RED2;
                }
                updateClockBuffer(count_road1, count_road2);
            }
            break;

        case PHASE_GREEN1_RED2:
            set_traffic_leds(COLOR_GREEN, COLOR_RED);
            if (isTimerExpired(TIMER_TRAFFIC_1S)) {
                setTimer(TIMER_TRAFFIC_1S, TIME_1S_MS);
                if (count_road2 > 1U) {
                    count_road2--;
                }
                if (count_road1 > 1U) {
                    count_road1--;
                } else {
                    /* Hết đèn Xanh Đường 1 -> Chuyển sang đèn Vàng Đường 1 */
                    count_road1  = time_amber;
                    trafficPhase = PHASE_AMBER1_RED2;
                }
                updateClockBuffer(count_road1, count_road2);
            }
            break;

        case PHASE_AMBER1_RED2:
            set_traffic_leds(COLOR_AMBER, COLOR_RED);
            if (isTimerExpired(TIMER_TRAFFIC_1S)) {
                setTimer(TIMER_TRAFFIC_1S, TIME_1S_MS);
                if (count_road1 > 1U) {
                    count_road1--;
                    if (count_road2 > 1U) {
                        count_road2--;
                    }
                } else {
                    /* Hết đèn Vàng Đường 1 -> Quay lại Pha 1: Đường 1 Đỏ, Đường 2 Xanh */
                    count_road1  = time_red;
                    count_road2  = time_green;
                    trafficPhase = PHASE_RED1_GREEN2;
                }
                updateClockBuffer(count_road1, count_road2);
            }
            break;

        default:
            trafficPhase = TRAFFIC_INIT;
            break;
    }
}

/**
 * @brief  Hàm FSM trung tâm điều phối toàn bộ hệ thống (gọi trong while(1) của main.c)
 */
void fsm_for_input_processing(void) {
    /* 1. Cập nhật FSM trạng thái của 3 nút nhấn */
    fsm_button_state_update();

    /* 2. Thực thi FSM 4 Chế độ hoạt động (Mode 1 -> 2 -> 3 -> 4 -> 1) */
    switch (currentMode) {
        case MODE_INIT:
            clear_all_traffic_leds();
            update7SEG(0U);
            setTimer(TIMER_7SEG_SCAN, TIME_7SEG_SCAN_MS);
            trafficPhase = TRAFFIC_INIT;
            currentMode  = MODE_1_NORMAL;
            break;

        /* =================================================================
         * MODE 1: CHẾ ĐỘ BÌNH THƯỜNG (NORMAL TRAFFIC LIGHT MODE)
         * ================================================================= */
        case MODE_1_NORMAL:
            fsm_traffic_light_normal();

            /* Nhấn Nút 1 (BUTTON_MODE) -> Chuyển sang Mode 2 (Chỉnh đèn Đỏ) */
            if (check_and_clear_button_event(BUTTON_MODE)) {
                /* Xóa các sự kiện nhấn INC hoặc SET vô tình bấm trong lúc ở Mode 1 */
                (void)check_and_clear_button_event(BUTTON_INC);
                (void)check_and_clear_button_event(BUTTON_SET);

                clear_all_traffic_leds();
                temp_value = time_red;
                updateClockBuffer(2U, temp_value);
                setTimer(TIMER_BLINK_2HZ, TIME_BLINK_2HZ_MS);
                currentMode = MODE_2_MOD_RED;
            }
            break;

        /* =================================================================
         * MODE 2: CHỈNH THỜI GIAN ĐÈN ĐỎ (EXERCISES 6 & 7)
         * ================================================================= */
        case MODE_2_MOD_RED:
            /* Chớp tắt 4 LED Đỏ ở tần số 2Hz (đảo trạng thái mỗi 250ms) */
            if (isTimerExpired(TIMER_BLINK_2HZ)) {
                setTimer(TIMER_BLINK_2HZ, TIME_BLINK_2HZ_MS);
                toggle_traffic_color_2hz(COLOR_RED);
            }

            /* Nhấn hoặc giữ Nút 2 (BUTTON_INC) -> Tăng temp_value (1..99) */
            if (check_and_clear_button_event(BUTTON_INC)) {
                increment_temp_value();
                updateClockBuffer(2U, temp_value);
            }

            /* Nhấn Nút 3 (BUTTON_SET) -> Lưu giá trị mới cho đèn Đỏ */
            if (check_and_clear_button_event(BUTTON_SET)) {
                time_red = temp_value;
                synchronize_traffic_durations(MODE_2_MOD_RED);
                temp_value = time_red;
                updateClockBuffer(2U, temp_value);
            }

            /* Nhấn Nút 1 (BUTTON_MODE) -> Chuyển sang Mode 3 (Chỉnh đèn Vàng) */
            if (check_and_clear_button_event(BUTTON_MODE)) {
                clear_all_traffic_leds();
                temp_value = time_amber;
                updateClockBuffer(3U, temp_value);
                setTimer(TIMER_BLINK_2HZ, TIME_BLINK_2HZ_MS);
                currentMode = MODE_3_MOD_AMBER;
            }
            break;

        /* =================================================================
         * MODE 3: CHỈNH THỜI GIAN ĐÈN VÀNG (EXERCISES 6 & 8)
         * ================================================================= */
        case MODE_3_MOD_AMBER:
            /* Chớp tắt 4 LED Vàng ở tần số 2Hz */
            if (isTimerExpired(TIMER_BLINK_2HZ)) {
                setTimer(TIMER_BLINK_2HZ, TIME_BLINK_2HZ_MS);
                toggle_traffic_color_2hz(COLOR_AMBER);
            }

            /* Nhấn hoặc giữ Nút 2 (BUTTON_INC) -> Tăng temp_value (1..99) */
            if (check_and_clear_button_event(BUTTON_INC)) {
                increment_temp_value();
                updateClockBuffer(3U, temp_value);
            }

            /* Nhấn Nút 3 (BUTTON_SET) -> Lưu giá trị mới cho đèn Vàng */
            if (check_and_clear_button_event(BUTTON_SET)) {
                time_amber = temp_value;
                synchronize_traffic_durations(MODE_3_MOD_AMBER);
                temp_value = time_amber;
                updateClockBuffer(3U, temp_value);
            }

            /* Nhấn Nút 1 (BUTTON_MODE) -> Chuyển sang Mode 4 (Chỉnh đèn Xanh) */
            if (check_and_clear_button_event(BUTTON_MODE)) {
                clear_all_traffic_leds();
                temp_value = time_green;
                updateClockBuffer(4U, temp_value);
                setTimer(TIMER_BLINK_2HZ, TIME_BLINK_2HZ_MS);
                currentMode = MODE_4_MOD_GREEN;
            }
            break;

        /* =================================================================
         * MODE 4: CHỈNH THỜI GIAN ĐÈN XANH (EXERCISES 6 & 9)
         * ================================================================= */
        case MODE_4_MOD_GREEN:
            /* Chớp tắt 4 LED Xanh ở tần số 2Hz */
            if (isTimerExpired(TIMER_BLINK_2HZ)) {
                setTimer(TIMER_BLINK_2HZ, TIME_BLINK_2HZ_MS);
                toggle_traffic_color_2hz(COLOR_GREEN);
            }

            /* Nhấn hoặc giữ Nút 2 (BUTTON_INC) -> Tăng temp_value (1..99) */
            if (check_and_clear_button_event(BUTTON_INC)) {
                increment_temp_value();
                updateClockBuffer(4U, temp_value);
            }

            /* Nhấn Nút 3 (BUTTON_SET) -> Lưu giá trị mới cho đèn Xanh */
            if (check_and_clear_button_event(BUTTON_SET)) {
                time_green = temp_value;
                synchronize_traffic_durations(MODE_4_MOD_GREEN);
                temp_value = time_green;
                updateClockBuffer(4U, temp_value);
            }

            /* Nhấn Nút 1 (BUTTON_MODE) -> Quay trở lại Mode 1 (Normal Mode) */
            if (check_and_clear_button_event(BUTTON_MODE)) {
                clear_all_traffic_leds();
                trafficPhase = TRAFFIC_INIT;
                currentMode  = MODE_1_NORMAL;
            }
            break;

        default:
            currentMode = MODE_INIT;
            break;
    }

    /* 3. Duy trì quét đa hợp 4 bóng LED 7 đoạn liên tục ở tất cả các Mode */
    led_7seg_scanning_run();
}
