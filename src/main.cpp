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

#include "pitches.h"
#include "pomidor_ui.h"

#define TOUCH_MODULES_CST_SELF
#include "TouchLib.h"
TouchLib touch(Wire, IIC_SDA_PIN, IIC_SCL_PIN, CTS820_SLAVE_ADDRESS);


// globals
PomidorUI *pomidor_ui = nullptr;

typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; // No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

DRAM_ATTR static const lcd_init_cmd_t st_init_cmds[] = {
    {0xFF, {0x77, 0x01, 0x00, 0x00, 0x10}, 0x05},
    {0xC0, {0x3b, 0x00}, 0x02},
    {0xC1, {0x0b, 0x02}, 0x02},
    {0xC2, {0x07, 0x02}, 0x02},
    {0xCC, {0x10}, 0x01},
    {0xCD, {0x08}, 0x01}, // 用565时屏蔽    666打开
    {0xb0, {0x00, 0x11, 0x16, 0x0e, 0x11, 0x06, 0x05, 0x09, 0x08, 0x21, 0x06, 0x13, 0x10, 0x29, 0x31, 0x18}, 0x10},
    {0xb1, {0x00, 0x11, 0x16, 0x0e, 0x11, 0x07, 0x05, 0x09, 0x09, 0x21, 0x05, 0x13, 0x11, 0x2a, 0x31, 0x18}, 0x10},
    {0xFF, {0x77, 0x01, 0x00, 0x00, 0x11}, 0x05},
    {0xb0, {0x6d}, 0x01},
    {0xb1, {0x37}, 0x01},
    {0xb2, {0x81}, 0x01},
    {0xb3, {0x80}, 0x01},
    {0xb5, {0x43}, 0x01},
    {0xb7, {0x85}, 0x01},
    {0xb8, {0x20}, 0x01},
    {0xc1, {0x78}, 0x01},
    {0xc2, {0x78}, 0x01},
    {0xc3, {0x8c}, 0x01},
    {0xd0, {0x88}, 0x01},
    {0xe0, {0x00, 0x00, 0x02}, 0x03},
    {0xe1, {0x03, 0xa0, 0x00, 0x00, 0x04, 0xa0, 0x00, 0x00, 0x00, 0x20, 0x20}, 0x0b},
    {0xe2, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x0d},
    {0xe3, {0x00, 0x00, 0x11, 0x00}, 0x04},
    {0xe4, {0x22, 0x00}, 0x02},
    {0xe5, {0x05, 0xec, 0xa0, 0xa0, 0x07, 0xee, 0xa0, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x10},
    {0xe6, {0x00, 0x00, 0x11, 0x00}, 0x04},
    {0xe7, {0x22, 0x00}, 0x02},
    {0xe8, {0x06, 0xed, 0xa0, 0xa0, 0x08, 0xef, 0xa0, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x10},
    {0xeb, {0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00}, 0x07},
    {0xed, {0xff, 0xff, 0xff, 0xba, 0x0a, 0xbf, 0x45, 0xff, 0xff, 0x54, 0xfb, 0xa0, 0xab, 0xff, 0xff, 0xff}, 0x10},
    {0xef, {0x10, 0x0d, 0x04, 0x08, 0x3f, 0x1f}, 0x06},
    {0xFF, {0x77, 0x01, 0x00, 0x00, 0x13}, 0x05},
    {0xef, {0x08}, 0x01},
    {0xFF, {0x77, 0x01, 0x00, 0x00, 0x00}, 0x05},
    {0x36, {0x08}, 0x01},
    {0x3a, {0x66}, 0x01},
    {0x11, {0x00}, 0x80},
    // {0xFF, {0x77, 0x01, 0x00, 0x00, 0x12}, 0x05},
    // {0xd1, {0x81}, 0x01},
    // {0xd2, {0x06}, 0x01},
    {0x29, {0x00}, 0x80},
    {0, {0}, 0xff}
};

XL9535 xl;

const int backlightPin = EXAMPLE_PIN_NUM_BK_LIGHT;
// TaskHandle_t pvCreatedTask;


void deep_sleep(void);
void tft_init(void);
void lcd_cmd(const uint8_t cmd);
void lcd_data(const uint8_t *data, int len);

const char *getTouchAddr()
{
    return "CST820";
}

void print_chip_info(void)
{
    Serial.print("Chip: ");
    Serial.println(ESP.getChipModel());
    Serial.print("ChipRevision: ");
    Serial.println(ESP.getChipRevision());
    Serial.print("Psram size: ");
    Serial.print(ESP.getPsramSize() / 1024);
    Serial.println("KB");
    Serial.print("Flash size: ");
    Serial.print(ESP.getFlashChipSize() / 1024);
    Serial.println("KB");
    Serial.print("CPU frequency: ");
    Serial.print(ESP.getCpuFreqMHz());
    Serial.println("MHz");
}

static void example_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
    lv_disp_flush_ready(drv);
}

static void lv_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    static uint16_t lastX, lastY;
    touch_point_t p = {0};

    if (touch.read()) {
        TP_Point t = touch.getPoint(0);
        data->point.x = p.x = t.x;
        data->point.y = p.y = t.y;
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }

    lv_msg_send(MSG_TOUCH_UPDATE, &p);
}

void waitInterruptReady()
{
    Serial.println("waitInterruptReady ...");

    uint32_t timeout = millis() + 500;

    // The touch interrupt of CST820 is a high and low pulse, it is not fixed, there will be a low level every ~10 milliseconds interval
    while (timeout > millis()) {
        while (!digitalRead(TP_INT_PIN)) {
            delay(20);
            timeout = millis() + 500;
        }
    }
}

// LilyGo  T-RGB  control backlight chip has 16 levels of adjustment range
// The adjustable range is 0~15, 0 is the minimum brightness, 15 is the maximum brightness
void setBrightness(uint8_t value)
{
    static uint8_t level = 0;
    static uint8_t steps = 16;
    if (value == 0) {
        digitalWrite(backlightPin, 0);
        delay(3);
        level = 0;
        return;
    }
    if (level == 0) {
        digitalWrite(backlightPin, 1);
        level = steps;
        delayMicroseconds(30);
    }
    int from = steps - level;
    int to = steps - value;
    int num = (steps + to - from) % steps;
    for (int i = 0; i < num; i++) {
        digitalWrite(backlightPin, 0);
        digitalWrite(backlightPin, 1);
    }
    level = value;
}

void setup()
{
    static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)
    static lv_disp_drv_t disp_drv;      // contains callback functions
    static lv_indev_drv_t indev_drv;

    // serial port setup
    Serial.begin(115200);

    // battery voltage setup
    pinMode(BAT_VOLT_PIN, ANALOG);

    xl.begin();

    uint8_t pin = (1 << PWR_EN_PIN) | (1 << LCD_CS_PIN) | (1 << TP_RES_PIN) | (1 << LCD_SDA_PIN) | (1 << LCD_CLK_PIN) |
                  (1 << LCD_RST_PIN) | (1 << SD_CS_PIN);

    xl.pinMode8(0, pin, OUTPUT);
    xl.digitalWrite(PWR_EN_PIN, HIGH);

    print_chip_info();

    // SD_init();

    delay(100);
    xl.digitalWrite(TP_RES_PIN, LOW);
    delay(300);
    xl.digitalWrite(TP_RES_PIN, HIGH);
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

    // start UI
    pomidor_ui = new PomidorUI();
    Serial.println("Finished setup");

    // buzzer
    // pinMode(BUZZER_PIN, OUTPUT);  // TODO: when OUTPUT – screen doesn't work
    // digitalWrite(BUZZER_PIN, LOW);
    // pinMode(SD_D0_PIN, OUTPUT);
    // digitalWrite(SD_D0_PIN, LOW);

}

void loop()
{
    // put your main code here, to run repeatedly:
    static uint32_t Millis;
    delay(2);
    lv_timer_handler();

    unsigned long time = millis();
    if (time - Millis > 50) {
        lv_msg_send(MSG_TIMER_UPDATE, &time);
        Millis = millis();
    }
}

void lcd_send_data(uint8_t data)
{
    uint8_t n;
    for (n = 0; n < 8; n++) {
        if (data & 0x80)
            xl.digitalWrite(LCD_SDA_PIN, 1);
        else
            xl.digitalWrite(LCD_SDA_PIN, 0);

        data <<= 1;
        xl.digitalWrite(LCD_CLK_PIN, 0);
        xl.digitalWrite(LCD_CLK_PIN, 1);
    }
}

void lcd_cmd(const uint8_t cmd)
{
    xl.digitalWrite(LCD_CS_PIN, 0);
    xl.digitalWrite(LCD_SDA_PIN, 0);
    xl.digitalWrite(LCD_CLK_PIN, 0);
    xl.digitalWrite(LCD_CLK_PIN, 1);
    lcd_send_data(cmd);
    xl.digitalWrite(LCD_CS_PIN, 1);
}

void lcd_data(const uint8_t *data, int len)
{
    uint32_t i = 0;
    if (len == 0)
        return; // no need to send anything
    do {
        xl.digitalWrite(LCD_CS_PIN, 0);
        xl.digitalWrite(LCD_SDA_PIN, 1);
        xl.digitalWrite(LCD_CLK_PIN, 0);
        xl.digitalWrite(LCD_CLK_PIN, 1);
        lcd_send_data(*(data + i));
        xl.digitalWrite(LCD_CS_PIN, 1);
        i++;
    } while (len--);
}

void tft_init(void)
{
    xl.digitalWrite(LCD_CS_PIN, 1);
    xl.digitalWrite(LCD_SDA_PIN, 1);
    xl.digitalWrite(LCD_CLK_PIN, 1);

    // Reset the display
    xl.digitalWrite(LCD_RST_PIN, 1);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    xl.digitalWrite(LCD_RST_PIN, 0);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    xl.digitalWrite(LCD_RST_PIN, 1);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    int cmd = 0;
    while (st_init_cmds[cmd].databytes != 0xff) {
        lcd_cmd(st_init_cmds[cmd].cmd);
        lcd_data(st_init_cmds[cmd].data, st_init_cmds[cmd].databytes & 0x1F);
        if (st_init_cmds[cmd].databytes & 0x80) {
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        cmd++;
    }
    Serial.println("Register setup complete");
}

void deep_sleep(void)
{
    // if (pvCreatedTask) {
    //     vTaskDelete(pvCreatedTask);
    // }


    Serial.println("DEEP SLEEP !!!!");

    pinMode(TP_INT_PIN, INPUT);
    waitInterruptReady();

    delay(2000);
    Serial.println("Touch release!!!");

    // If the SD card is initialized, it needs to be unmounted.

    //turn off blacklight
    for (int i = 16; i >= 0; --i) {
        setBrightness(i);
        delay(30);
    }

    Serial.println("Enter deep sleep");
    waitInterruptReady();
    delay(1000);

    // After setting touch to sleep, touch wakeup cannot be used, it needs to be changed to button wakeup, or timing wakeup
    // esp_sleep_enable_ext1_wakeup(1ULL << BOOT_BTN_PIN, ESP_EXT1_WAKEUP_ALL_LOW);

    esp_sleep_enable_ext1_wakeup(1ULL << TP_INT_PIN, ESP_EXT1_WAKEUP_ALL_LOW);
    esp_deep_sleep_start();
}
