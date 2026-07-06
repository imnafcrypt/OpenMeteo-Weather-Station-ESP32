#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations

// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_WIND_SPEED = 0,
    FLOW_GLOBAL_VARIABLE_HUMIDITY = 1,
    FLOW_GLOBAL_VARIABLE_TEMP = 2,
    FLOW_GLOBAL_VARIABLE_CONDITION = 3,
    FLOW_GLOBAL_VARIABLE_NETWORK = 4,
    FLOW_GLOBAL_VARIABLE_CITY = 5
};

// Native global variables

extern float get_var_wind_speed();
extern void set_var_wind_speed(float value);
extern int32_t get_var_humidity();
extern void set_var_humidity(int32_t value);
extern float get_var_temp();
extern void set_var_temp(float value);
extern const char *get_var_condition();
extern void set_var_condition(const char *value);
extern bool get_var_network();
extern void set_var_network(bool value);
extern const char *get_var_city();
extern void set_var_city(const char *value);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/