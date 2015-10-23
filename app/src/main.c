#include <pebble.h>
#include "main.h"

static Window *window;
static Layer *s_simple_bg_layer, *s_hands_layer;

static BitmapLayer *clock_layer;
static GBitmap *clock_bitmap;

static GPath *s_second_arrow;

static void ftoa(char* str, double val, int precision) {
  //  start with positive/negative
  if (val < 0) {
    *(str++) = '-';
    val = -val;
  }
  //  integer value
  snprintf(str, 12, "%d", (int) val);
  str += strlen(str);
  val -= (int) val;
  //  decimals
  if ((precision > 0) && (val >= .00001)) {
    //  add period
    *(str++) = '.';
    //  loop through precision
    for (int i = 0;  i < precision;  i++)
      if (val > 0) {
        val *= 10;
        *(str++) = '0' + (int) (val + ((i == precision - 1) ? .5 : 0));
        val -= (int) val;
      } else
        break;
  }
  //  terminate
  *str = '\0';
}

static void hands_update_proc(Layer *layer, GContext *ctx) {

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);

  // SECOND HAND //

  float percentage = 1-((86400.0-((t->tm_hour * 60.0 * 60.0) + (t->tm_min * 60.0) + t->tm_sec))/86400.0);
  char buf[] = "";

  float angle;

    angle = ((TRIG_MAX_ANGLE * percentage) - (TRIG_MAX_ANGLE/2.0));

  if(angle < 0){
    //angle = ((360 + (TRIG_MAX_ANGLE / 360.0)) * percentage - 180) * 360.0;
    angle = angle+TRIG_MAX_ANGLE;
  }

  ftoa(buf,((TRIG_MAX_ANGLE / 360.0) * percentage),5);

  app_log(1,"main.c",69,"angle %s",buf);

    //angle = 127*360;

  gpath_rotate_to(s_second_arrow, angle);
  #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_context_set_stroke_color(ctx, GColorWhite);
  #else
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_context_set_stroke_color(ctx, GColorWhite);
  #endif
  gpath_draw_filled(ctx, s_second_arrow);
  gpath_draw_outline(ctx, s_second_arrow);

  #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorRed);
    graphics_context_set_stroke_color(ctx, GColorRed);
  #else
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_context_set_stroke_color(ctx, GColorBlack);
  #endif

  //CENTER CIRCLES
  GPoint center_circle = {
    .x = center.x,
    .y = center.y,
  };

  #ifdef PBL_COLOR
    graphics_context_set_stroke_color(ctx, GColorLightGray);
    graphics_context_set_fill_color(ctx, GColorLightGray);
  #else
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_fill_color(ctx, GColorWhite);
  #endif

  graphics_draw_circle(ctx, center_circle, 6);
  graphics_fill_circle(ctx, center_circle, 6);


    GPoint center_circle2 = {
    .x = center.x,
    .y = center.y,
  };

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);
    
  graphics_draw_circle(ctx, center_circle2, 5);
  graphics_fill_circle(ctx, center_circle2, 5);

}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(window));
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  //ACTION: Create GBitmap, then set to created BitmapLayer
  clock_bitmap = gbitmap_create_with_resource(RESOURCE_ID_bg_image);
  clock_layer = bitmap_layer_create(GRect(0, 0, 180, 180));
  bitmap_layer_set_bitmap(clock_layer, clock_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(clock_layer));

  s_hands_layer = layer_create(bounds);
  layer_set_update_proc(s_hands_layer, hands_update_proc);
  layer_add_child(window_layer, s_hands_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_simple_bg_layer);
  layer_destroy(s_hands_layer);
  gbitmap_destroy(clock_bitmap);
  bitmap_layer_destroy(clock_layer);
}

static void init() {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);

  // init hand paths
  s_second_arrow = gpath_create(&SECOND_HAND_POINTS);
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GPoint center = grect_center_point(&bounds);
  gpath_move_to(s_second_arrow, center);

  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
}

static void deinit() {
  gpath_destroy(s_second_arrow);
  tick_timer_service_unsubscribe();
  window_destroy(window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}