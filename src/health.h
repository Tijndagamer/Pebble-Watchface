#ifndef HEALTH_H_INCLUDED
#define HEALTH_H_INCLUDED

#include <pebble.h>
static void update_step_counter();
static void health_handler(HealthEventType event, void *context);

#endif
