#pragma once

#define MSG_BAT_VOLT_UPDATE 1
#define MSG_TOUCH_UPDATE    2
#define MSG_WIFI_UPDATE     3
#define MSG_TOUCH_INT_UPDATE     4
#define MSG_TIMER_UPDATE    6

#include <string>
#include "stdint.h"
#include "lvgl.h"

#include "timer.h"

typedef struct {
    uint16_t x;
    uint16_t y;
} touch_point_t;

void ui_begin();

#define FT5x06_ADDR 0x38
#define CST820_ADDR 0x15
#define GT911_ADDR  0x5A

// fonts
LV_FONT_DECLARE(inter_semibold_100);
LV_FONT_DECLARE(monofonto_100);
LV_FONT_DECLARE(sf_mono_100);
LV_FONT_DECLARE(droid_sans_mono_100);
LV_FONT_DECLARE(interstate_mono_bold_100);

// styles

// #define BACKGROUND_COLOR lv_color_make(255, 255, 255)
#define BACKGROUND_COLOR lv_color_make(0, 0, 0)
// #define BACKGROUND_COLOR lv_color_make(255, 0, 0)

#define TIMER_CIRCLE_COLOR lv_color_make(255, 0, 0)
// #define TIMER_CIRCLE_COLOR lv_color_make(0, 0, 0)
// #define TIMER_CIRCLE_COLOR lv_color_make(244, 67, 54)
#define TIMER_CIRCLE_BACKGROUND_COLOR lv_color_make(135, 128, 128)
#define TIMER_CIRCLE_BACKGROUND_COLOR_OPACITY 38
#define TIMER_CIRCLE_WIDTH 12
#define TIMER_CIRCLE_RADIUS_PCT lv_pct(100)
#define TIMER_CIRCLE_MAX_VALUE 100

#define TIMER_LABEL_COLOR lv_color_make(255, 255, 255)
#define TIMER_LABEL_FONT &interstate_mono_bold_100
// #define TIMER_LABEL_COLOR lv_color_make(0, 0, 0)
// #define TIMER_LABEL_COLOR lv_color_make(244, 67, 54)

#define WORK_INTERVAL_DURATION 10 * 1000
#define REST_INTERVAL_DURATION 5 * 1000

const char *getTouchAddr();


enum IntervalType {
    WORK,
    REST
};


class PomidorUI {
public:
    PomidorUI() {
        _init_ui();

        // timer
        _timer = new Timer(5 * 60 * 1000);  // 25 minutes
    }

    void _init_ui() {
        _init_ui_screen();
        _init_ui_timer_label();
        _init_ui_timer_circle();

        // lv_timer_create(_timer_update_handler, 1000, this);
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
            PomidorUI::screen_tap_handler,
            LV_EVENT_CLICKED,
            this
        );

        // timer update event
        lv_obj_add_event_cb(
            _ui_screen,
            PomidorUI::timer_update_handler,
            LV_EVENT_MSG_RECEIVED,
            this
        );
        lv_msg_subsribe_obj(MSG_TIMER_UPDATE, _ui_screen, NULL);
    }

    void _init_ui_timer_label() {
        assert (_ui_screen != nullptr);

        _ui_timer_label = lv_label_create(_ui_screen);
        lv_obj_align(_ui_timer_label, LV_ALIGN_CENTER, 0, 0);

        // style
        static lv_style_t timer_label_style;
        lv_style_init(&timer_label_style);

        lv_style_set_text_color(&timer_label_style, TIMER_LABEL_COLOR);
        lv_style_set_text_letter_space(&timer_label_style, 10);
        // lv_style_set_size(&timer_label_style, 200);

        lv_style_set_text_font(&timer_label_style, TIMER_LABEL_FONT);


        lv_obj_add_style(_ui_timer_label, &timer_label_style, LV_PART_MAIN);
        // lv_obj_set_height(_ui_timer_label, 50);
        // lv_font_load(&lv_font_montserrat_28_compressed);
    }

    void _init_ui_timer_circle() {
        assert (_ui_screen != nullptr);

        _ui_timer_circle = lv_arc_create(_ui_screen);
        lv_arc_set_rotation(_ui_timer_circle, 270);  // now 0 is at the top
        lv_arc_set_bg_angles(_ui_timer_circle, 0, 360);
        lv_arc_set_angles(_ui_timer_circle, 0, 360);

        lv_obj_align(_ui_timer_circle, LV_ALIGN_CENTER, 0, 0);
        lv_obj_clear_flag(_ui_timer_circle, LV_OBJ_FLAG_CLICKABLE);  // make non-tappable

        // style
        static lv_style_t timer_circle_main_style;
        lv_style_init(&timer_circle_main_style);
        lv_style_set_arc_color(&timer_circle_main_style, TIMER_CIRCLE_BACKGROUND_COLOR);
        lv_style_set_arc_opa(&timer_circle_main_style, TIMER_CIRCLE_BACKGROUND_COLOR_OPACITY);
        // lv_style_set_arc_opa(&timer_circle_main_style, LV_OPA_0);
        lv_style_set_width(&timer_circle_main_style, TIMER_CIRCLE_RADIUS_PCT);
        lv_style_set_height(&timer_circle_main_style, TIMER_CIRCLE_RADIUS_PCT);
        lv_style_set_arc_width(&timer_circle_main_style, TIMER_CIRCLE_WIDTH);

        static lv_style_t timer_circle_indicator_style;
        lv_style_init(&timer_circle_indicator_style);
        lv_style_set_arc_color(&timer_circle_indicator_style, TIMER_CIRCLE_COLOR);
        // lv_style_set_width(&timer_circle_indicator_style, TIMER_CIRCLE_RADIUS_PCT);
        // lv_style_set_height(&timer_circle_indicator_style, TIMER_CIRCLE_RADIUS_PCT);
        lv_style_set_arc_width(&timer_circle_indicator_style, TIMER_CIRCLE_WIDTH);

        lv_obj_add_style(_ui_timer_circle, &timer_circle_main_style, LV_PART_MAIN);
        lv_obj_add_style(_ui_timer_circle, &timer_circle_indicator_style, LV_PART_INDICATOR);
        lv_obj_remove_style(_ui_timer_circle, NULL, LV_PART_KNOB);  // make knob not visible
    }

private:
    // event handlers
    static void timer_update_handler(lv_event_t *event);
    static void screen_tap_handler(lv_event_t *event);

    // utils
    static std::string format_time(unsigned long n_ms);

    // PomidorUIState _state = PomidorUIState::None;
    Timer *_timer = nullptr;

    // ui elements
    // - screen
    lv_obj_t *_ui_screen = nullptr;

    // - timer

    lv_obj_t *_ui_timer_label = nullptr;
    std::string _prev_ui_timer_label_text = "";
    lv_obj_t *_ui_timer_circle = nullptr;
};


void PomidorUI::timer_update_handler(lv_event_t *e) {
    PomidorUI *pomidor_ui = (PomidorUI *)lv_event_get_user_data(e);

    const unsigned long remaining_time = pomidor_ui->_timer->get_remaining_time();


    std::string current_ui_timer_label_text = lv_label_get_text(pomidor_ui->_ui_timer_label);
    std::string new_ui_timer_label_text = PomidorUI::format_time(remaining_time);
    if (new_ui_timer_label_text != current_ui_timer_label_text) {
        // update timer label
        lv_label_set_text(pomidor_ui->_ui_timer_label, new_ui_timer_label_text.c_str());

        // update timer circle
        const unsigned long duration = pomidor_ui->_timer->get_duration();
        float progress = (float)remaining_time / (float)duration;
        uint16_t new_end_angle = 360 * progress;
        lv_arc_set_end_angle(pomidor_ui->_ui_timer_circle, new_end_angle);
    }
};

void PomidorUI::screen_tap_handler(lv_event_t *e) {
    PomidorUI *pomidor_ui = (PomidorUI *)lv_event_get_user_data(e);
    Serial.println("Screen tap");

    if (pomidor_ui->_timer->is_running()) {
        pomidor_ui->_timer->pause();
    } else {
        pomidor_ui->_timer->start();
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