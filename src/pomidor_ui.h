#pragma once

#define MSG_BAT_VOLT_UPDATE 1
#define MSG_TOUCH_UPDATE    2
#define MSG_WIFI_UPDATE     3
#define MSG_TOUCH_INT_UPDATE     4
#define MSG_TIMER_UPDATE    6

#include <string>
#include "stdint.h"
#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main.cpp"


#include "timer.h"

typedef struct {
    uint16_t x;
    uint16_t y;
} touch_point_t;

void ui_begin();

#define FT5x06_ADDR 0x38
#define CST820_ADDR 0x15
#define GT911_ADDR  0x5A

// static
LV_FONT_DECLARE(inter_semibold_100);
LV_FONT_DECLARE(monofonto_100);
LV_FONT_DECLARE(sf_mono_100);
LV_FONT_DECLARE(droid_sans_mono_100);
LV_FONT_DECLARE(interstate_mono_bold_100);
LV_FONT_DECLARE(interstate_mono_bold_30);

LV_IMG_DECLARE(next_interval_button_img);
LV_IMG_DECLARE(pause_button_img);
LV_IMG_DECLARE(play_button_img);
LV_IMG_DECLARE(work_gif);
LV_IMG_DECLARE(rest_gif);
LV_IMG_DECLARE(pause_gif);

// styles`

// #define BACKGROUND_COLOR lv_color_make(255, 255, 255)
#define BACKGROUND_COLOR lv_color_make(0, 0, 0)
// #define BACKGROUND_COLOR lv_color_make(255, 0, 0)

#define TIMER_CIRCLE_WORK_COLOR lv_color_make(255, 0, 0)
#define TIMER_CIRCLE_REST_COLOR lv_color_make(0, 255, 0)
// #define TIMER_CIRCLE_COLOR lv_color_make(0, 0, 0)
// #define TIMER_CIRCLE_COLOR lv_color_make(0, 255, 0)
#define TIMER_CIRCLE_BACKGROUND_COLOR lv_color_make(48, 48, 48)
// #define TIMER_CIRCLE_BACKGROUND_COLOR_OPACITY 38
#define TIMER_CIRCLE_WIDTH 12
#define TIMER_CIRCLE_RADIUS_PCT lv_pct(100)
#define TIMER_CIRCLE_MIN_ANGLE 5
#define TIMER_CIRCLE_MAX_ANGLE 360 - TIMER_CIRCLE_MIN_ANGLE

#define TIMER_LABEL_COLOR lv_color_make(255, 255, 255)
#define TIMER_LABEL_FONT &interstate_mono_bold_100
#define TIMER_LABEL_LETTER_SPACE 1
// #define TIMER_LABEL_COLOR lv_color_make(0, 0, 0)
// #define TIMER_LABEL_COLOR lv_color_make(244, 67, 54)

#define TAP_TO_START_LABEL_FONT &interstate_mono_bold_30
#define TAP_TO_START_LABEL_TEXT "Tap to start"
// #define TAP_TO_START_LABEL_COLOR lv_color_make(255, 255, 255)
#define TAP_TO_START_LABEL_COLOR lv_color_make(77, 77, 77)
#define TAP_TO_START_LABEL_COLOR_OPACITY 255

#define WORK_INTERVAL_DURATION 25 * 60 * 1000
#define REST_INTERVAL_DURATION 5 * 60 * 1000

const char *getTouchAddr();


enum class PomidorIntervalType {
    None,
    Work,
    Rest
};

enum class PomidorUIState {
    None,
    ReadyToStart,
    Running,
    Paused
};

TaskHandle_t BuzzerTaskHandle = NULL;

void buzzer_task(void *pvParameters) {
    // tone(BUZZER_PIN, 1000);
    // delay(1000);
    // noTone(BUZZER_PIN);
    // delay(1000);
    // tone(BUZZER_PIN, 1000);
    // delay(1000);
    // noTone(BUZZER_PIN);
    // delay(1000);
    vTaskDelete(NULL);
}



class PomidorUI {
public:
    PomidorUI() {
        // states
        _current_ui_state = PomidorUIState::ReadyToStart;
        _current_interval_type = PomidorIntervalType::Work;

        // timer
        _timer = new Timer(WORK_INTERVAL_DURATION);

        // ui
        _init_ui();

        // buzzer
        // xTaskCreate(buzzer_task, "buzzer_task", 2048, NULL, tskIDLE_PRIORITY, NULL);
        // Serial.println("buzzer_task created");
    }

    void _init_ui() {
        _init_ui_screen();
        _init_ui_timer();
        // TODO: _init_ui_tap_to_start_label();
        _init_ui_next_interval_button();
        _init_ui_play_pause_button();
        _init_gifs();

        _ui_deep_sleep_button = lv_btn_create(_ui_screen);

        lv_obj_align(_ui_next_interval_button, LV_ALIGN_CENTER, 0, 100);

        // event handler
        lv_obj_add_event_cb(
            _ui_deep_sleep_button,
            PomidorUI::_ui_deep_sleep_button_handler,
            LV_EVENT_ALL,
            this
        );

    }

    void _init_ui_screen() {
        assert (_ui_screen == nullptr);

        _ui_screen = lv_obj_create(lv_scr_act());

        // style
        static lv_style_t screen_style;
        lv_style_init(&screen_style);
        lv_style_set_bg_color(&screen_style, BACKGROUND_COLOR);
        lv_style_set_border_width(&screen_style, 0);
        lv_style_set_width(&screen_style, lv_pct(100));
        lv_style_set_height(&screen_style, lv_pct(100));
        lv_style_set_pad_all(&screen_style, -1);  // small gap to avoid flickering on screen edges

        lv_obj_add_style(_ui_screen, &screen_style, LV_PART_MAIN);

        // screen tap event
        lv_obj_add_event_cb(
            _ui_screen,
            PomidorUI::_ui_screen_tap_handler,
            LV_EVENT_CLICKED,
            this
        );
    }

    void _init_ui_timer() {
        assert (_ui_screen != nullptr);

        // label
        _ui_timer_label = lv_label_create(_ui_screen);
        lv_obj_align(_ui_timer_label, LV_ALIGN_CENTER, 0, 0);

        // style
        static lv_style_t timer_label_style;
        lv_style_init(&timer_label_style);

        lv_style_set_text_color(&timer_label_style, TIMER_LABEL_COLOR);
        lv_style_set_text_align(&timer_label_style, LV_TEXT_ALIGN_CENTER);
        lv_style_set_text_letter_space(&timer_label_style, TIMER_LABEL_LETTER_SPACE);
        // lv_style_set_size(&timer_label_style, 200);

        lv_style_set_text_font(&timer_label_style, TIMER_LABEL_FONT);


        lv_obj_add_style(_ui_timer_label, &timer_label_style, LV_PART_MAIN);
        // lv_obj_set_height(_ui_timer_label, 50);
        // lv_font_load(&lv_font_montserrat_28_compressed);

        // circle
        _ui_timer_circle = lv_arc_create(_ui_screen);
        lv_arc_set_rotation(_ui_timer_circle, 270);  // now 0 is at the top
        lv_arc_set_bg_angles(_ui_timer_circle, TIMER_CIRCLE_MIN_ANGLE, TIMER_CIRCLE_MAX_ANGLE);
        lv_arc_set_angles(_ui_timer_circle, TIMER_CIRCLE_MIN_ANGLE, TIMER_CIRCLE_MAX_ANGLE);

        lv_obj_align(_ui_timer_circle, LV_ALIGN_CENTER, 0, 0);
        lv_obj_clear_flag(_ui_timer_circle, LV_OBJ_FLAG_CLICKABLE);  // make non-tappable

        // style
        static lv_style_t timer_circle_main_style;
        lv_style_init(&timer_circle_main_style);
        lv_style_set_arc_color(&timer_circle_main_style, TIMER_CIRCLE_BACKGROUND_COLOR);
        // lv_style_set_arc_opa(&timer_circle_main_style, TIMER_CIRCLE_BACKGROUND_COLOR_OPACITY);
        // lv_style_set_arc_opa(&timer_circle_main_style, LV_OPA_0);
        lv_style_set_width(&timer_circle_main_style, TIMER_CIRCLE_RADIUS_PCT);
        lv_style_set_height(&timer_circle_main_style, TIMER_CIRCLE_RADIUS_PCT);
        lv_style_set_arc_width(&timer_circle_main_style, TIMER_CIRCLE_WIDTH);

        static lv_style_t timer_circle_indicator_style;
        lv_style_init(&timer_circle_indicator_style);
        lv_style_set_arc_color(&timer_circle_indicator_style, TIMER_CIRCLE_WORK_COLOR);
        // lv_style_set_width(&timer_circle_indicator_style, TIMER_CIRCLE_RADIUS_PCT);
        // lv_style_set_height(&timer_circle_indicator_style, TIMER_CIRCLE_RADIUS_PCT);
        lv_style_set_arc_width(&timer_circle_indicator_style, TIMER_CIRCLE_WIDTH);

        lv_obj_add_style(_ui_timer_circle, &timer_circle_main_style, LV_PART_MAIN);
        lv_obj_add_style(_ui_timer_circle, &timer_circle_indicator_style, LV_PART_INDICATOR);
        lv_obj_remove_style(_ui_timer_circle, NULL, LV_PART_KNOB);  // make knob not visible

        // tap to start label
        _ui_tap_to_start_label = lv_label_create(_ui_screen);
        lv_label_set_text(_ui_tap_to_start_label, TAP_TO_START_LABEL_TEXT);
        lv_obj_align(_ui_tap_to_start_label, LV_ALIGN_CENTER, 0, 70);

        // style
        static lv_style_t tap_to_start_label_style;
        lv_style_init(&tap_to_start_label_style);

        lv_style_set_text_color(&tap_to_start_label_style, TAP_TO_START_LABEL_COLOR);
        lv_style_set_text_opa(&tap_to_start_label_style, TAP_TO_START_LABEL_COLOR_OPACITY);
        lv_style_set_text_align(&tap_to_start_label_style, LV_TEXT_ALIGN_CENTER);
        // lv_style_set_text_letter_space(&tap_to_start_label_style, );
        // lv_style_set_size(&timer_label_style, 200);
        lv_style_set_text_font(&tap_to_start_label_style, TAP_TO_START_LABEL_FONT);

        lv_obj_add_style(_ui_tap_to_start_label, &tap_to_start_label_style, LV_PART_MAIN);

        // clock
        _ui_timer_clock = lv_timer_create(PomidorUI::_ui_timer_update_handler, 250, this);
    }

    void _init_ui_play_pause_button() {
        static lv_style_t pressed_style;
        lv_style_init(&pressed_style);
        lv_style_set_img_recolor_opa(&pressed_style, LV_OPA_30);
        lv_style_set_img_recolor(&pressed_style, lv_color_black());

        _ui_play_pause_button = lv_imgbtn_create(_ui_screen);
        lv_imgbtn_set_src(_ui_play_pause_button, LV_IMGBTN_STATE_RELEASED, NULL, &pause_button_img, NULL);
        lv_imgbtn_set_src(_ui_play_pause_button, LV_IMGBTN_STATE_PRESSED, NULL, &pause_button_img, NULL);
        lv_imgbtn_set_src(_ui_play_pause_button, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &play_button_img, NULL);
        lv_imgbtn_set_src(_ui_play_pause_button, LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &play_button_img, NULL);
        lv_obj_add_flag(_ui_play_pause_button, LV_OBJ_FLAG_CHECKABLE);

        lv_obj_align(_ui_play_pause_button, LV_ALIGN_CENTER, -70, 120);
        lv_obj_set_size(_ui_play_pause_button, pause_button_img.header.w, pause_button_img.header.h);
        lv_obj_add_flag(_ui_play_pause_button, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_style(_ui_play_pause_button, &pressed_style, LV_STATE_PRESSED);

        // event handler
        lv_obj_add_event_cb(
            _ui_play_pause_button,
            PomidorUI::_ui_play_pause_button_handler,
            LV_EVENT_ALL,
            this
        );
    }

    void _init_ui_next_interval_button() {
        static lv_style_t pressed_style;
        lv_style_init(&pressed_style);
        lv_style_set_img_recolor_opa(&pressed_style, LV_OPA_30);
        lv_style_set_img_recolor(&pressed_style, lv_color_black());


        _ui_next_interval_button = lv_imgbtn_create(_ui_screen);
        lv_imgbtn_set_src(_ui_next_interval_button, LV_IMGBTN_STATE_RELEASED, NULL, &next_interval_button_img, NULL);
        lv_imgbtn_set_src(_ui_next_interval_button, LV_IMGBTN_STATE_PRESSED, NULL, &next_interval_button_img, NULL);

        lv_obj_align(_ui_next_interval_button, LV_ALIGN_CENTER, 70, 120);
        lv_obj_set_size(_ui_next_interval_button, next_interval_button_img.header.h, next_interval_button_img.header.h);
        lv_obj_add_flag(_ui_next_interval_button, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_style(_ui_next_interval_button, &pressed_style, LV_STATE_PRESSED);

        // event handler
        lv_obj_add_event_cb(
            _ui_next_interval_button,
            PomidorUI::_ui_next_interval_button_handler,
            LV_EVENT_ALL,
            this
        );
    }

    void _change_ui_state(PomidorUIState new_ui_state) {
        if (_current_ui_state == PomidorUIState::ReadyToStart && new_ui_state == PomidorUIState::Running) {
            _timer->start();
            lv_obj_add_flag(_ui_tap_to_start_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(_ui_play_pause_button, LV_OBJ_FLAG_HIDDEN);
            lv_imgbtn_set_state(_ui_play_pause_button, LV_IMGBTN_STATE_RELEASED);
            lv_obj_clear_flag(_ui_next_interval_button, LV_OBJ_FLAG_HIDDEN);
            _update_ui_timer();
        } else if (_current_ui_state == PomidorUIState::Running && new_ui_state == PomidorUIState::Paused) {
            lv_imgbtn_set_state(_ui_play_pause_button, LV_IMGBTN_STATE_CHECKED_RELEASED);
            _timer->pause();
            _update_ui_timer();
        } else if (_current_ui_state == PomidorUIState::Paused && new_ui_state == PomidorUIState::Running) {
            lv_imgbtn_set_state(_ui_play_pause_button, LV_IMGBTN_STATE_RELEASED);
            _timer->start();
            _update_ui_timer();
        } else if (new_ui_state == PomidorUIState::ReadyToStart) {
            if (_current_interval_type == PomidorIntervalType::Work) {
                _current_interval_type = PomidorIntervalType::Rest;
                _timer = new Timer(REST_INTERVAL_DURATION);
                lv_obj_set_style_arc_color(_ui_timer_circle, TIMER_CIRCLE_REST_COLOR, LV_PART_INDICATOR);
            } else {
                _current_interval_type = PomidorIntervalType::Work;
                _timer = new Timer(WORK_INTERVAL_DURATION);
                lv_obj_set_style_arc_color(_ui_timer_circle, TIMER_CIRCLE_WORK_COLOR, LV_PART_INDICATOR);
            }

            lv_obj_add_flag(_ui_play_pause_button, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(_ui_next_interval_button, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(_ui_tap_to_start_label, LV_OBJ_FLAG_HIDDEN);
            _update_ui_timer();
        } else {
            assert (false && "invalid state transition");
        };

        // gifs
        if (new_ui_state == PomidorUIState::Paused || new_ui_state == PomidorUIState::ReadyToStart) {
            lv_gif_set_src(_ui_gif, &pause_gif);
        } else {
            if (_current_interval_type == PomidorIntervalType::Work) {
                lv_gif_set_src(_ui_gif, &work_gif);
            } else {
                lv_gif_set_src(_ui_gif, &rest_gif);
            }
        }

        Serial.printf("UI state changed from %d to %d\n", _current_ui_state, new_ui_state);

        _current_ui_state = new_ui_state;
    }

    void _init_gifs() {
        _ui_gif = lv_gif_create(_ui_screen);
        lv_gif_set_src(_ui_gif, &pause_gif);
        lv_obj_align(_ui_gif, LV_ALIGN_CENTER, 0, -120);

        // _work_gif = lv_gif_create(_ui_screen);
        // lv_gif_set_src(_work_gif, &work_gif);
        // lv_obj_align(_work_gif, LV_ALIGN_CENTER, 0, -120);
        // lv_obj_add_flag(_work_gif, LV_OBJ_FLAG_HIDDEN);

        // _rest_gif = lv_gif_create(_ui_screen);
        // lv_gif_set_src(_rest_gif, &rest_gif);
        // lv_obj_align(_rest_gif, LV_ALIGN_CENTER, 0, -120);
        // lv_obj_add_flag(_rest_gif, LV_OBJ_FLAG_HIDDEN);

        // _pause_gif = lv_gif_create(_ui_screen);
        // lv_gif_set_src(_pause_gif, &pause_gif);
        // lv_obj_align(_pause_gif, LV_ALIGN_CENTER, 0, -120);
    }

    void _update_ui_timer() {
        const unsigned long remaining_time = _timer->get_remaining_time();

        std::string current_ui_timer_label_text = lv_label_get_text(_ui_timer_label);
        std::string new_ui_timer_label_text = PomidorUI::format_time(remaining_time);

        if (new_ui_timer_label_text != current_ui_timer_label_text) {
            // update timer label
            lv_label_set_text(_ui_timer_label, new_ui_timer_label_text.c_str());

            // update timer circle
            const unsigned long duration = _timer->get_duration();
            float progress = (float)remaining_time / (float)duration;
            uint16_t new_end_angle = TIMER_CIRCLE_MIN_ANGLE + progress * (TIMER_CIRCLE_MAX_ANGLE - TIMER_CIRCLE_MIN_ANGLE);
            lv_arc_set_end_angle(_ui_timer_circle, new_end_angle);
        }

        if (remaining_time == 0) {
            _change_ui_state(PomidorUIState::ReadyToStart);
            return;
        }
    }

private:
    // event handlers
    static void _ui_timer_update_handler(lv_timer_t *timer);
    static void _ui_screen_tap_handler(lv_event_t *event);
    static void _ui_next_interval_button_handler(lv_event_t *event);
    static void _ui_play_pause_button_handler(lv_event_t *event);
    static void _play_music(lv_event_t *event);
    static void _ui_deep_sleep_button_handler(lv_event_t *event);

    // utils
    static std::string format_time(unsigned long n_ms);

    Timer *_timer = nullptr;
    PomidorUIState _current_ui_state = PomidorUIState::None;
    PomidorIntervalType _current_interval_type = PomidorIntervalType::None;

    // ui elements
    // - screen
    lv_obj_t *_ui_screen = nullptr;

    // - timer
    lv_timer_t *_ui_timer_clock = nullptr;

    lv_obj_t *_ui_timer_label = nullptr;
    lv_obj_t *_ui_timer_circle = nullptr;

    lv_obj_t *_ui_tap_to_start_label = nullptr;
    lv_obj_t *_ui_next_interval_button = nullptr;
    lv_obj_t *_ui_play_pause_button = nullptr;

    // - gifs
    lv_obj_t *_ui_gif = nullptr;
    // lv_obj_t *_work_gif = nullptr;
    // lv_obj_t *_rest_gif = nullptr;
    // lv_obj_t *_pause_gif = nullptr;

    // deep sleep
    lv_obj_t *_ui_deep_sleep_button = nullptr;
};

void PomidorUI::_ui_timer_update_handler(lv_timer_t *timer) {
    PomidorUI *pomidor_ui = (PomidorUI *)timer->user_data;
    pomidor_ui->_update_ui_timer();
};

void PomidorUI::_ui_screen_tap_handler(lv_event_t *e) {
    Serial.println("Screen tapped");
    PomidorUI *pomidor_ui = (PomidorUI *)lv_event_get_user_data(e);

    if (pomidor_ui->_current_ui_state == PomidorUIState::ReadyToStart) {
        pomidor_ui->_change_ui_state(PomidorUIState::Running);
    }
};

void PomidorUI::_ui_next_interval_button_handler(lv_event_t *e) {
    PomidorUI *pomidor_ui = (PomidorUI *)lv_event_get_user_data(e);

    if (lv_event_get_code(e) == LV_EVENT_RELEASED) {
        Serial.println("Next interval button released");
        pomidor_ui->_change_ui_state(PomidorUIState::ReadyToStart);
    };
};

void PomidorUI::_ui_play_pause_button_handler(lv_event_t *e) {
    PomidorUI *pomidor_ui = (PomidorUI *)lv_event_get_user_data(e);

    if (lv_event_get_code(e) == LV_EVENT_RELEASED) {
        if (pomidor_ui->_current_ui_state == PomidorUIState::Running) {
            Serial.println("Pause button released");
            pomidor_ui->_change_ui_state(PomidorUIState::Paused);
        } else if (pomidor_ui->_current_ui_state == PomidorUIState::Paused) {
            Serial.println("Play button released");
            pomidor_ui->_change_ui_state(PomidorUIState::Running);
        } else {
            assert(false && "invalid state transition");
        }
    };
};

void PomidorUI::_play_music(lv_event_t *e) {

}

void PomidorUI::_ui_deep_sleep_button_handler(lv_event_t *e) {
    PomidorUI *pomidor_ui = (PomidorUI *)lv_event_get_user_data(e);

    if (lv_event_get_code(e) == LV_EVENT_RELEASED) {
        Serial.println("Deep sleep button released");
        deep_sleep();
    };
};

// format time in ms to string mm:ss
std::string PomidorUI::format_time(unsigned long time_ms) {
    unsigned long time_s = time_ms / 1000;
    unsigned long time_m = time_s / 60;
    time_s = time_s % 60;

    char buf[5];
    sprintf(buf, "%02lu:%02lu", time_m, time_s);
    return std::string(buf);
}