#include "pebble.h"

static Window *window;
static Layer *window_layer;

GBitmap *img_battery_100;
GBitmap *img_battery_90;
GBitmap *img_battery_80;
GBitmap *img_battery_70;
GBitmap *img_battery_60;
GBitmap *img_battery_50;
GBitmap *img_battery_40;
GBitmap *img_battery_30;
GBitmap *img_battery_20;
GBitmap *img_battery_10;
GBitmap *img_battery_charge;
BitmapLayer *layer_batt_img;

const int MONTH_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_MONTH_JAN,
  RESOURCE_ID_IMAGE_MONTH_FEB,
  RESOURCE_ID_IMAGE_MONTH_MAR,
  RESOURCE_ID_IMAGE_MONTH_APR,
  RESOURCE_ID_IMAGE_MONTH_MAY,
  RESOURCE_ID_IMAGE_MONTH_JUN,
  RESOURCE_ID_IMAGE_MONTH_JUL,
  RESOURCE_ID_IMAGE_MONTH_AUG,
  RESOURCE_ID_IMAGE_MONTH_SEP,
  RESOURCE_ID_IMAGE_MONTH_OCT,
  RESOURCE_ID_IMAGE_MONTH_NOV,
  RESOURCE_ID_IMAGE_MONTH_DEC
};

const int DAY_NAME_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_DAY_NAME_SUN,
  RESOURCE_ID_IMAGE_DAY_NAME_MON,
  RESOURCE_ID_IMAGE_DAY_NAME_TUE,
  RESOURCE_ID_IMAGE_DAY_NAME_WED,
  RESOURCE_ID_IMAGE_DAY_NAME_THU,
  RESOURCE_ID_IMAGE_DAY_NAME_FRI,
  RESOURCE_ID_IMAGE_DAY_NAME_SAT
};

static GBitmap *day_name_image;
static BitmapLayer *day_name_layer;

static GBitmap *month_image;
static BitmapLayer *month_layer;

#define TOTAL_DATE_DIGITS 2
static GBitmap *date_digits_images[TOTAL_DATE_DIGITS];
static BitmapLayer *date_digits_layers[TOTAL_DATE_DIGITS];

const int DATENUM_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_DATENUM_0,
  RESOURCE_ID_IMAGE_DATENUM_1,
  RESOURCE_ID_IMAGE_DATENUM_2,
  RESOURCE_ID_IMAGE_DATENUM_3,
  RESOURCE_ID_IMAGE_DATENUM_4,
  RESOURCE_ID_IMAGE_DATENUM_5,
  RESOURCE_ID_IMAGE_DATENUM_6,
  RESOURCE_ID_IMAGE_DATENUM_7,
  RESOURCE_ID_IMAGE_DATENUM_8,
  RESOURCE_ID_IMAGE_DATENUM_9
};

#define TOTAL_TIME_DIGITS 4
static GBitmap *time_digits_images[TOTAL_TIME_DIGITS];
static BitmapLayer *time_digits_layers[TOTAL_TIME_DIGITS];

const int BIG_DIGIT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUM_0,
  RESOURCE_ID_IMAGE_NUM_1,
  RESOURCE_ID_IMAGE_NUM_2,
  RESOURCE_ID_IMAGE_NUM_3,
  RESOURCE_ID_IMAGE_NUM_4,
  RESOURCE_ID_IMAGE_NUM_5,
  RESOURCE_ID_IMAGE_NUM_6,
  RESOURCE_ID_IMAGE_NUM_7,
  RESOURCE_ID_IMAGE_NUM_8,
  RESOURCE_ID_IMAGE_NUM_9
};

int charge_percent = 0;
TextLayer *battery_text_layer;

Layer *minute_display_layer;

static GBitmap *bluetooth_image;
static BitmapLayer *bluetooth_layer;

static GFont *custom_font;


const GPathInfo MINUTE_SEGMENT_PATH_POINTS = {
  3,
  (GPoint []) {
    {0, 0},
    {-8, -72}, // 70 = radius + fudge; 7 = 70*tan(6 degrees); 6 degrees per minute;
    {8,  -72},
  }
};

static GPath *minute_segment_path;

static void minute_display_layer_update_callback(Layer *layer, GContext* ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  unsigned int angle = t->tm_min * 6;
  gpath_rotate_to(minute_segment_path, (TRIG_MAX_ANGLE / 360) * angle);

  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
#ifdef PBL_PLATFORM_BASALT
	  graphics_context_set_fill_color(ctx, GColorDarkGray);
#else
	  graphics_context_set_fill_color(ctx, GColorBlack);
#endif  
	graphics_fill_circle(ctx, center, 70);

  graphics_context_set_fill_color(ctx, GColorWhite);
  gpath_draw_filled(ctx, minute_segment_path);
  graphics_fill_circle(ctx, center, 66);
}

	
static void set_container_image(GBitmap **bmp_image, BitmapLayer *bmp_layer, const int resource_id, GPoint origin) {
  GBitmap *old_image = *bmp_image;

  *bmp_image = gbitmap_create_with_resource(resource_id);
#ifdef PBL_PLATFORM_BASALT
  GRect bitmap_bounds = gbitmap_get_bounds((*bmp_image));
#else
  GRect bitmap_bounds = (*bmp_image)->bounds;
#endif
  GRect frame = GRect(origin.x, origin.y, bitmap_bounds.size.w, bitmap_bounds.size.h);
  bitmap_layer_set_bitmap(bmp_layer, *bmp_image);
  layer_set_frame(bitmap_layer_get_layer(bmp_layer), frame);

  if (old_image != NULL) {
  	gbitmap_destroy(old_image);
  }
}

void update_battery(BatteryChargeState charge_state) {
    static char battery_text[] = "+100";

    if (charge_state.is_charging) {
        bitmap_layer_set_bitmap(layer_batt_img, img_battery_charge);
		text_layer_set_text_color(battery_text_layer, GColorBlack);
		snprintf(battery_text, sizeof(battery_text), "+%d", charge_state.charge_percent);

    } else {
        snprintf(battery_text, sizeof(battery_text), "%d", charge_state.charge_percent);       
	
        if (charge_state.charge_percent <= 10) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_10);
#ifdef PBL_PLATFORM_BASALT
text_layer_set_text_color(battery_text_layer, GColorWhite);
			#else
text_layer_set_text_color(battery_text_layer, GColorBlack);
#endif			  

        } else if (charge_state.charge_percent <= 20) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_20);
#ifdef PBL_PLATFORM_BASALT
text_layer_set_text_color(battery_text_layer, GColorWhite);
			#else
text_layer_set_text_color(battery_text_layer, GColorBlack);
#endif			  

		} else if (charge_state.charge_percent <= 30) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_30);
#ifdef PBL_PLATFORM_BASALT
text_layer_set_text_color(battery_text_layer, GColorWhite);
			#else
text_layer_set_text_color(battery_text_layer, GColorBlack);
#endif			  

		} else if (charge_state.charge_percent <= 40) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_40);
			  text_layer_set_text_color(battery_text_layer, GColorBlack);

		} else if (charge_state.charge_percent <= 50) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_50);
			  text_layer_set_text_color(battery_text_layer, GColorBlack);

        } else if (charge_state.charge_percent <= 60) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_60);
			  text_layer_set_text_color(battery_text_layer, GColorBlack);

		} else if (charge_state.charge_percent <= 70) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_70);
			  text_layer_set_text_color(battery_text_layer, GColorBlack);

		} else if (charge_state.charge_percent <= 80) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_80);
			  text_layer_set_text_color(battery_text_layer, GColorBlack);

		} else if (charge_state.charge_percent <= 90) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_90);
#ifdef PBL_PLATFORM_BASALT
text_layer_set_text_color(battery_text_layer, GColorWhite);
			#else
text_layer_set_text_color(battery_text_layer, GColorBlack);
#endif			  

		} else if (charge_state.charge_percent <= 100) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_100);
#ifdef PBL_PLATFORM_BASALT
text_layer_set_text_color(battery_text_layer, GColorWhite);
			#else
text_layer_set_text_color(battery_text_layer, GColorBlack);
#endif			  

		} else {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_100);
			  text_layer_set_text_color(battery_text_layer, GColorBlack);

        } 
    } 
    charge_percent = charge_state.charge_percent;   
	text_layer_set_text(battery_text_layer, battery_text);
} 

static void handle_bluetooth(bool connected) {
  if( !connected ) {
    //vibe!
    vibes_long_pulse();
  }
  layer_set_hidden(bitmap_layer_get_layer(bluetooth_layer), !connected);
}

void force_update_a(void) {
    handle_bluetooth(bluetooth_connection_service_peek());
    update_battery(battery_state_service_peek());
}

static unsigned short get_display_hour(unsigned short hour) {
  if (clock_is_24h_style()) {
    return hour;
  }

  unsigned short display_hour = hour % 12;

  // Converts "0" to "12"
  return display_hour ? display_hour : 12;
}

static void update_time (struct tm *current_time) {
  unsigned short display_hour = get_display_hour(current_time->tm_hour);

  int yPos = 69;
  set_container_image(&time_digits_images[0], time_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(13, yPos));
  set_container_image(&time_digits_images[1], time_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(41, yPos));

  set_container_image(&time_digits_images[2], time_digits_layers[2], BIG_DIGIT_IMAGE_RESOURCE_IDS[current_time->tm_min/10], GPoint(75, yPos));
  set_container_image(&time_digits_images[3], time_digits_layers[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[current_time->tm_min%10], GPoint(102, yPos));

  if (!clock_is_24h_style() && display_hour/10 == 0 ) {
 
  set_container_image(&time_digits_images[0], time_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(6, yPos));
  set_container_image(&time_digits_images[1], time_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(26, yPos));

  set_container_image(&time_digits_images[2], time_digits_layers[2], BIG_DIGIT_IMAGE_RESOURCE_IDS[current_time->tm_min/10], GPoint(64, yPos));
  set_container_image(&time_digits_images[3], time_digits_layers[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[current_time->tm_min%10], GPoint(92, yPos));
  }
	  
    if (display_hour/10 == 0) {
    	layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[0]), true);
    } else {
    	layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[0]), false);
    }
}

static void update_date (int day_of_month, int days_since_sunday) {
	
  set_container_image(&day_name_image, day_name_layer, DAY_NAME_IMAGE_RESOURCE_IDS[days_since_sunday], GPoint(23, 47));

  set_container_image(&date_digits_images[0], date_digits_layers[0], DATENUM_IMAGE_RESOURCE_IDS[day_of_month/10], GPoint(59, 130));
  set_container_image(&date_digits_images[1], date_digits_layers[1], DATENUM_IMAGE_RESOURCE_IDS[day_of_month%10], GPoint(71, 130));
}

static void update_month (int months_since_january) {
  int month_value = months_since_january;
  set_container_image(&month_image, month_layer, MONTH_IMAGE_RESOURCE_IDS[month_value], GPoint(23, 109));
}

static void update_display(struct tm *current_time, bool force_update_a) {

	  layer_mark_dirty(minute_display_layer);

  if (current_time->tm_sec == 0 || force_update_a) {
  	// the hours and minutes
    update_time(current_time);

    if ((current_time->tm_hour == 0 && current_time->tm_min == 0) || force_update_a) {
  		// the day and date
      update_date(current_time->tm_mday, current_time->tm_wday);

	  if (current_time->tm_mday == 1 || force_update_a) {
    	update_month(current_time->tm_mon); 
      }
  	}
  }
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  update_display(tick_time, false);
}

static void init(void) {
  memset(&time_digits_layers, 0, sizeof(time_digits_layers));
  memset(&time_digits_images, 0, sizeof(time_digits_images));
	
  memset(&date_digits_layers, 0, sizeof(date_digits_layers));
  memset(&date_digits_images, 0, sizeof(date_digits_images));
	
  window = window_create();
  if (window == NULL) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "OOM: couldn't allocate window");
      return;
  }
	
  window_set_background_color(window, GColorBlack);

  window_stack_push(window, true /* Animated */);
  Layer *window_layer = window_get_root_layer(window);
	
  GRect bounds = layer_get_bounds(window_layer);

  custom_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CUSTOM_14));

  // layers
	
  img_battery_100   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_100);
  img_battery_90   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_90);
  img_battery_80   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_80);
  img_battery_70   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_70);
  img_battery_60   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_60);
  img_battery_50   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_50);
  img_battery_40   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_40);
  img_battery_30   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_30);
  img_battery_20    = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_20);
  img_battery_10    = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_10);
  img_battery_charge = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_CHARGING);

  layer_batt_img  = bitmap_layer_create(GRect(101, 126, 50, 50));
  bitmap_layer_set_bitmap(layer_batt_img, img_battery_100);
  layer_add_child(window_layer, bitmap_layer_get_layer(layer_batt_img));
	
  // Init the layer for the second display
  minute_display_layer = layer_create(bounds);
  layer_set_update_proc(minute_display_layer, minute_display_layer_update_callback);
  layer_add_child(window_layer, minute_display_layer);

  // Init the second segment path
  minute_segment_path = gpath_create(&MINUTE_SEGMENT_PATH_POINTS);
  gpath_move_to(minute_segment_path, grect_center_point(&bounds));
	
  battery_text_layer = text_layer_create(GRect(111, 142, 30, 16));
  text_layer_set_text_color(battery_text_layer, GColorBlack);
  text_layer_set_background_color(battery_text_layer, GColorClear);
  text_layer_set_font(battery_text_layer, custom_font);
  text_layer_set_text_alignment(battery_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(battery_text_layer));				

  bluetooth_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH);
#ifdef PBL_PLATFORM_BASALT
  GRect bitmap_bounds = gbitmap_get_bounds(bluetooth_image);
#else
  GRect bitmap_bounds = bluetooth_image->bounds;
#endif
  GRect frame = GRect(68, 27, bitmap_bounds.size.w, bitmap_bounds.size.h);
  bluetooth_layer = bitmap_layer_create(frame);
  bitmap_layer_set_bitmap(bluetooth_layer, bluetooth_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(bluetooth_layer));	

  // Create time and date layers
  GRect dummy_frame = { {0, 0}, {0, 0} };
	
  day_name_layer = bitmap_layer_create(dummy_frame);
  layer_add_child(window_layer, bitmap_layer_get_layer(day_name_layer));
	
  month_layer = bitmap_layer_create(dummy_frame);
  layer_add_child(window_layer, bitmap_layer_get_layer(month_layer));	
	
  for (int i = 0; i < TOTAL_TIME_DIGITS; ++i) {
    time_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(time_digits_layers[i]));
  }
	
  for (int i = 0; i < TOTAL_DATE_DIGITS; ++i) {
    date_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(date_digits_layers[i]));
  }

	// draw first frame
    force_update_a();
	
  // Avoids a blank screen on watch start.
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);

  update_display(tick_time, true);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
  bluetooth_connection_service_subscribe(&handle_bluetooth);
  battery_state_service_subscribe(&update_battery);

}

static void deinit(void) {
 
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  tick_timer_service_unsubscribe();

  layer_remove_from_parent(bitmap_layer_get_layer(bluetooth_layer));
  bitmap_layer_destroy(bluetooth_layer);
  gbitmap_destroy(bluetooth_image);
	
  layer_remove_from_parent(bitmap_layer_get_layer(day_name_layer));
  bitmap_layer_destroy(day_name_layer);
  gbitmap_destroy(day_name_image);

  layer_remove_from_parent(bitmap_layer_get_layer(month_layer));
  bitmap_layer_destroy(month_layer);
  gbitmap_destroy(month_image);
	
  for (int i = 0; i < TOTAL_DATE_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(date_digits_layers[i]));
    gbitmap_destroy(date_digits_images[i]);
    bitmap_layer_destroy(date_digits_layers[i]);
  }

  for (int i = 0; i < TOTAL_TIME_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(time_digits_layers[i]));
    gbitmap_destroy(time_digits_images[i]);
    bitmap_layer_destroy(time_digits_layers[i]);
  }

  layer_remove_from_parent(bitmap_layer_get_layer(layer_batt_img));
  bitmap_layer_destroy(layer_batt_img);
	
  gbitmap_destroy(img_battery_100);
  gbitmap_destroy(img_battery_90);
  gbitmap_destroy(img_battery_80);
  gbitmap_destroy(img_battery_70);
  gbitmap_destroy(img_battery_60);
  gbitmap_destroy(img_battery_50);
  gbitmap_destroy(img_battery_40);
  gbitmap_destroy(img_battery_30);
  gbitmap_destroy(img_battery_20);
  gbitmap_destroy(img_battery_10);
  gbitmap_destroy(img_battery_charge);		

  text_layer_destroy( battery_text_layer );

  fonts_unload_custom_font(custom_font);

  layer_remove_from_parent(minute_display_layer);
  layer_destroy(minute_display_layer);
  gpath_destroy(minute_segment_path);
	
  layer_remove_from_parent(window_layer);
  layer_destroy(window_layer);
	
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
