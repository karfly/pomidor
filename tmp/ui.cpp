#include "ui.h"
#include "Arduino.h"
#include "SD_MMC.h"
#include "lvgl.h"

#include "timer.h"

extern void deep_sleep();

void ui_begin()
{
    lv_obj_t *cout = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cout, LV_PCT(100), LV_PCT(100));
    lv_obj_set_scroll_dir(cout, LV_DIR_NONE);

    Timer timer(10000);
    timer.start(millis());

    lv_obj_t *timer_label = lv_label_create(cout);
    lv_obj_align(timer_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_height(timer_label, 50);

    lv_obj_add_event_cb(
        timer_label,
    [](lv_event_t *e) {
        lv_obj_t *label = (lv_obj_t *)lv_event_get_target(e);
        lv_msg_t *m = lv_event_get_msg(e);

        const unsigned long *time = (const unsigned long *)lv_msg_get_payload(m);
        int timer_current_value = timer.get_value_ms(*time);
        lv_label_set_text_fmt(label, "TIME: %d", timer_current_value);
    },
    LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(MSG_TIMER_UPDATE, timer_label, NULL);


    // lv_obj_t *btn = lv_btn_create(cout);
    // lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_set_height(btn, 50);
    // lv_obj_add_event_cb( btn, [](lv_event_t *e) {
    //     deep_sleep();
    // }, LV_EVENT_CLICKED, NULL);

    // lv_obj_t *btn_label = lv_label_create(btn);
    // lv_obj_center(btn_label);
    // lv_label_set_text(btn_label, "Click on Me Enter Deep sleep");
}
