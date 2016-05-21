#include <pebble.h>
#include "health.h"

extern static TextLayer *s_step_count_layer;

static void update_step_counter() {
    HealthMetric metric = HealthMetricStepCount;
    time_t start = time_start_of_today();
    time_t end = time(NULL);

    // Check the metric has data available for today
    HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric,
    start, end);

    if(mask & HealthServiceAccessibilityMaskAvailable) {
        static char steps_count_char[] = "00000000000";
        snprintf(steps_count_char, sizeof(steps_count_char), "%d", (int)health_service_sum_today(metric));
        // Data is available!
        APP_LOG(APP_LOG_LEVEL_INFO, "Steps today: %d", (int)health_service_sum_today(metric));
        APP_LOG(APP_LOG_LEVEL_INFO, steps_count_char);
        text_layer_set_text(s_step_count_layer, steps_count_char);
    } else {
        // No data recorded yet today
        APP_LOG(APP_LOG_LEVEL_ERROR, "Data unavailable!");
    }
}

static void health_handler(HealthEventType event, void *context) {
    switch (event) {
        case HealthEventSignificantUpdate:
            APP_LOG(APP_LOG_LEVEL_INFO, "Significant update");
            update_step_counter();
            break;
        case HealthEventMovementUpdate:
            APP_LOG(APP_LOG_LEVEL_INFO, "movement update");
            update_step_counter();
            break;
        case HealthEventSleepUpdate:
            // This is not important for the step counter, so just ignore it.
            break;
    }
}
