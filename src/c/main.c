#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_batt_layer;

static BitmapLayer *mblyr;
static GBitmap *bmptr;

// Custom fonts
static GFont s_time_font;
static GFont s_date_font;
static GFont status_font;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_time_buffer[8];
  strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ?
                                                    "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_time_buffer);

  // Write the current date into a buffer
  static char s_date_buffer[16];
  strftime(s_date_buffer, sizeof(s_date_buffer), "%a %b %d", tick_time);

  // Display the date
  text_layer_set_text(s_date_layer, s_date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100%";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "+%d%%", charge_state.charge_percent);
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  }
  text_layer_set_text(s_batt_layer, battery_text);
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  //background
  //bmptr = gbitmap_create_with_resource(RESOURCE_ID_WHEEL_NO_CENTER);
  bmptr = gbitmap_create_with_resource(RESOURCE_ID_WHEEL_GREEN_CENTER);
  //bmptr = gbitmap_create_with_resource(RESOURCE_ID_WHEEL);
  mblyr = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(mblyr,bmptr);
  layer_add_child(window_layer, bitmap_layer_get_layer(mblyr));
  
  // Load custom fonts
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_JERSEY_56));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_JERSEY_24));
  // Load system font
  //s_time_font = fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS);
  //s_date_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  status_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD); 

  // Center the time + date block vertically
  int time_height = 56;
  int date_height = 30;
  int block_height = time_height + date_height;
  int time_y = (bounds.size.h / 2) - (block_height / 2) - 10;
  int date_y = time_y + time_height;
  // Battery % below date, but deliberately off center vertically
  int batt_y = date_y + date_height - 5;

  // Create the time TextLayer — centered in the screen
  s_time_layer = text_layer_create(
      GRect(0, time_y, bounds.size.w, 60));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Create the date TextLayer — just below the time
  s_date_layer = text_layer_create(
      GRect(0, date_y, bounds.size.w, 30));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

// Create the battery TextLayer — just below the time
  s_batt_layer = text_layer_create(
      GRect(0, batt_y, bounds.size.w, 26));
  text_layer_set_background_color(s_batt_layer, GColorClear);
  text_layer_set_text_color(s_batt_layer, GColorWhite);
  text_layer_set_font(s_batt_layer, status_font);
  text_layer_set_text_alignment(s_batt_layer, GTextAlignmentCenter);

  // Add layers to the Window
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_batt_layer));

  handle_battery(battery_state_service_peek());
}

static void main_window_unload(Window *window) {
  // Destroy bitmap layers
  gbitmap_destroy(bmptr);
  bitmap_layer_destroy(mblyr);

  // Destroy TextLayers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_batt_layer);

  // Unload custom fonts
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set the background color
  window_set_background_color(s_main_window, GColorBlack);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  battery_state_service_subscribe(handle_battery);

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
