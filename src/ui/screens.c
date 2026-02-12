#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"
//#include "../esp32-bi750.h"
#include "../battery-interface.h"

#include <string.h>

objects_t objects;
lv_obj_t *tick_value_change_obj;

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 170);
    {
        lv_obj_t *parent_obj = obj;
        {
            // percentage_container
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.percentage_container = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 136, 136);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            //lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0); // Transparent
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_GESTURE_BUBBLE);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE); // Disable scrolling

            {
                lv_obj_t *parent_obj = obj;
                {
                    // label_percentage
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.label_percentage = obj;
                    lv_obj_set_pos(obj, 34, 55);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "N/A");
                    lv_obj_set_style_text_font(obj, &ui_font_roboto_28, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    // arc_battery_percentage
                    lv_obj_t *obj = lv_arc_create(parent_obj);
                    objects.arc_battery_percentage = obj;
                    lv_obj_set_pos(obj, 5, 6);
                    lv_obj_set_size(obj, 126, 124);
                    lv_arc_set_value(obj, 90);
                    lv_arc_set_bg_start_angle(obj, 90);
                    lv_arc_set_bg_end_angle(obj, 89);
                    lv_obj_set_style_arc_color(obj, lv_color_hex(0xff45bf12), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_arc_width(obj, 12, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff60f321), LV_PART_KNOB | LV_STATE_DEFAULT);
                }
            }
        }
        {
            // details_container
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.details_container = obj;
            lv_obj_set_pos(obj, 136, 0);
            lv_obj_set_size(obj, 184, 170);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // label_status
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.label_status = obj;
                    lv_obj_set_pos(obj, 35, 16);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "Booting..");
                    lv_obj_set_style_text_font(obj, &ui_font_roboto_28, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    // label_voltage
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.label_voltage = obj;
                    lv_obj_set_pos(obj, 59, 46);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "--.-- V");
                    lv_obj_set_style_text_font(obj, &ui_font_roboto_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    // label_amperage
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.label_amperage = obj;
                    lv_obj_set_pos(obj, 64, 70);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "-.-- A");
                    lv_obj_set_style_text_font(obj, &ui_font_roboto_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    // label_wattage
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.label_wattage = obj;
                    lv_obj_set_pos(obj, 62, 94);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "-.-- W");
                    lv_obj_set_style_text_font(obj, &ui_font_roboto_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    // label_charge_time
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.label_charge_time = obj;
                    lv_obj_set_pos(obj, 11, 120);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "Charge Time: N/A");
                    lv_obj_set_style_text_font(obj, &ui_font_roboto_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    // label_firmware
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.label_firmware = obj;
                    lv_obj_set_pos(obj, 150, 154);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "N/A");
                    lv_obj_set_style_text_font(obj, &ui_font_roboto_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    // label_device_name
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.label_device_name = obj;
                    lv_obj_set_pos(obj, 20, 154);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "N/A");
                    lv_obj_set_style_text_font(obj, &ui_font_roboto_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    // label_mah
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.label_mah = obj;
                    lv_obj_set_pos(obj, 72, 154);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "N/A");
                    lv_obj_set_style_text_font(obj, &ui_font_roboto_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            // temperature_container
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.temperature_container = obj;
            lv_obj_set_pos(obj, 0, 136);
            lv_obj_set_size(obj, 136, 34);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // label_temperature
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.label_temperature = obj;
                    lv_obj_set_pos(obj, 44, 6);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "16°C");
                    lv_obj_set_style_text_font(obj, &ui_font_roboto_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
    }
    lv_scr_load(objects.main);
    tick_screen_main();
}

void tick_screen_main() {
    lv_label_set_text(objects.label_device_name, batteryData.deviceName);
    lv_label_set_text_fmt(objects.label_firmware, "v%s", batteryData.fwVer);
    lv_label_set_text(objects.label_status, (batteryData.chargeMode == 1) ? "Charging" : "Discharging");
    if (batteryData.chargeMode == 1) {
        // if charging, set the input current values
        lv_label_set_text_fmt(objects.label_amperage, "%.2f A", batteryData.inA);
        lv_label_set_text_fmt(objects.label_voltage, "%.2f V", batteryData.inV);
        lv_label_set_text_fmt(objects.label_wattage, "%.2f W", batteryData.inA * batteryData.inV);
        lv_label_set_text_fmt(objects.label_charge_time, "Charge Time: %s", batteryData.chargeMinutes);
        lv_obj_align(objects.label_status, LV_ALIGN_CENTER, 0, -54);
        lv_obj_align(objects.label_charge_time, LV_ALIGN_CENTER, 0, 44);
    } else {
        // if discharging, out the output current values
        lv_label_set_text_fmt(objects.label_amperage, "%.2f A", batteryData.outA);
        lv_label_set_text_fmt(objects.label_voltage, "%.2f V", batteryData.outV);
        lv_label_set_text_fmt(objects.label_wattage, "%.2f W", batteryData.outA * batteryData.outV);
        lv_label_set_text_fmt(objects.label_charge_time, "Batt Time: %s", batteryData.dischargeMinutes);
        lv_obj_align(objects.label_status, LV_ALIGN_CENTER, 0, -54);
        lv_obj_align(objects.label_charge_time, LV_ALIGN_CENTER, 0, 44);
    }
    
    lv_label_set_text_fmt(objects.label_temperature, "%d °C", batteryData.temperature);
    lv_label_set_text_fmt(objects.label_percentage, "%d%%", batteryData.percentage);
    lv_obj_align(objects.label_percentage, LV_ALIGN_CENTER, 0, 0);
    lv_arc_set_value(objects.arc_battery_percentage, batteryData.percentage);
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

void create_screens() {
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_main();
}
