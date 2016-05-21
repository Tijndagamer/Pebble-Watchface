#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <pebble.h>
static void battery_state_handler(BatteryChargeState charge);
static void update_time();
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void main_window_load(Window *window);
static void main_window_unload(Window *window);
static void init();
static void deinit();
int main(void);

#endif
