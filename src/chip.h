#include "Wire.h"
#include "XL9535_driver.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_vendor.h"
#include "lvgl.h"
#include "pin_config.h"
#include <Arduino.h>
#include "OneButton.h"

#define TOUCH_MODULES_CST_SELF
#include "TouchLib.h"
TouchLib touch(Wire, IIC_SDA_PIN, IIC_SCL_PIN, CTS820_SLAVE_ADDRESS);


class Chip {
public:
    Chip() {};

    void init() {
        static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)
        static lv_disp_drv_t disp_drv;      // contains callback functions
        static lv_indev_drv_t indev_drv;

        // serial port setup
        Serial.begin(115200);

        // battery voltage setup
        pinMode(BAT_VOLT_PIN, ANALOG);

        _xl.begin();

        uint8_t pin = (1 << PWR_EN_PIN) | (1 << LCD_CS_PIN) | (1 << TP_RES_PIN) | (1 << LCD_SDA_PIN) | (1 << LCD_CLK_PIN) |
                    (1 << LCD_RST_PIN) | (1 << SD_CS_PIN);

        _xl.pinMode8(0, pin, OUTPUT);
        _xl.digitalWrite(PWR_EN_PIN, HIGH);

        // print_chip_info();

        // SD_init();

        delay(100);
        _xl.digitalWrite(TP_RES_PIN, LOW);
        delay(300);
        _xl.digitalWrite(TP_RES_PIN, HIGH);
        delay(300);
        pinMode(TP_INT_PIN, INPUT);

        touch.init();

        tft_init();
        esp_lcd_panel_handle_t panel_handle = NULL;
        esp_lcd_rgb_panel_config_t panel_config = {
            .clk_src = LCD_CLK_SRC_PLL160M,
            .timings =
            {
                .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
                .h_res = EXAMPLE_LCD_H_RES,
                .v_res = EXAMPLE_LCD_V_RES,
                // The following parameters should refer to LCD spec
                .hsync_pulse_width = 1,
                .hsync_back_porch = 30,
                .hsync_front_porch = 50,
                .vsync_pulse_width = 1,
                .vsync_back_porch = 30,
                .vsync_front_porch = 20,
                .flags =
                {
                    .pclk_active_neg = 1,
                },
            },
            .data_width = 16, // RGB565 in parallel mode, thus 16bit in width
            .psram_trans_align = 64,
            .hsync_gpio_num = EXAMPLE_PIN_NUM_HSYNC,
            .vsync_gpio_num = EXAMPLE_PIN_NUM_VSYNC,
            .de_gpio_num = EXAMPLE_PIN_NUM_DE,
            .pclk_gpio_num = EXAMPLE_PIN_NUM_PCLK,
            .data_gpio_nums =
            {
                // EXAMPLE_PIN_NUM_DATA0,
                EXAMPLE_PIN_NUM_DATA13,
                EXAMPLE_PIN_NUM_DATA14,
                EXAMPLE_PIN_NUM_DATA15,
                EXAMPLE_PIN_NUM_DATA16,
                EXAMPLE_PIN_NUM_DATA17,

                EXAMPLE_PIN_NUM_DATA6,
                EXAMPLE_PIN_NUM_DATA7,
                EXAMPLE_PIN_NUM_DATA8,
                EXAMPLE_PIN_NUM_DATA9,
                EXAMPLE_PIN_NUM_DATA10,
                EXAMPLE_PIN_NUM_DATA11,
                // EXAMPLE_PIN_NUM_DATA12,

                EXAMPLE_PIN_NUM_DATA1,
                EXAMPLE_PIN_NUM_DATA2,
                EXAMPLE_PIN_NUM_DATA3,
                EXAMPLE_PIN_NUM_DATA4,
                EXAMPLE_PIN_NUM_DATA5,
            },
            .disp_gpio_num = EXAMPLE_PIN_NUM_DISP_EN,
            .on_frame_trans_done = NULL,
            .user_ctx = NULL,
            .flags =
            {
                .fb_in_psram = 1, // allocate frame buffer in PSRAM
            },
        };
        ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &panel_handle));
        ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
        ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));


        lv_init();
        // alloc draw buffers used by LVGL from PSRAM
        lv_color_t *buf1 = (lv_color_t *)ps_malloc(EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * sizeof(lv_color_t));
        assert(buf1);
        lv_color_t *buf2 = (lv_color_t *)ps_malloc(EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * sizeof(lv_color_t));
        assert(buf2);
        lv_disp_draw_buf_init(&disp_buf, buf1, buf2, EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES);

        Serial.println("Register display driver to LVGL");
        lv_disp_drv_init(&disp_drv);
        disp_drv.hor_res = EXAMPLE_LCD_H_RES;
        disp_drv.ver_res = EXAMPLE_LCD_V_RES;
        disp_drv.flush_cb = example_lvgl_flush_cb;
        disp_drv.draw_buf = &disp_buf;
        disp_drv.user_data = panel_handle;
        lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

        lv_indev_drv_init(&indev_drv);
        indev_drv.type = LV_INDEV_TYPE_POINTER;
        indev_drv.read_cb = lv_touchpad_read;
        lv_indev_drv_register(&indev_drv);


        waitInterruptReady();

        lv_task_handler();


        pinMode(backlightPin, OUTPUT);
        //LilyGo T-RGB control backlight chip has 16 levels of adjustment range
        for (int i = 0; i < 16; ++i) {
            setBrightness(i);
            delay(30);
        }

    }
private:
    XL9535 _xl;
};