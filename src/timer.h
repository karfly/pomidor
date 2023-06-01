#include <cassert>
#include <algorithm>
#include <Arduino.h>
#include <lvgl.h>

class Timer {
public:
    Timer(unsigned long duration):
        _duration(duration),
        _start_time(-1),
        _pause_time(-1)
    {}

    void start() {
        if (_start_time == -1) {
            // _start_time = millis();
            _start_time = lv_tick_get();

        } else if (_pause_time != -1) {
            // unsigned long elapsed_pause = millis() - _pause_time;
            unsigned long elapsed_pause = lv_tick_get() - _pause_time;
            _start_time += elapsed_pause;
            _pause_time = -1;
        }
    }

    void pause() {
        if (_start_time != -1 && _pause_time == -1) {
            // _pause_time = millis();
            _pause_time = lv_tick_get();
        }
    }

    unsigned long get_remaining_time() {
        if (_start_time == -1) {  // not started
            return _duration;
        }

        unsigned long elapsed_time = -1;
        if (_pause_time != -1) {  // paused
            elapsed_time = _pause_time - _start_time;
        } else {  // running
            // elapsed_time = millis() - _start_time;
            elapsed_time = lv_tick_get() - _start_time;
        }

        if (elapsed_time >= _duration) {
            return 0;
        } else {
            return _duration - elapsed_time;
        }
    }

    bool is_running() {
        return _start_time != -1 && _pause_time == -1;
    }

    unsigned long get_duration() {
        return _duration;
    }

private:
    // TODO: use lv_tick_t instead of unsigned long
    unsigned long _duration;
    unsigned long _start_time;
    unsigned long _pause_time;
};
