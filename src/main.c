#include <pebble.h>

static Window *main_window;
static TextLayer *time_layer, *date_layer;
static Layer *light_layer, *dark_layer;
static GFont time_font;
static PropertyAnimation *property_animation_light, *property_animation_dark;
static Animation *animation_light, *animation_dark, *layer_slide;

static GColor dark;
static GColor light;
static int day_of_week;
static int color_num;

#define COLOR_KEY 8
#define DAY_KEY 7
  
static void gen_num() {
  color_num = rand() % 8;
}

static void set_color() {
  GColor dark_choices[8] = {GColorCobaltBlue, GColorDarkGreen, GColorDarkCandyAppleRed, GColorOrange, 
                             GColorMidnightGreen, GColorDarkGray, GColorOxfordBlue, GColorIndigo};
  GColor light_choices[8] = {GColorPictonBlue, GColorIslamicGreen, GColorRed, GColorChromeYellow, 
                              GColorTiffanyBlue, GColorLightGray, GColorIndigo, GColorVividViolet};
  
  dark = dark_choices[color_num];
  light = light_choices[color_num];
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(time_layer, buffer);
}

static void update_date() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *date = localtime(&temp);
  
  static char full_date[16];
  day_of_week = date->tm_wday;
  int month = date->tm_mon;
  char day[] = "31";
  
  full_date[0] = '\0';
  
  // Get current date
  strftime(day, sizeof("31"), "%d", date);
  
  switch (day_of_week) {
    case 0:
      strcat(full_date, "SUN, ");
      break;
    case 1:
      strcat(full_date, "MON, ");
      break;
    case 2:
      strcat(full_date, "TUES, ");
      break;
    case 3:
      strcat(full_date, "WED, ");
      break;
    case 4:
      strcat(full_date, "THURS, ");
      break;
    case 5:
      strcat(full_date, "FRI, ");
      break;
    case 6:
      strcat(full_date, "SAT, ");
      break;
  }
  
  switch (month) {
    case 0:
      strcat(full_date, "JAN ");
      break;
    case 1:
      strcat(full_date, "FEB ");
      break;
    case 2:
      strcat(full_date, "MAR ");
      break;
    case 3:
      strcat(full_date, "APR ");
      break;
    case 4:
      strcat(full_date, "MAY ");
      break;
    case 5:
      strcat(full_date, "JUNE ");
      break;
    case 6:
      strcat(full_date, "JULY ");
      break;
    case 7:
      strcat(full_date, "AUG ");
      break;
    case 8:
      strcat(full_date, "SEPT ");
      break;
    case 9:
      strcat(full_date, "OCT ");
      break;
    case 10:
      strcat(full_date, "NOV ");
      break;
    case 11:
      strcat(full_date, "DEC ");
      break;
  }
  
  if (day[0] == '0') {
    strcat(full_date, &day[1]);
  }
  else {
    strcat(full_date, day);
  }

  // Display this date on the TextLayer
  text_layer_set_text(date_layer, full_date);
}

static void update_color() {
  while (color_num == persist_read_int(COLOR_KEY)) {
    // Generates a new color for the sliding boxes
    gen_num();
  }
  
  // Sets colors
  set_color();
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  if (tick_time->tm_wday != day_of_week) {
    update_color();
    update_date();
  }
}

static void draw_light_layer(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Draw a light blue filled rectangle with sharp corners
  graphics_context_set_fill_color(ctx, light);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
}

static void draw_dark_layer(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Draw a dark blue filled rectangle with sharp corners
  graphics_context_set_fill_color(ctx, dark);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
}

static void animations() {
  // Slide from left animation for light background layer
  GRect light_start = GRect(-144, 0, 0, 168);
  GRect light_end = GRect(0, 0, 144, 168);
  property_animation_light = property_animation_create_layer_frame(light_layer, &light_start, &light_end);
  animation_light = property_animation_get_animation(property_animation_light);
  animation_set_duration(animation_light, 800);
  
  // Slide from right animation for dark background layer
  GRect dark_start = GRect(144, 43, 144, 82);
  GRect dark_end = GRect(0, 43, 144, 82);
  property_animation_dark = property_animation_create_layer_frame(dark_layer, &dark_start, &dark_end);
  animation_dark = property_animation_get_animation(property_animation_dark);
  animation_set_duration(animation_dark, 800);
  
  // Create spawn animation for both layers to slide at the same time
  layer_slide = animation_spawn_create(animation_light, animation_dark, NULL);
  animation_set_play_count(layer_slide, 1);
  animation_schedule(layer_slide);
}

static void main_window_load(Window *window) {
  // Get the root layer
  Layer *window_layer = window_get_root_layer(window);
  
  // Create light layer
  light_layer = layer_create(GRectZero);
  layer_set_update_proc(light_layer, draw_light_layer);
  layer_add_child(window_layer, light_layer);
  
  // Create dark layer
  dark_layer = layer_create(GRectZero);
  layer_set_update_proc(dark_layer, draw_dark_layer);
  layer_add_child(window_layer, dark_layer);
  
  // Create time TextLayer
  time_layer = text_layer_create(GRect(0, 42, 144, 70));
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_text_color(time_layer, GColorWhite);

  // Apply to time TextLayer
  time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LATO_BOLD_46));
  text_layer_set_font(time_layer, time_font);
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
  
  // Create date TextLayer
  date_layer = text_layer_create(GRect(0, 88, 144, 80));
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_color(date_layer, GColorWhite);

  // Apply to date TextLayer
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(date_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(time_layer);
  text_layer_destroy(date_layer);
  
  // Destroy colored layers
  layer_destroy(light_layer);
  layer_destroy(dark_layer);
  
  // Destroy animations
  property_animation_destroy(property_animation_dark);
  property_animation_destroy(property_animation_light);
  animation_destroy(animation_light);
  animation_destroy(animation_dark);
  animation_destroy(layer_slide);
}

static void init() {
  srand(time(NULL));
  
  // Create main Window element and assign to pointer
  main_window = window_create();
  window_set_background_color(main_window, GColorBlack);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(main_window, true);
  
  // Make sure the time is displayed from the start
  update_time();
  
  // Make sure the date is displayed from the start
  update_date();
  
  // Make sure the color is displayed from the start
  if (persist_exists(COLOR_KEY) && persist_exists (DAY_KEY) && (persist_read_int(DAY_KEY) == day_of_week)) {
    // Same day, don't generate a new color
    color_num = persist_read_int(COLOR_KEY);
    
    // Sets colors
    set_color();
  }
  else {
    update_color();
  }
  
  // Play animations
  animations();
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  // Stores color and day of week so that the color only changes once a day
  persist_delete(COLOR_KEY);
  persist_delete(DAY_KEY);
  persist_write_int(COLOR_KEY, color_num);
  persist_write_int(DAY_KEY, day_of_week);
  
  // Destroy Window
  window_destroy(main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}