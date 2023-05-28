#include "HTTPClient.h"
#include "SD_MMC.h"
#include "WiFi.h"
#include "Wire.h"
#include "XL9535_driver.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_vendor.h"
#include "factory_ui.h"
#include "img.h"
#include "lvgl.h"
#include "pin_config.h"
#include <Arduino.h>
#include "OneButton.h"

// #define USING_2_1_INC_CST820     1           //  Full circle 2.1 inches using CST820 touch screen
// #define USING_2_8_INC_GT911      1           //  Full circle 2.8 inches using GT911 touch screen
// #define USING_2_1_INC_FT3267     1           //  Half circle 2.1 inches use FT3267 touch screen


#if defined(USING_2_1_INC_FT3267)
#include "ft3267.h"
#endif
#if defined(USING_2_1_INC_CST820)
#define TOUCH_MODULES_CST_SELF
#include "TouchLib.h"
TouchLib touch(Wire, IIC_SDA_PIN, IIC_SCL_PIN, CTS820_SLAVE_ADDRESS);
#elif defined(USING_2_8_INC_GT911)
#define TOUCH_MODULES_GT911
#include "TouchLib.h"
TouchLib touch(Wire, IIC_SDA_PIN, IIC_SCL_PIN, GT911_SLAVE_ADDRESS1);
#endif



#if !defined(USING_2_1_INC_CST820) && !defined(USING_2_8_INC_GT911) && !defined(USING_2_1_INC_FT3267)
#error "Please define the size of the screen and open the macro definition at the top of the sketch"
#endif

typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; // No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

#if defined(USING_2_1_INC_CST820) || defined(USING_2_1_INC_FT3267)
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
#elif defined(USING_2_8_INC_GT911)
DRAM_ATTR static const lcd_init_cmd_t st_init_cmds[] = {
    {0xFF, {0x77, 0x01, 0x00, 0x00, 0x13}, 0x05},
    {0xEF, {0x08}, 0x01},
    {0xFF, {0x77, 0x01, 0x00, 0x00, 0x10}, 0x05},
    {0xC0, {0x3B, 0X00}, 0x02},
    {0xC1, {0x10, 0x0C}, 0x02},
    {0xC2, {0x07, 0x0A}, 0x02},
    {0xC7, {0x00}, 0x01},
    {0xCC, {0x10}, 0x01},
    {0xCD, {0x08}, 0x01}, // 用565时屏蔽    666打开
    {0xb0, {0x05, 0x12, 0x98, 0x0e, 0x0F, 0x07, 0x07, 0x09, 0x09, 0x23, 0x05, 0x52, 0x0F, 0x67, 0x2C, 0x11}, 0x10},
    {0xb1, {0x0B, 0x11, 0x97, 0x0C, 0x12, 0x06, 0x06, 0x08, 0x08, 0x22, 0x03, 0x51, 0x11, 0x66, 0x2B, 0x0F}, 0x10},
    {0xFF, {0x77, 0x01, 0x00, 0x00, 0x11}, 0x05},
    {0xb0, {0x5d}, 0x01},
    {0xb1, {0x2D}, 0x01},
    {0xb2, {0x81}, 0x01},
    {0xb3, {0x80}, 0x01},
    {0xb5, {0x4E}, 0x01},
    {0xb7, {0x85}, 0x01},
    {0xb8, {0x20}, 0x01},
    {0xc1, {0x78}, 0x01},
    {0xc2, {0x78}, 0x01},
    // {0xc3, {0x8c}, 0x01},
    {0xd0, {0x88}, 0x01},
    {0xe0, {0x00, 0x00, 0x02}, 0x03},
    {0xe1, {0x06, 0x30, 0x08, 0x30, 0x05, 0x30, 0x07, 0x30, 0x00, 0x33, 0x33}, 0x0b},
    {0xe2, {0x11, 0x11, 0x33, 0x33, 0xf4, 0x00, 0x00, 0x00, 0xf4, 0x00, 0x00, 0x00}, 0x0c},
    {0xe3, {0x00, 0x00, 0x11, 0x11}, 0x04},
    {0xe4, {0x44, 0x44}, 0x02},
    {0xe5, {0x0d, 0xf5, 0x30, 0xf0, 0x0f, 0xf7, 0x30, 0xf0, 0x09, 0xf1, 0x30, 0xf0, 0x0b, 0xf3, 0x30, 0xf0}, 0x10},
    {0xe6, {0x00, 0x00, 0x11, 0x11}, 0x04},
    {0xe7, {0x44, 0x44}, 0x02},
    {0xe8, {0x0c, 0xf4, 0x30, 0xf0, 0x0e, 0xf6, 0x30, 0xf0, 0x08, 0xf0, 0x30, 0xf0, 0x0a, 0xf2, 0x30, 0xf0}, 0x10},
    {0xe9, {0x36}, 0x01},
    {0xeb, {0x00, 0x01, 0xe4, 0xe4, 0x44, 0x88, 0x40}, 0x07},
    {0xed, {0xff, 0x10, 0xaf, 0x76, 0x54, 0x2b, 0xcf, 0xff, 0xff, 0xfc, 0xb2, 0x45, 0x67, 0xfa, 0x01, 0xff}, 0x10},
    {0xef, {0x08, 0x08, 0x08, 0x45, 0x3f, 0x54}, 0x06},
    {0xFF, {0x77, 0x01, 0x00, 0x00, 0x00}, 0x05},

    {0x11, {0x00}, 0x80},
    {0x3a, {0x66}, 0x01},
    {0x36, {0x08}, 0x01},
    {0x35, {0x00}, 0x01},
    {0x29, {0x00}, 0x80},
    {0, {0}, 0xff}
};
#endif


XL9535 xl;
OneButton button(0, true);

const int backlightPin = EXAMPLE_PIN_NUM_BK_LIGHT;
bool click = false;
TaskHandle_t pvCreatedTask;


void deep_sleep(void);
void SD_init(void);
void tft_init(void);
void lcd_cmd(const uint8_t cmd);
void lcd_data(const uint8_t *data, int len);
void wifi_task(void *param);
bool touchDevicesOnline = false;
uint8_t touchAddress = 0;

const char *getTouchAddr()
{
    if (touchAddress == FT5x06_ADDR) {
        return "FT3267";
    } else if (touchAddress == CST820_ADDR) {
        return "CST820";
    } else if (touchAddress == GT911_ADDR) {
        return "GT911";
    }
#ifdef USING_2_1_INC_CST820
    return "CST820";
#else
    return "UNKONW";
#endif
}

void scanDevices(void)
{
    byte error, address;
    int nDevices = 0;
    Serial.println("Scanning for I2C devices ...");
    for (address = 0x01; address < 0x7f; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        if (error == 0) {
            Serial.printf("I2C device found at address 0x%02X\n", address);
            if (address == FT5x06_ADDR) {
                Serial.println("Find FT5X06 touch device!"); touchDevicesOnline = true; touchAddress = FT5x06_ADDR;
            } else if (address == CST820_ADDR) {
                Serial.println("Find CST820 touch device!"); touchDevicesOnline = true; touchAddress = CST820_ADDR;
            } else if (address == GT911_ADDR) {
                Serial.println("Find GT911 touch device!"); touchDevicesOnline = true; touchAddress = GT911_ADDR;
            }
            nDevices++;
        } else if (error != 2) {
            Serial.printf("Error %d at address 0x%02X\n", error, address);
        }
    }
    if (nDevices == 0) {
        Serial.println("No I2C devices found");
    }
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
#if defined(USING_2_1_INC_FT3267)

    uint8_t touch_points_num;
    ft3267_read_pos(&touch_points_num, &p.x, &p.y);
    data->point.x = p.x;
    data->point.y = p.y;
    if (p.x != lastX  || p.y != lastY) {
        lastX = p.x;
        lastY = p.y;
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }

#elif defined(USING_2_1_INC_CST820) || defined(USING_2_8_INC_GT911)
    if (touch.read()) {
        TP_Point t = touch.getPoint(0);
        data->point.x = p.x = t.x;
        data->point.y = p.y = t.y;
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
#endif
    lv_msg_send(MSG_TOUCH_UPDATE, &p);
}

void waitInterruptReady()
{
    Serial.println("waitInterruptReady ...");

    uint32_t timeout = millis() + 500;

// The touch interrupt of CST820 is a high and low pulse, it is not fixed, there will be a low level every ~10 milliseconds interval
#ifdef USING_2_1_INC_CST820
    while (timeout > millis()) {
        while (!digitalRead(TP_INT_PIN)) {
            delay(20);
            timeout = millis() + 500;
        }
    }
#else
    //Wait for the GT911 interrupt signal to be ready
    while (!digitalRead(TP_INT_PIN)) {
#if defined(USING_2_8_INC_GT911)
        if (timeout <  millis()) {
            Serial.println("timeout !");
            esp_restart();
        }
        touch.read();
#endif
        delay(10);
    }

#endif

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

    Serial.begin(115200);

    // put your setup code here, to run once:
    pinMode(BAT_VOLT_PIN, ANALOG);

    Wire.begin(IIC_SDA_PIN, IIC_SCL_PIN);

    xl.begin();

    uint8_t pin = (1 << PWR_EN_PIN) | (1 << LCD_CS_PIN) | (1 << TP_RES_PIN) | (1 << LCD_SDA_PIN) | (1 << LCD_CLK_PIN) |
                  (1 << LCD_RST_PIN) | (1 << SD_CS_PIN);

    xl.pinMode8(0, pin, OUTPUT);
    xl.digitalWrite(PWR_EN_PIN, HIGH);

    print_chip_info();

    SD_init();

#ifdef USING_2_8_INC_GT911
    //Reset GT911
    xl.pinMode(TP_RES_PIN, OUTPUT);
    pinMode(TP_INT_PIN, OUTPUT);
    digitalWrite(TP_INT_PIN, LOW);
    xl.digitalWrite(TP_RES_PIN, LOW);
    delayMicroseconds(120);
    xl.digitalWrite(TP_RES_PIN, HIGH);
    delay(8);
    pinMode(TP_INT_PIN, INPUT);

    waitInterruptReady();
#else
    delay(100);
    xl.digitalWrite(TP_RES_PIN, LOW);
    delay(300);
    xl.digitalWrite(TP_RES_PIN, HIGH);
    delay(300);
    pinMode(TP_INT_PIN, INPUT);
#endif

    // Scanning I2C cannot get the device address of CST820, it is a non-standard I2C device
    scanDevices();

#if defined(USING_2_1_INC_FT3267)
    ft3267_init(Wire);
#elif defined(USING_2_8_INC_GT911) || defined(USING_2_1_INC_CST820)
    touch.init();
#endif

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



    //Test screen color
    LV_IMG_DECLARE(photo2);
    LV_IMG_DECLARE(photo4);
    LV_IMG_DECLARE(photo5);

    lv_obj_t *img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &photo2);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

    const lv_img_dsc_t *photo[] = { &photo2, &photo4, &photo5};

    button.attachClick([]() {
        click = true;
    });

    //Wait for the GT911 interrupt signal to be ready
    waitInterruptReady();

    lv_task_handler();


    pinMode(backlightPin, OUTPUT);
    //LilyGo T-RGB control backlight chip has 16 levels of adjustment range
    for (int i = 0; i < 16; ++i) {
        setBrightness(i);
        delay(30);
    }


    int i = 1;
    while (i <= 3) {
        if (click || !digitalRead(TP_INT_PIN)) {
            click = false;
            lv_img_set_src(img, photo[i]);
            i++;
            waitInterruptReady();
        }

        button.tick();
        lv_task_handler();
        delay(5);
    }

    lv_obj_del(img);

    ui_begin();

    xTaskCreate(wifi_task, "wifi_task", 1024 * 6, NULL, 1, &pvCreatedTask);
}

bool lastStatus = false;

void loop()
{
    // put your main code here, to run repeatedly:
    static uint32_t Millis;
    delay(2);
    lv_timer_handler();
    if (millis() - Millis > 50) {
        float v = (analogRead(BAT_VOLT_PIN) * 2 * 3.3) / 4096;
        lv_msg_send(MSG_BAT_VOLT_UPDATE, &v);
        Millis = millis();
    }

#ifndef USING_2_1_INC_CST820
    bool touched = digitalRead(TP_INT_PIN) == LOW;
    if (touched) {
        lastStatus = touched;
        lv_msg_send(MSG_TOUCH_INT_UPDATE, &touched);
    } else if (!touched && lastStatus) {
        lastStatus = false;
        lv_msg_send(MSG_TOUCH_INT_UPDATE, &touched);
    }
#endif
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

void SD_init(void)
{
    xl.digitalWrite(SD_CS_PIN, 1); // To use SDIO one-line mode, you need to pull the CS pin high
    SD_MMC.setPins(SD_CLK_PIN, SD_CMD_PIN, SD_D0_PIN);
    if (!SD_MMC.begin("/sdcard", true, true)) {
        Serial.println("Card Mount Failed");
        return;
    }

    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");

    if (cardType == CARD_MMC)
        Serial.println("MMC");
    else if (cardType == CARD_SD)
        Serial.println("SDSC");
    else if (cardType == CARD_SDHC)
        Serial.println("SDHC");
    else
        Serial.println("UNKNOWN");

    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

// This task is used to test WIFI, http test
void wifi_task(void *param)
{
    String str;
    HTTPClient http_client;
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    delay(100);
    Serial.println("scan start");
    str = "wifi scan start";
    lv_msg_send(MSG_WIFI_UPDATE, str.c_str());
    delay(1000);
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    str = "scan done\r\n";
    lv_msg_send(MSG_WIFI_UPDATE, str.c_str());
    if (n == 0) {
        Serial.println("no networks found");
        str = "no networks found";
        lv_msg_send(MSG_WIFI_UPDATE, str.c_str());
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        str += n;
        str += " networks found\r\n";
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");

            str += i + 1;
            str += ": ";
            str += WiFi.SSID(i);
            str += " (";
            str += WiFi.RSSI(i);
            str += ")";
            str += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*";
            str += "\r\n";
        }
    }
    str += "\r\n";
    lv_msg_send(MSG_WIFI_UPDATE, str.c_str());
    Serial.println("");
    WiFi.disconnect();

    delay(3000);
    uint32_t last_m = millis();
    str = "connecting to wifi\r\n";
    str += "SSID : " WIFI_SSID "\r\n";
    str += "PASSWORD : " WIFI_PASSWORD "\r\n";
    lv_msg_send(MSG_WIFI_UPDATE, str.c_str());

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    uint32_t timestamp = millis() + 30000;
    while (WiFi.status() != WL_CONNECTED) {
        if (timestamp < millis()) {
            Serial.println("WiFi Connect failed!");
            lv_msg_send(MSG_WIFI_UPDATE, "WiFi Connect failed!");
            pvCreatedTask = NULL;
            vTaskDelete(NULL);
            while (1) {
                delay(30000);
            }
        }
        Serial.print(".");
        vTaskDelay(500);
        str += ".";
        lv_msg_send(MSG_WIFI_UPDATE, str.c_str());
    }
    Serial.printf("\r\n-- wifi connect success! --\r\n");
    Serial.printf("It takes %d milliseconds\r\n", millis() - last_m);

    str += "\r\n-- wifi connect success! --\r\n";
    str += "It takes ";
    str += millis() - last_m;
    str += "milliseconds\r\n";
    lv_msg_send(MSG_WIFI_UPDATE, str.c_str());

    delay(2000);
    String rsp;
    bool is_get_http = false;
    do {
        http_client.begin("https://www.arduino.cc/");
        str = "getting https://www.arduino.cc/";
        lv_msg_send(MSG_WIFI_UPDATE, str.c_str());

        int http_code = http_client.GET();
        Serial.println(http_code);
        if (http_code > 0) {
            Serial.printf("HTTP get code: %d\n", http_code);
            if (http_code == HTTP_CODE_OK) {
                rsp = http_client.getString();
                Serial.println(rsp);
                is_get_http = true;
                str += rsp;
                lv_msg_send(MSG_WIFI_UPDATE, str.c_str());

            } else {
                Serial.printf("fail to get http client,code:%d\n", http_code);
            }
        } else {
            Serial.println("HTTP GET failed. Try again");
            str = "HTTP GET failed. Try again";
            lv_msg_send(MSG_WIFI_UPDATE, str.c_str());
        }
        delay(3000);
    } while (!is_get_http);
    WiFi.disconnect();
    http_client.end();

    str = "#00ff00 WIFI detection function completed #";
    lv_msg_send(MSG_WIFI_UPDATE, str.c_str());

    pvCreatedTask = NULL;
    vTaskDelete(NULL);
}

void deep_sleep(void)
{
    if (pvCreatedTask) {
        vTaskDelete(pvCreatedTask);
    }

    WiFi.disconnect();

    Serial.println("DEEP SLEEP !!!!");


#ifdef USING_2_8_INC_GT911
    // After setting touch to sleep, touch wakeup cannot be used, it needs to be changed to button wakeup, or timing wakeup
    // pinMode(TP_INT_PIN, OUTPUT);
    // digitalWrite(TP_INT_PIN, LOW); //Before touch to set sleep, it is necessary to set INT to LOW
    // touch.enableSleep();
#endif

    pinMode(TP_INT_PIN, INPUT);
    waitInterruptReady();

    delay(2000);
    Serial.println("Touch release!!!");

    // If the SD card is initialized, it needs to be unmounted.
    if (SD_MMC.cardSize())
        SD_MMC.end();

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
