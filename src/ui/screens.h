#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *percentage_container;
    lv_obj_t *label_percentage;
    lv_obj_t *arc_battery_percentage;
    lv_obj_t *details_container;
    lv_obj_t *label_status;
    lv_obj_t *label_voltage;
    lv_obj_t *label_amperage;
    lv_obj_t *label_wattage;
    lv_obj_t *label_charge_time;
    lv_obj_t *label_firmware;
    lv_obj_t *label_device_name;
    lv_obj_t *label_mah;
    lv_obj_t *temperature_container;
    lv_obj_t *label_temperature;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
};

void create_screen_main();
void tick_screen_main();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/