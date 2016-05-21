#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_battery_info_layer;
static TextLayer *s_step_count_layer;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

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

static void battery_state_handler(BatteryChargeState charge) {
    // Convert to char
    static char charge_percent_char[] = "00000000000";
    snprintf(charge_percent_char, sizeof(charge_percent_char), "%d", charge.charge_percent);

    // Display to screen
    text_layer_set_text(s_battery_info_layer, charge_percent_char);
}

static void update_time() {
    // Get a tm struct
    time_t temp = time(NULL);

    // Adjust the struct to the local time format
    struct tm *tick_time = localtime(&temp);

    // Write current hours and minutes into a buffer
    static char s_buffer[8];
    // Format the time. Fuck 12-hour format.
    strftime(s_buffer, sizeof(s_buffer), "%H:%M", tick_time);

    // Display the text in the TextLayer
    text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();
}

static void main_window_load(Window *window) {
    // Get information about the window needed for drawing text 'n shit
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    /* Bitmap layer for displaying the background */

    // Create GBitmap
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);

    // Create BitmapLayer to display the GBitmap
    s_background_layer = bitmap_layer_create(bounds);

    // Add bitmap to the layer and add the layer to the window as a child
    bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));

    /* Text layer for displaying the time */

    // Create TextLayer
    s_time_layer = text_layer_create(
        GRect(0, PBL_IF_ROUND_ELSE(58, 54), bounds.size.w, 55));

    // Set layout options
    text_layer_set_background_color(s_time_layer, GColorRed);
    text_layer_set_text_color(s_time_layer, GColorYellow);
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

    // Add child
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

    /* Text Layer for displaying battery info */

    // Create TextLayer
    s_battery_info_layer = text_layer_create(GRect(-10, 10, bounds.size.w, 20));

    // Set layout options
    text_layer_set_background_color(s_battery_info_layer, GColorRed);
    text_layer_set_text_color(s_battery_info_layer, GColorYellow);
    text_layer_set_font(s_battery_info_layer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));
    text_layer_set_text_alignment(s_battery_info_layer, GTextAlignmentRight);

    // Add child
    layer_add_child(window_layer, text_layer_get_layer(s_battery_info_layer));

    // Get first value
    BatteryChargeState charge = battery_state_service_peek();
    static char charge_percent_char[] = "00000000000";
    snprintf(charge_percent_char, sizeof(charge_percent_char), "%d", charge.charge_percent);
    text_layer_set_text(s_battery_info_layer, charge_percent_char);

    /* Text Layer for displaying steps counted by Pebble Health */

    // Create TextLayer
    s_step_count_layer = text_layer_create(GRect(-10, 30, bounds.size.w, 20));

    // Set layout options
    text_layer_set_background_color(s_step_count_layer, GColorRed);
    text_layer_set_text_color(s_step_count_layer, GColorYellow);
    text_layer_set_font(s_step_count_layer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));
    text_layer_set_text_alignment(s_step_count_layer, GTextAlignmentRight);

    // Add child
    layer_add_child(window_layer, text_layer_get_layer(s_step_count_layer));

    // Get first value
    update_step_counter();
}

static void main_window_unload(Window *window) {
    text_layer_destroy(s_time_layer);
    gbitmap_destroy(s_background_bitmap);
    bitmap_layer_destroy(s_background_layer);
}

static void init() {
    // Create a new window and assign to pointer
    s_main_window = window_create();

    // Set handlers to handle everything inside the window_set
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });

    // Show Window on the watch, animated = true
    window_stack_push(s_main_window, true);

    // Register TickTimerService
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

    // Make sure the time is correct when first starting the watchface
    update_time();

    // Subscribe to battery state updates
    battery_state_service_subscribe(battery_state_handler);

    // Subscribe to health updates
    health_service_events_subscribe(health_handler, NULL);
}

static void deinit() {
    // Destroy Window
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
