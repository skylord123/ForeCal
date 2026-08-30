#include "pebble.h"
#include "common.h"
#include "effect_layer.h"
#include "layout.h"

#define SAVEDATA_KEY 30
#define SAVE_VER_KEY 99
#define SAVE_VER 7
#define MAX_RETRIES 3
#define MAX_RETRIES_HOURLY 5
#define RETRY_INTERVAL 5000
  
static Window *window;

static bool loading = false;

static Layer *current_layer = NULL;
static TextLayer *clock_layer = NULL;
static GFont clock_font = NULL;
static TextLayer *pm_layer = NULL;
static TextLayer *week_layer = NULL;
static TextLayer *date_layer = NULL;
static AppTimer *bt_timer = NULL;
static GBitmap *bt_icon = NULL;
static BitmapLayer *bt_layer = NULL;
static GBitmap *batt_icon = NULL;
static BitmapLayer *batt_layer = NULL;

static TextLayer *curr_temp_layer = NULL;
static TextLayer *wind_speed_layer = NULL;

static Layer *forecast_layer = NULL;
static TextLayer *status_bg_layer = NULL;
static TextLayer *forecast_day_layer = NULL;
static TextLayer *status_layer = NULL;
static AppTimer *weatherinit_timer = NULL;
static AppTimer *status_timer = NULL;
static TextLayer *condition_layer = NULL;
static TextLayer *high_temp_layer = NULL;
static TextLayer *high_label_layer = NULL;
static TextLayer *low_temp_layer = NULL;
static TextLayer *low_label_layer = NULL;
static TextLayer *sun_rise_set_layer = NULL;

static BitmapLayer *icon_layer;
static GBitmap *icon_bitmap = NULL;

static BitmapLayer *sun_layer;
static GBitmap *sun_bitmap = NULL;

static Layer *cal_layer = NULL;

static EffectLayer *daymode_layer = NULL;
static EffectLayer *brightness_inverter = NULL;

static const uint32_t bt_warn_pattern[] = { 1000, 500, 1000 };
static char *weekdays[7] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};

static const int inbound_size = 512;
static const int outbound_size = 512;
static AppSync sync;
static uint8_t sync_buffer[512];

static char current_time[] = "00:00";
static char current_date[] = "Sun Jan 01";
static char current_week[4] = "";

static savedata_t s_savedata;
static int prev_daytime = 99;
static bool force_sun_update = false;
static int sun_update_count = 0;
static bool last_error = false;
static char err_msg[50] = "";
static uint8_t retry_count = 0;
static uint8_t retry_count_hourly = 0;
static AppTimer *retry_timer = NULL;
static bool bt_connected = false;
static batt_level_t last_batt_level = BATT_NA;
static time_t last_update_attempt;
static bool endpoint_configured = false;
static uint8_t last_battery_percent = 0;
static bool last_is_charging = false;
static uint8_t last_request_battery = 0;  // Track last request_battery value to detect transitions
static bool battery_sync_pending = false;  // Track if battery sync is pending (waiting for app_sync to initialize)
#if (defined(PBL_HEALTH) && defined(PBL_COLOR))
static Layer *steps_layer = NULL;
static bool steps_on = false;
static HealthValue steps_now = 0;
static HealthValue steps_avg_now = 0;
static HealthValue steps_avg_day = 0;
#endif

// App Message Keys for Tuples transferred from Javascript
enum MessageKey {
  WEATHER_STATUS_KEY = 0,
  WEATHER_CURR_TEMP_KEY = 1,
  WEATHER_SUN_RISE_SET_KEY = 2,
  WEATHER_FORECAST_DAY_KEY = 3,
  WEATHER_HIGH_TEMP_KEY = 4,
  WEATHER_LOW_TEMP_KEY = 5,
  WEATHER_ICON_KEY = 6,
  WEATHER_CONDITION_KEY = 7,
  WEATHER_DAYMODE_KEY = 8,
  WEATHER_CITY_KEY = 9,
  WEATHER_SUN_RISE_HOUR_KEY = 10,
  WEATHER_SUN_RISE_MIN_KEY = 11,
  WEATHER_SUN_SET_HOUR_KEY = 12,
  WEATHER_SUN_SET_MIN_KEY = 13,
  WEATHER_AUTO_DAYMODE_KEY = 14,
  CAL_FIRST_DAY_KEY = 16,
  CAL_OFFSET_KEY = 17,
  SHOW_BT_KEY = 18,
  SHOW_BATT_KEY = 19,
  TIME_24HR_KEY = 20,
  LOC_CHANGED_KEY = 21,
  BT_VIBES_KEY = 22,
  DATE_FORMAT_KEY = 23,
  SHOW_WIND_KEY = 24,
  WIND_SPEED_KEY = 25,
  FORECAST_HOUR_KEY = 26,
  FORECAST_MIN_KEY = 27,
  QT_START_HOUR_KEY = 28,
  QT_START_MIN_KEY = 29,
  QT_END_HOUR_KEY = 30,
  QT_END_MIN_KEY = 31,
  QT_BT_VIBES_KEY = 32,
  QT_FETCH_WEATHER_KEY = 33,
  SHOW_WEEK_KEY = 34,
  SHOW_STEPS_KEY = 35,
  WEATHER_UPDATE_INTERVAL_KEY = 36,
  REQUEST_BATTERY_KEY = 37,
  BATTERY_PERCENT_KEY = 38,
  ENDPOINT_CONFIGURED_KEY = 39,
  IS_CHARGING_KEY = 40,
  WEATHER_FETCHED_KEY = 99
};

// Weather icon resources defined in order to match Javascript icon values
static const uint32_t WEATHER_ICONS[] = {
  RESOURCE_ID_IMAGE_NA, //0
  RESOURCE_ID_IMAGE_SUNNY, //1
  RESOURCE_ID_IMAGE_PARTLYCLOUDY, //2
  RESOURCE_ID_IMAGE_CLOUDY, //3
  RESOURCE_ID_IMAGE_COLD, //4
  RESOURCE_ID_IMAGE_HOT, //5
  RESOURCE_ID_IMAGE_WINDY, //6
  RESOURCE_ID_IMAGE_LOWVISIBILITY, //7
  RESOURCE_ID_IMAGE_DRIZZLE, //8
  RESOURCE_ID_IMAGE_RAIN, //9
  RESOURCE_ID_IMAGE_LIGHTSNOW, //10
  RESOURCE_ID_IMAGE_MIXEDSNOW, //11
  RESOURCE_ID_IMAGE_HAIL, //12
  RESOURCE_ID_IMAGE_SNOW, //13
  RESOURCE_ID_IMAGE_THUNDERSHOWERS, //14
  RESOURCE_ID_IMAGE_ISOLATEDTHUNDERSTORMS, //15
  RESOURCE_ID_IMAGE_SCATTEREDTHUNDERSTORMS, //16
  RESOURCE_ID_IMAGE_STORM, //17
  RESOURCE_ID_IMAGE_TORNADO, //18
  RESOURCE_ID_IMAGE_HURRICANE //19
};

// Procedure that sends the Pebble 12/24hr setting to the Phone and initializes the first weather call
static void init_weather(void) {  
  Tuplet values[] = {
    TupletInteger(TIME_24HR_KEY, clock_is_24h_style() ? 1 : 0)
  };
  
  app_sync_set(&sync, values, 1);
}

// Procedure that triggers the weather data to update via Javascript
static void update_weather(void) {
  last_error = false;
  text_layer_set_text(status_layer, "Fetching...");
  
  Tuplet values[] = {
    TupletCString(WEATHER_CITY_KEY, "Fetching...")
  };
  
  app_sync_set(&sync, values, 1);
}

// Procedure that sends battery percentage and charging status to the phone for remote sync
static void send_battery_to_phone(uint8_t battery_percent, bool is_charging) {
  if (!endpoint_configured) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Endpoint not configured, skipping battery sync");
    return;
  }

  // If still loading (app_sync not initialized yet), mark as pending and return
  if (loading) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App sync not ready, marking battery sync as pending");
    battery_sync_pending = true;
    return;
  }

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Sending battery to phone: %d%% (charging: %s)",
          battery_percent, is_charging ? "yes" : "no");

  Tuplet values[] = {
    TupletInteger(BATTERY_PERCENT_KEY, battery_percent),
    TupletInteger(IS_CHARGING_KEY, is_charging ? 1 : 0)
  };

  app_sync_set(&sync, values, 2);
}

// Timer event that fires after the app sync init to differentiate between
// the tuple callback due to the init and the callback due to the first phone comm success
static void handle_weatherinit_timer(void *data) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather init timer event fired");
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Last Update: %ld", s_savedata.last_update);
  loading = false;
  weatherinit_timer = NULL;

  // Send pending battery sync if any (battery changed while app_sync was initializing)
  if (battery_sync_pending) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Sending pending battery sync: %d%% (charging: %s)",
            last_battery_percent, last_is_charging ? "yes" : "no");
    battery_sync_pending = false;
    send_battery_to_phone(last_battery_percent, last_is_charging);
  }

  init_weather();
}

// Timer event that retries updating the weather after an error
static void handle_retry_timer(void *data) {
  retry_timer = NULL;
  update_weather();
}

// Timer event that shows status after displaying the City/Error for 5 seconds
static void handle_status_timer(void *data) {
#ifdef DEBUG
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Status timer event fired");
#endif
  if (status_timer != NULL && strlen(s_savedata.status) > 0)
    text_layer_set_text(status_layer, s_savedata.status);
  else {
#ifdef DEBUG
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Status is blank");
#endif
  }
  
  status_timer = NULL;
}

// App message communication error
static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  if (status_timer != NULL) {
    app_timer_cancel(status_timer);
    status_timer = NULL;
  }
  
  APP_LOG(APP_LOG_LEVEL_ERROR, "App Message Sync Error: %d", app_message_error);
  last_error = true;
  bool retry = false;
  
  if (dict_error == DICT_OK) {
    switch (app_message_error) {
      case APP_MSG_NOT_CONNECTED:
        if (bluetooth_connection_service_peek()) {
          strncpy(err_msg, "Run phone app", sizeof(err_msg));
          retry = true;
        } else
          strncpy(err_msg, "BT not conn.", sizeof(err_msg));
        break;
      case APP_MSG_SEND_TIMEOUT:
        strncpy(err_msg, "Comm Timeout", sizeof(err_msg));
        retry = true;
        break;
      case APP_MSG_BUSY:
        strncpy(err_msg, "Comm Busy", sizeof(err_msg));
        retry = true;
        break;
      case APP_MSG_BUFFER_OVERFLOW:
        strncpy(err_msg, "Comm Overflow", sizeof(err_msg));
        break;
      default:
        snprintf(err_msg, sizeof(err_msg), "Comm error:%d", app_message_error);
    }
    
  } else {
    snprintf(err_msg, sizeof(err_msg), "Data error:%d", dict_error);
  }
  
  text_layer_set_text(status_layer, err_msg);
  
  if (retry) {
    // If there is a possibility that retrying may succeed, try updating the weather
    // in RETRY_INTERVAL milliseconds
    if (++retry_count <= MAX_RETRIES && ++retry_count_hourly <= MAX_RETRIES_HOURLY) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather fetch retry: %d", retry_count);
      if (retry_timer == NULL)
        app_timer_register(RETRY_INTERVAL, handle_retry_timer, NULL);
      else
        app_timer_reschedule(retry_timer, RETRY_INTERVAL);
    } else {
      last_update_attempt = 0;
    }
  } else {
    last_update_attempt = 0;
  }
}

static void set_daymode(bool daymode_on) {
  layer_set_hidden(effect_layer_get_layer(daymode_layer), !daymode_on);
  layer_set_hidden(effect_layer_get_layer(brightness_inverter), !daymode_on);
}

static void update_sun_layer(struct tm *t) {
  if (s_savedata.sun_rise_hour == 99 || s_savedata.sun_rise_min == 99 ||
      s_savedata.sun_set_hour == 99 || s_savedata.sun_set_min == 99) {
    text_layer_set_text(sun_rise_set_layer, "");
    layer_set_hidden(bitmap_layer_get_layer(sun_layer), true);
    return;
  }

  if (t == NULL) {
    time_t temp = time(NULL);
    t = localtime(&temp);
  }

  bool daytime = true;
  if (t->tm_hour < s_savedata.sun_rise_hour ||
        (t->tm_hour == s_savedata.sun_rise_hour && t->tm_min <= s_savedata.sun_rise_min) ||
      t->tm_hour > s_savedata.sun_set_hour ||
        (t->tm_hour == s_savedata.sun_set_hour && t->tm_min >= s_savedata.sun_set_min))
    daytime = false;

  // Always reconcile daymode against current time when auto_daymode is on. Doing this
  // unconditionally (rather than only on the prev_daytime transition) means a stale or
  // out-of-sync s_savedata.daymode — for instance from a WEATHER_DAYMODE_KEY message that
  // disagreed with local time — gets corrected on the next tick instead of needing a
  // watchface restart. set_daymode is a pair of layer_set_hidden calls; cheap when no-op.
  if (s_savedata.auto_daymode && s_savedata.daymode != daytime) {
    s_savedata.daymode = daytime;
    set_daymode(daytime);
  }

  // Sun bitmap and rise/set text only need refreshing on transition (or when location
  // changed). Loading a bitmap resource every minute would be wasteful.
  bool transition = (daytime ? prev_daytime != 1 : prev_daytime != 0);
  bool force = (force_sun_update && sun_update_count >= 4);

  if (transition || force) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Updating sun layer");

    if (sun_bitmap) {
      gbitmap_destroy(sun_bitmap);
      sun_bitmap = NULL;
    }

    // Constrain to real clock ranges before formatting. The fields are uint8_t, so
    // without this the compiler assumes 0-255 (3 digits) and flags the fixed 7-byte
    // sun_rise_set as a possible truncation target; the modulo also guards against a
    // corrupt persisted hour/min. The field cannot simply be widened because it sits
    // mid-struct in the persisted savedata_t and would shift every following field.
    if (daytime) {
      int h = s_savedata.sun_set_hour % 24;
      int m = s_savedata.sun_set_min % 60;
      if (clock_is_24h_style())
        snprintf(s_savedata.sun_rise_set, sizeof(s_savedata.sun_rise_set), "%d:%.2d", h, m);
      else
        snprintf(s_savedata.sun_rise_set, sizeof(s_savedata.sun_rise_set), "%d:%.2d%s",
                 ((h + 11) % 12) + 1, m, (h >= 12 ? "P" : "a"));
      sun_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SUNSET);
      prev_daytime = 1;
    } else {
      int h = s_savedata.sun_rise_hour % 24;
      int m = s_savedata.sun_rise_min % 60;
      if (clock_is_24h_style())
        snprintf(s_savedata.sun_rise_set, sizeof(s_savedata.sun_rise_set), "%d:%.2d", h, m);
      else
        snprintf(s_savedata.sun_rise_set, sizeof(s_savedata.sun_rise_set), "%d:%.2d%s",
                 ((h + 11) % 12) + 1, m, (h >= 12 ? "P" : "a"));
      sun_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SUNRISE);
      prev_daytime = 0;
    }

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Sun rise/set time: %s", s_savedata.sun_rise_set);

    bitmap_layer_set_bitmap(sun_layer, sun_bitmap);
    text_layer_set_text(sun_rise_set_layer, s_savedata.sun_rise_set);
    layer_set_hidden(bitmap_layer_get_layer(sun_layer), false);

    force_sun_update = false;
    sun_update_count = 0;
  } else if (!force_sun_update && sun_update_count >= 4) {
    sun_update_count = 0;
  }
}

// Updates the current date shown
static void update_date(struct tm *t) {
  
  if (t == NULL) {
    // Get current time if not passed in
    time_t temp;
    temp = time(NULL);
    t = localtime(&temp);
  }
  
  if (s_savedata.show_wind) {
    // Hide day name if showing wind speed
    if (s_savedata.date_format == 0)
      // Month, Date
      strftime(current_date, sizeof(current_date), "%b %d", t);
    else
      // Date, Month
      strftime(current_date, sizeof(current_date), "%d %b", t);
  } else {
    // Show day name as well
    if (s_savedata.date_format == 0)
      // Month, Date
      strftime(current_date, sizeof(current_date), "%a %b %d", t);
    else
      // Date, Month
      strftime(current_date, sizeof(current_date), "%a %d %b", t);
  }
  
  text_layer_set_text(date_layer, current_date);
}

#if (defined(PBL_HEALTH) && defined(PBL_COLOR))

// Draw the steps progress bar
static void draw_steps_progress(struct Layer *layer, GContext *ctx) {
  if (steps_on && steps_avg_day != 0) {
    GRect bounds = layer_get_bounds(layer);
#ifdef PBL_ROUND
    GRect icon_frame = layer_get_frame(bitmap_layer_get_layer(icon_layer));
#endif
    if (steps_now != 0) {
      if (steps_now >= steps_avg_now)
        graphics_context_set_fill_color(ctx, (s_savedata.daymode ? GColorIslamicGreen : GColorGreen));
      else
        graphics_context_set_fill_color(ctx, (s_savedata.daymode ? GColorBlueMoon : GColorVividCerulean));
#ifdef PBL_ROUND
      int steps_pos = ((bounds.size.w - icon_frame.size.w) * steps_now) / steps_avg_day;
      if (steps_pos > icon_frame.origin.x) steps_pos += icon_frame.size.w;
#else
      int steps_pos = (bounds.size.w * steps_now) / steps_avg_day;
#endif
      graphics_fill_rect(ctx, GRect(0, 0, steps_pos, bounds.size.h), 0, GCornersAll);
    }
    if (steps_avg_now != 0) {
      graphics_context_set_stroke_color(ctx, GColorOrange);
      graphics_context_set_stroke_width(ctx, 2);
#ifdef PBL_ROUND
      int avg_bar = ((bounds.size.w - icon_frame.size.w) * steps_avg_now) / steps_avg_day;
      if (avg_bar > icon_frame.origin.x) avg_bar += icon_frame.size.w;
#else
      int avg_bar = (bounds.size.w * steps_avg_now) / steps_avg_day;
#endif
      graphics_draw_line(ctx, GPoint(avg_bar, 0), GPoint(avg_bar, bounds.size.h));
    }
  }
}

// Get all the steps metrics and store in static variables
static void get_steps() {
  const HealthMetric metric = HealthMetricStepCount;
  const HealthServiceTimeScope scope = HealthServiceTimeScopeWeekly;
  const time_t day_start = time_start_of_today();
  const time_t day_end = day_start + SECONDS_PER_DAY;
  const time_t now = time(NULL);
  
  // If the daily average has not been fetched or is being refreshed, get it
  if (steps_avg_day == 0)
    steps_avg_day = health_service_sum_averaged(metric, day_start, day_end, scope);
  
  if (steps_avg_day == 0) {
    // If the daily average still isn't available, do not show any other metrics
    steps_avg_now = 0;
    steps_now = 0;
  } else {
    // Get the daily average up to now and the total steps for today
    steps_avg_now = health_service_sum_averaged(metric, day_start, now, scope);
    steps_now = health_service_sum(metric, day_start, now);
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Get Steps - Daily Avg: %d, Avg to Now: %d, Today's Steps: %d", (int)steps_avg_day, (int)steps_avg_now, (int)steps_now);
}

// Check if all steps metrics are available
static bool steps_available() {
  const HealthMetric metric = HealthMetricStepCount;
  const HealthServiceTimeScope scope = HealthServiceTimeScopeWeekly;
  const time_t day_start = time_start_of_today();
  const time_t day_end = day_start + SECONDS_PER_DAY;
  const time_t now = time(NULL);
  
  // Check if steps so far today are available
  if (!(health_service_metric_accessible(metric, day_start, now) & HealthServiceAccessibilityMaskAvailable))
    return false;
  
  // Check if daily average for today is available
  if (!(health_service_metric_averaged_accessible(metric, day_start, day_end, scope) & HealthServiceAccessibilityMaskAvailable))
    return false;
  
  // Check if daily average up to now is available
  if (!(health_service_metric_averaged_accessible(metric, day_start, now, scope) & HealthServiceAccessibilityMaskAvailable))
    return false;
  
  // If we get here, all metrics are available
  return true;
}

// Called whenever the health service registers an update
static void steps_update(HealthEventType event, void *context) {
  if (steps_on) {
    switch(event) {
      case HealthEventSignificantUpdate:
        // Force daily average update
        steps_avg_day = 0;
        
        // Recheck if metrics available and get steps if they are
        if (steps_available()) {
          get_steps();
        } else {
          // Otherwise reset everything else
          steps_avg_now = 0;
          steps_now = 0;
        }
        // Update steps progress display
        layer_mark_dirty(steps_layer);
        break;
      case HealthEventMovementUpdate:
        // Get step metrics and update progress display
        get_steps();
        layer_mark_dirty(steps_layer);
        break;
      default:
        // Ignore everything else
        break;
    }
  }
}

// Turns on step count progress bar (and related events)
static void enable_steps_display() {
  if (!steps_on) {
    // Check metrics are available
    if (steps_available()) {
      // Subscribe to updates
      if (health_service_events_subscribe(steps_update, NULL)) {
        // Get metrics and display progress bar
        get_steps();
        layer_mark_dirty(steps_layer);
        layer_set_hidden(steps_layer, false);
        steps_on = true;
      }
    }
  }
}

// Turns off step count display (and related events)
static void disable_steps_display() {
  if (steps_on) {
    health_service_events_unsubscribe();
    layer_set_hidden(steps_layer, true);
    steps_on = false;
  }
}
#endif

// Event fired when data received from Javascript
static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Message Key: %d", (int)key);
  switch (key) {
    case WEATHER_ICON_KEY:
      if (icon_bitmap == NULL || s_savedata.icon != new_tuple->value->uint8) {
        if (icon_bitmap) {
          gbitmap_destroy(icon_bitmap);
          icon_bitmap = NULL;
        }
        s_savedata.icon = new_tuple->value->uint8;
        layer_set_hidden(bitmap_layer_get_layer(icon_layer), (s_savedata.icon == 0));
        if (s_savedata.icon > 0 && s_savedata.icon < ARRAY_LENGTH(WEATHER_ICONS)) {
          icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[s_savedata.icon]);
          bitmap_layer_set_bitmap(icon_layer, icon_bitmap);
          layer_mark_dirty(bitmap_layer_get_layer(icon_layer));
        }
      }
      break;
    case WEATHER_STATUS_KEY:
      // Save status for displaying after showing City for 5 seconds
      strncpy(s_savedata.status, new_tuple->value->cstring, sizeof(s_savedata.status));
      s_savedata.status[sizeof(s_savedata.status) - 1] = '\0';
      break;
    case WEATHER_CITY_KEY:
      strncpy(s_savedata.city, new_tuple->value->cstring, sizeof(s_savedata.city));
      s_savedata.city[sizeof(s_savedata.city) - 1] = '\0';
      text_layer_set_text(status_layer, s_savedata.city);
      // Show City for 5 seconds and then replace with Status
      if (status_timer) {
        app_timer_reschedule(status_timer, 5000);
      }
      else {
        status_timer = app_timer_register(5000, handle_status_timer, NULL);
      }
      break;
    case WEATHER_CURR_TEMP_KEY:
      strncpy(s_savedata.curr_temp, new_tuple->value->cstring, sizeof(s_savedata.curr_temp));
      s_savedata.curr_temp[sizeof(s_savedata.curr_temp) - 1] = '\0';
      text_layer_set_text(curr_temp_layer, s_savedata.curr_temp);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Displaying Current Temp: %s", s_savedata.curr_temp);
      break;
    case WEATHER_FORECAST_DAY_KEY:
      strncpy(s_savedata.forecast_day, new_tuple->value->cstring, sizeof(s_savedata.forecast_day));
      s_savedata.forecast_day[sizeof(s_savedata.forecast_day) - 1] = '\0';
      text_layer_set_text(forecast_day_layer, s_savedata.forecast_day);
      break;
    case WEATHER_HIGH_TEMP_KEY:
      strncpy(s_savedata.high_temp, new_tuple->value->cstring, sizeof(s_savedata.high_temp));
      s_savedata.high_temp[sizeof(s_savedata.high_temp) - 1] = '\0';
      text_layer_set_text(high_temp_layer, s_savedata.high_temp);
      layer_set_hidden(text_layer_get_layer(high_label_layer), strlen(s_savedata.high_temp) == 0);
      break;
    case WEATHER_LOW_TEMP_KEY:
      strncpy(s_savedata.low_temp, new_tuple->value->cstring, sizeof(s_savedata.low_temp));
      s_savedata.low_temp[sizeof(s_savedata.low_temp) - 1] = '\0';
      text_layer_set_text(low_temp_layer, s_savedata.low_temp);
      layer_set_hidden(text_layer_get_layer(low_label_layer), strlen(s_savedata.low_temp) == 0);
      break;
    case WEATHER_CONDITION_KEY:
      strncpy(s_savedata.condition, new_tuple->value->cstring, sizeof(s_savedata.condition));
      s_savedata.condition[sizeof(s_savedata.condition) - 1] = '\0';
      text_layer_set_text(condition_layer, s_savedata.condition);
      break;
    case WEATHER_DAYMODE_KEY:
      s_savedata.daymode = (new_tuple->value->uint8 == 1);
      set_daymode(s_savedata.daymode);
      update_sun_layer(NULL);
      break;
    case WEATHER_SUN_RISE_HOUR_KEY:
      s_savedata.sun_rise_hour = new_tuple->value->uint8;
      sun_update_count++;
      update_sun_layer(NULL);
      break;
    case WEATHER_SUN_RISE_MIN_KEY:
      s_savedata.sun_rise_min = new_tuple->value->uint8;
      sun_update_count++;
      update_sun_layer(NULL);
      break;
    case WEATHER_SUN_SET_HOUR_KEY:
      s_savedata.sun_set_hour = new_tuple->value->uint8;
      sun_update_count++;
      update_sun_layer(NULL);
      break;
    case WEATHER_SUN_SET_MIN_KEY:
      s_savedata.sun_set_min = new_tuple->value->uint8;
      sun_update_count++;
      update_sun_layer(NULL);
      break;
    case WEATHER_AUTO_DAYMODE_KEY:
      s_savedata.auto_daymode = (new_tuple->value->uint8 == 1);
      update_sun_layer(NULL);
      break;
    case CAL_FIRST_DAY_KEY:
      s_savedata.startday = new_tuple->value->uint8;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "First Day: %d", s_savedata.startday);
      layer_mark_dirty(cal_layer);
      break;
    case CAL_OFFSET_KEY:
      s_savedata.cal_offset = new_tuple->value->uint8;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Calendar Offset: %d", s_savedata.cal_offset);
      layer_mark_dirty(cal_layer);
      break;
    case SHOW_BT_KEY:
      s_savedata.show_bt = (new_tuple->value->uint8 == 1);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Show BT: %d (%s)", s_savedata.show_bt, s_savedata.show_bt ? "ON" : "OFF");
      layer_set_hidden(bitmap_layer_get_layer(bt_layer), !bluetooth_connection_service_peek() || !s_savedata.show_bt);
      break;
    case BT_VIBES_KEY:
      s_savedata.bt_vibes = (new_tuple->value->uint8 == 1);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "BT Vibes: %d (%s)", s_savedata.bt_vibes, s_savedata.bt_vibes ? "ON" : "OFF");
      break;
    case SHOW_BATT_KEY:
      s_savedata.show_batt = (new_tuple->value->uint8 == 1);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Show Battery: %d", s_savedata.show_batt);
      layer_set_hidden(bitmap_layer_get_layer(batt_layer), !s_savedata.show_batt);
      break;
    case SHOW_WEEK_KEY:
      s_savedata.show_week = (new_tuple->value->uint8 == 1);
      layer_set_hidden(text_layer_get_layer(week_layer), !s_savedata.show_week);
      if (s_savedata.show_week) text_layer_set_text(pm_layer, "");
      break;
    case SHOW_STEPS_KEY:
      s_savedata.show_steps = (new_tuple->value->uint8 == 1);
#if (defined(PBL_HEALTH) && defined(PBL_COLOR))
      if (s_savedata.show_steps)
        enable_steps_display();
      else
        disable_steps_display();
#endif
      break;
    case LOC_CHANGED_KEY:
      if (!loading && (int)new_tuple->value->uint8 == 1) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Forcing sun layer update");
        force_sun_update = true;
        update_sun_layer(NULL);
      }
      break;
    case DATE_FORMAT_KEY:
      s_savedata.date_format = new_tuple->value->uint8;
      update_date(NULL);
      break;
    case SHOW_WIND_KEY:
      s_savedata.show_wind = (new_tuple->value->uint8 == 1);
      layer_set_hidden(text_layer_get_layer(wind_speed_layer), !s_savedata.show_wind);
      update_date(NULL);
      break;
    case WIND_SPEED_KEY:
      strncpy(s_savedata.wind_speed, new_tuple->value->cstring, sizeof(s_savedata.wind_speed));
      s_savedata.wind_speed[sizeof(s_savedata.wind_speed) - 1] = '\0';
      text_layer_set_text(wind_speed_layer, s_savedata.wind_speed);
      break;
    case FORECAST_HOUR_KEY:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "FORECAST HOUR: %d", new_tuple->value->uint8);
      s_savedata.forecast_hour = new_tuple->value->uint8;
      break;
    case FORECAST_MIN_KEY:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "FORECAST MIN: %d", new_tuple->value->uint8);
      s_savedata.forecast_min = new_tuple->value->uint8;
      break;
    case QT_START_HOUR_KEY:
      s_savedata.qt_start_hour = new_tuple->value->uint8;
      break;
    case QT_START_MIN_KEY:
      s_savedata.qt_start_min = new_tuple->value->uint8;
      break;
    case QT_END_HOUR_KEY:
      s_savedata.qt_end_hour = new_tuple->value->uint8;
      break;
    case QT_END_MIN_KEY:
      s_savedata.qt_end_min = new_tuple->value->uint8;
      break;
    case QT_BT_VIBES_KEY:
      s_savedata.qt_bt_vibes = (new_tuple->value->uint8 == 1);
      break;
    case QT_FETCH_WEATHER_KEY:
      s_savedata.qt_fetch_weather = (new_tuple->value->uint8 == 1);
      break;
    case WEATHER_UPDATE_INTERVAL_KEY:
      s_savedata.weather_update_interval = new_tuple->value->uint16;
      if (s_savedata.weather_update_interval < 1) {
        s_savedata.weather_update_interval = 60; // Default to 60 minutes if invalid
      }
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather update interval set to: %d minutes", s_savedata.weather_update_interval);
      break;
    case WEATHER_FETCHED_KEY:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "WEATHER FETCHED: %d", new_tuple->value->uint8);
      if (new_tuple->value->uint8 == 1) {
        // On a successful retrieve reset error retry counter and stop any retry
        retry_count = 0;
        if (retry_timer != NULL) app_timer_cancel(retry_timer);
        last_error = false;

        // Record the time of the successful update as the time when the update started
        if (last_update_attempt == 0) {
          // If there is no update start time, use now (without seconds)
          last_update_attempt = time(NULL);
          last_update_attempt -= last_update_attempt % 60;
        }
        s_savedata.last_update = last_update_attempt;
      }
      last_update_attempt = 0;
      break;
    case REQUEST_BATTERY_KEY:
      // Only respond when value transitions from 0 to 1 (new request)
      // This prevents infinite loops since AppSync callback fires for all tuples on every sync
      if (new_tuple->value->uint8 == 1 && last_request_battery == 0) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery request received from phone");
        // Phone is requesting current battery data - get fresh value
        BatteryChargeState batt_state = battery_state_service_peek();
        last_battery_percent = batt_state.charge_percent;
        last_is_charging = batt_state.is_charging;
        // Update persisted values
        s_savedata.last_battery_percent = batt_state.charge_percent;
        s_savedata.last_is_charging = batt_state.is_charging;
        send_battery_to_phone(batt_state.charge_percent, batt_state.is_charging);
      }
      last_request_battery = new_tuple->value->uint8;
      break;
    case ENDPOINT_CONFIGURED_KEY:
      endpoint_configured = (new_tuple->value->uint8 == 1);
      s_savedata.endpoint_configured = endpoint_configured;  // Persist the value
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Endpoint configured: %s", endpoint_configured ? "true" : "false");
      break;
    case BATTERY_PERCENT_KEY:
    case IS_CHARGING_KEY:
    case TIME_24HR_KEY:
      // These keys are for watch->phone communication, ignore if received from phone
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Unknown App Message Key: %d", (int)key);
      break;
  }
}

// Calculates whether tomorrow's forecast should be showing
static bool show_forecast_tomorrow() {
  time_t curr_time = time(NULL);
  struct tm *t = localtime(&curr_time);

  return (t->tm_hour > s_savedata.forecast_hour ||
      (t->tm_hour == s_savedata.forecast_hour && t->tm_min >= s_savedata.forecast_min));
}

// Calculates whether quiet time is active
static bool quiet_time_active() {
  time_t curr_time = time(NULL);
  struct tm *t = localtime(&curr_time);
  
  if ((s_savedata.qt_start_hour * 60) + s_savedata.qt_start_min >
      (s_savedata.qt_end_hour * 60) + s_savedata.qt_end_min) {
    // Quiet Time runs over midnight
    return ((t->tm_hour > s_savedata.qt_start_hour || 
             (t->tm_hour == s_savedata.qt_start_hour && t->tm_min >= s_savedata.qt_start_min)) ||
            (t->tm_hour < s_savedata.qt_end_hour ||
             (t->tm_hour == s_savedata.qt_end_hour && t->tm_min <= s_savedata.qt_end_min)));
  } else {
    // Quiet Time starts and ends on same day
    return ((t->tm_hour > s_savedata.qt_start_hour || 
             (t->tm_hour == s_savedata.qt_start_hour && t->tm_min >= s_savedata.qt_start_min)) &&
            (t->tm_hour < s_savedata.qt_end_hour ||
             (t->tm_hour == s_savedata.qt_end_hour && t->tm_min <= s_savedata.qt_end_min)));
  }
}

// Handle clock change events
static void handle_tick(struct tm *t, TimeUnits units_changed) {
  if ((units_changed & MINUTE_UNIT) != 0) {
    // Format time manually to avoid platform-specific differences with clock_copy_time_string()
    // that can cause display issues with SUBSET fonts (e.g., leading spaces on some platforms)
    if (clock_is_24h_style()) {
      strftime(current_time, sizeof(current_time), "%H:%M", t);
    } else {
      strftime(current_time, sizeof(current_time), "%I:%M", t);
      // Remove leading zero for 12-hour format (e.g., "09:32" -> "9:32")
      if (current_time[0] == '0') {
        memmove(current_time, current_time + 1, sizeof(current_time) - 1);
      }
    }
    text_layer_set_text(clock_layer, current_time);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Current time: %s", current_time);
    
    // Weather update logic - skip entirely during initial loading
    if (!loading) {
      time_t now = time(NULL);

      // Calculate update interval in seconds (default 60 minutes if not set)
      uint16_t interval_minutes = (s_savedata.weather_update_interval > 0) ? s_savedata.weather_update_interval : 60;
      time_t interval_seconds = (time_t)(interval_minutes * 60);

      // Time since last successful update. Treat "no prior update" and a future
      // last_update (clock went backwards, e.g. timezone change overnight) the same as
      // "interval elapsed" so we always recover into an update on the next non-QT tick.
      time_t time_since_update;
      if (s_savedata.last_update <= 0 || s_savedata.last_update > now) {
        time_since_update = interval_seconds;
      } else {
        time_since_update = now - s_savedata.last_update;
      }

      bool in_quiet_time = quiet_time_active();
      bool qt_blocks_update = in_quiet_time && !s_savedata.qt_fetch_weather;

      // Special triggers fire even inside QT: midnight rolls the forecast day, and the
      // forecast hour switches Today/Tomorrow display.
      bool is_midnight = (t->tm_hour == 0 && t->tm_min == 0);
      bool is_forecast_transition = (t->tm_hour == s_savedata.forecast_hour && t->tm_min == s_savedata.forecast_min);
      bool special_trigger = is_midnight || is_forecast_transition;

      bool interval_due = (time_since_update >= interval_seconds);

      if (special_trigger) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather update: special trigger (%s)",
                is_midnight ? "midnight" : "forecast transition");
        last_update_attempt = now - (now % 60);
        update_weather();
      } else if (interval_due && !qt_blocks_update) {
        // Out of QT (or QT doesn't block fetches) and interval elapsed - update.
        // Earlier versions extended the post-QT interval by the full QT duration as
        // anti-thundering-herd, but that could leave weather stale for hours after QT
        // ended (especially with short intervals + long QT) and any stale flag would
        // silently suppress recovery. A single user has no herd to spread.
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather update: interval elapsed (%d min since last)",
                (int)(time_since_update / 60));
        last_update_attempt = now - (now % 60);
        update_weather();
      } else if (interval_due && qt_blocks_update) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather update skipped: quiet time active");
      }
    }
    
    update_sun_layer(t);
  }
  if ((units_changed & HOUR_UNIT) != 0) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Hour changed");
    retry_count_hourly = 0;
    if (clock_is_24h_style() || s_savedata.show_week) {
      text_layer_set_text(pm_layer, "");
    }
    else {
      if (t->tm_hour >= 12) {
        text_layer_set_text(pm_layer, "PM");
      }
      else {
        text_layer_set_text(pm_layer, "AM");
      }
    }
  }
  if ((units_changed & DAY_UNIT) != 0) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Day changed");
    update_date(t);
    strftime(current_week, sizeof(current_week), "w%V", t);
    text_layer_set_text(week_layer, current_week);
    // Trigger redraw of calendar
    layer_mark_dirty(cal_layer);
  }
}

// Try updating the weather again after a delay when BT reconnects to give everything time to reopen
static void handle_reconnect_delay(void *data) {
  // Record the time when the update attempt started without seconds
  last_update_attempt = time(NULL);
  last_update_attempt -= last_update_attempt % 60;
  update_weather();
}

// Show or hide Bluetooth icon based on connection status and vibrate on disconnect
static void update_bt_icon(bool connected) {
  if (connected) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "BT Connected");
    layer_set_hidden(bitmap_layer_get_layer(bt_layer), !s_savedata.show_bt);
  }
  else {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "BT DISCONNECTED");
    layer_set_hidden(bitmap_layer_get_layer(bt_layer), true);
    
    if (!loading && s_savedata.bt_vibes && (!quiet_time_active() || s_savedata.qt_bt_vibes)) {
      // Play long vibe pattern on BT disconnect
      // as long as not during Quiet Time or else set to vibe during Quiet Time
      VibePattern pat = {
        .durations = bt_warn_pattern,
        .num_segments = ARRAY_LENGTH(bt_warn_pattern),
      };
      vibes_enqueue_custom_pattern(pat);
    }
  }
  layer_mark_dirty(current_layer);
}

// Handle Bluetooth disconnect timer to show disconnect after 15 seconds
static void handle_bt_timeout(void *data) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "BT Update - 15sec");
  bt_timer = NULL;
  bt_connected = bluetooth_connection_service_peek();
  update_bt_icon(bt_connected);
}

// Handle Bluetooth status updates
static void handle_bt_update(bool connected) {
  if (connected) {
    // If connected, immediately update BT icon
    if (bt_timer) {
      app_timer_cancel(bt_timer);
      bt_timer = NULL;
    }
    
    update_bt_icon(connected);
    
    if (!loading) {
      // If wasn't connected for at least 15 seconds, play short vibe on reconnecting
      // as long as not during Quiet Time or else set to vibe during Quiet Time
      if (!bt_connected && s_savedata.bt_vibes && (!quiet_time_active() || s_savedata.qt_bt_vibes)) vibes_short_pulse();
      
      // If reconnected after last weather update failed, try updating the weather again in 5 seconds
      if (last_error) app_timer_register(5000, handle_reconnect_delay, NULL);
    }
    
    bt_connected = true;
  }
  else {
    // If disconnected, wait 15 seconds to update BT icon in case of reconnect
    if (bt_timer) {
      app_timer_reschedule(bt_timer, 15000);
    } else {
      bt_timer = app_timer_register(15000, handle_bt_timeout, NULL);
    }
  }
}

// Handle battery status updates
static void handle_batt_update(BatteryChargeState batt_status) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery Update: %d%% (%s)", batt_status.charge_percent,
          batt_status.is_charging ? "Charging" : "NOT Charging");

  batt_level_t new_batt_level;

  if (batt_status.is_charging) {
    new_batt_level = BATT_CHARGING;
  }
  else {
    if (batt_status.charge_percent > 75) {
      new_batt_level = BATT_100;
    } else if (batt_status.charge_percent > 50) {
      new_batt_level = BATT_75;
    } else if (batt_status.charge_percent > 25) {
      new_batt_level = BATT_50;
    } else {
      new_batt_level = BATT_25;
    }
  }

  // Send battery data to phone if percentage or charging state changed
  if (batt_status.charge_percent != last_battery_percent ||
      batt_status.is_charging != last_is_charging) {
    last_battery_percent = batt_status.charge_percent;
    last_is_charging = batt_status.is_charging;
    // Update persisted values
    s_savedata.last_battery_percent = batt_status.charge_percent;
    s_savedata.last_is_charging = batt_status.is_charging;
    send_battery_to_phone(batt_status.charge_percent, batt_status.is_charging);
  }

  if (new_batt_level != last_batt_level) {
    if (batt_icon) {
      gbitmap_destroy(batt_icon);
      batt_icon = NULL;
    }

    switch (new_batt_level) {
      case BATT_CHARGING:
        batt_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_CHARGE);
        break;
      case BATT_25:
        batt_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_25);
        break;
      case BATT_50:
        batt_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_50);
        break;
      case BATT_75:
        batt_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_75);
        break;
      case BATT_100:
        batt_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_100);
        break;
      case BATT_NA:
        batt_icon = NULL;
    }

    bitmap_layer_set_bitmap(batt_layer, batt_icon);
    layer_mark_dirty(bitmap_layer_get_layer(batt_layer));
    last_batt_level = new_batt_level;
  }
}

// Draw dates for a single week in the calendar
static void cal_week_draw_dates(GContext *ctx, int start_date, int curr_mon_len, int prev_mon_len, GColor font_color, int ypos, int highlight_day) {
  
  int curr_date;
  char curr_date_str[12];
  GColor back_color;
  
  graphics_context_set_text_color(ctx, font_color);
  
  for (int d = 0; d < 7; d++) {
    // Calculate the current date being drawn
    if ((start_date + d) < 1)
      curr_date = start_date + d + prev_mon_len;
    else if ((start_date + d) > curr_mon_len)
      curr_date = start_date + d - curr_mon_len;
    else
      curr_date = start_date + d;
    
    if (curr_date == highlight_day) {
      // invert the highlghted date (use subtle back color on color Pebbles)
      if (gcolor_equal(font_color, GColorBlack)) {
        if (layer_get_hidden(effect_layer_get_layer(daymode_layer))) { 
          back_color = COLOR_FALLBACK(GColorCobaltBlue, GColorBlack);;
          graphics_context_set_text_color(ctx, GColorWhite);
        } else {
          back_color = COLOR_FALLBACK(GColorCyan, GColorBlack);
          graphics_context_set_text_color(ctx, GColorWhite);
        }
      } else {
        if (layer_get_hidden(effect_layer_get_layer(daymode_layer))) {
          back_color = COLOR_FALLBACK(GColorCyan, GColorWhite);
          graphics_context_set_text_color(ctx, GColorBlack);
        } else {
          back_color = COLOR_FALLBACK(GColorDukeBlue, GColorWhite);
          graphics_context_set_text_color(ctx, GColorBlack);
        }
      }
      
      graphics_context_set_fill_color(ctx, back_color);
      graphics_fill_rect(ctx, GRect(d * LAYOUT_CAL_CELL_PITCH, ypos + LAYOUT_CAL_HL_DY, LAYOUT_CAL_CELL_W, LAYOUT_CAL_HL_H), 0, GCornerNone);
      
    }
    
    // Draw the date text in the correct calendar cell
    snprintf(curr_date_str, sizeof(curr_date_str), "%d", curr_date);
    graphics_draw_text(ctx, curr_date_str, fonts_get_system_font(LAYOUT_CAL_FONT_BOLD), 
                       GRect(d * LAYOUT_CAL_CELL_PITCH, ypos, LAYOUT_CAL_CELL_W, LAYOUT_CAL_TEXT_H), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
    
    if (curr_date == highlight_day) graphics_context_set_text_color(ctx, font_color);
  }
}

// Handle drawing of the 3 week calendar layer
static void cal_layer_draw(Layer *layer, GContext *ctx) {
  
  GRect bounds = layer_get_bounds(layer);
  
  // Sanitize calendar parameters that somehow get messed up on some watches
  if (s_savedata.startday > 1) s_savedata.startday = 0;
  if (s_savedata.cal_offset != 0 && s_savedata.cal_offset != 7) s_savedata.cal_offset = 0;
  
  // Paint calendar background
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, bounds.size.h), 0, GCornerNone);
  
  int rowtexttop1;
  int rowtexttop2;
  int rowtexttop3;
  
  // Paint inverted rows background (Pebble Times have rounded corners so need to draw calendar more compact)
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, LAYOUT_CAL_BAND1_Y, bounds.size.w, LAYOUT_CAL_BAND1_H), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(0, LAYOUT_CAL_BAND2_Y, bounds.size.w, LAYOUT_CAL_BAND2_H), 0, GCornerNone);
  rowtexttop1 = LAYOUT_CAL_ROW1_Y;
  rowtexttop2 = LAYOUT_CAL_ROW2_Y;
  rowtexttop3 = LAYOUT_CAL_ROW3_Y;
  
  // Get current time
  struct tm *t;
  time_t temp;
  temp = time(NULL);
  t = localtime(&temp);
  
  graphics_context_set_text_color(ctx, GColorBlack);
  GFont curr_font;
  
  // Draw week day names
  for (int d = 0; d < 7; d++) {
    if (t->tm_wday == ((d + s_savedata.startday) % 7))
      curr_font = fonts_get_system_font(LAYOUT_CAL_FONT_BOLD);
    else
      curr_font = fonts_get_system_font(LAYOUT_CAL_FONT);
    graphics_draw_text(ctx, weekdays[(d + s_savedata.startday) % 7], curr_font, 
                       GRect(d * LAYOUT_CAL_CELL_PITCH, LAYOUT_CAL_HDR_Y, LAYOUT_CAL_CELL_W +
                             PBL_IF_ROUND_ELSE(((d+s_savedata.startday) == 3 ? 1 : 0), 0), LAYOUT_CAL_TEXT_H), 
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }
  
  // Calculate leap year and month lengths
  int leap_year = (((1900 + t->tm_year) % 100) == 0 ? 0 : (((1900 + t->tm_year) % 4) == 0) ? 1 : 0);
  int prev_mon = (t->tm_mon) == 0 ? 12 : t->tm_mon;
  int curr_mon = t->tm_mon + 1;
  int prev_mon_len = 31 - ((prev_mon == 2) ? (3 - leap_year) : ((prev_mon - 1) % 7 % 2));
  int curr_mon_len = 31 - ((curr_mon == 2) ? (3 - leap_year) : ((curr_mon - 1) % 7 % 2));
  
  // If the current week day is before the calendar week start day, push the dates down by 1 more week
  int extra_offset = (t->tm_wday < s_savedata.startday) ? -7 : 0;
  
  // Draw previous week dates
  cal_week_draw_dates(ctx, t->tm_mday - t->tm_wday - 7 + s_savedata.startday + s_savedata.cal_offset + extra_offset, 
                      curr_mon_len, prev_mon_len, GColorWhite, rowtexttop1, t->tm_mday);
  // Draw current week dates
  cal_week_draw_dates(ctx, t->tm_mday - t->tm_wday + s_savedata.startday + s_savedata.cal_offset + extra_offset, 
                      curr_mon_len, prev_mon_len, GColorBlack, rowtexttop2, t->tm_mday);
  // Draw next week dates
  cal_week_draw_dates(ctx, t->tm_mday - t->tm_wday + 7 + s_savedata.startday + s_savedata.cal_offset + extra_offset, 
                      curr_mon_len, prev_mon_len, GColorWhite, rowtexttop3, t->tm_mday);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  
  loading = true;
  
  // Init settings and weather data
  s_savedata.show_bt = true;
  s_savedata.bt_vibes = true;
  s_savedata.show_batt = true;
  s_savedata.startday = 0;
  s_savedata.cal_offset = 0;
  s_savedata.daymode = false;
  s_savedata.status[0] = '\0';
  s_savedata.curr_temp[0] = '\0';
  s_savedata.forecast_day[0] = '\0';
  s_savedata.high_temp[0] = '\0';
  s_savedata.low_temp[0] = '\0';
  s_savedata.icon = 0;
  s_savedata.condition[0] = '\0';
  s_savedata.sun_rise_hour = 99;
  s_savedata.sun_rise_min = 99;
  s_savedata.sun_set_hour = 99;
  s_savedata.sun_set_min = 99;
  s_savedata.auto_daymode = true;
  s_savedata.date_format = 0;
  s_savedata.show_wind = false;
  s_savedata.wind_speed[0] = '\0';
  s_savedata.last_update = 0;
  s_savedata.forecast_hour = 18;
  s_savedata.forecast_min = 0;
  s_savedata.qt_start_hour = 0;
  s_savedata.qt_start_min = 15;
  s_savedata.qt_end_hour = 6;
  s_savedata.qt_end_min = 30;
  s_savedata.qt_bt_vibes = true;
  s_savedata.qt_fetch_weather = false;
  s_savedata.show_week = false;
  s_savedata.show_steps = false;
  s_savedata.weather_update_interval = 60; // Default to 60 minutes
  s_savedata.last_battery_percent = 0;
  s_savedata.last_is_charging = false;
  s_savedata.endpoint_configured = false;

  if (persist_exists(SAVE_VER_KEY)) {
    // Handle different save versions for migration
    if (persist_exists(SAVEDATA_KEY)) {
      switch (persist_read_int(SAVE_VER_KEY)) {
        case 4:
          persist_read_data(SAVEDATA_KEY, &s_savedata, sizeof(s_savedata) - (sizeof(s_savedata.show_week)+sizeof(s_savedata.show_steps)+sizeof(s_savedata.weather_update_interval)+sizeof(s_savedata.last_battery_percent)+sizeof(s_savedata.last_is_charging)+sizeof(s_savedata.endpoint_configured)));
          break;
        case 5:
          persist_read_data(SAVEDATA_KEY, &s_savedata, sizeof(s_savedata) - (sizeof(s_savedata.weather_update_interval)+sizeof(s_savedata.last_battery_percent)+sizeof(s_savedata.last_is_charging)+sizeof(s_savedata.endpoint_configured)));
          break;
        case 6:
          persist_read_data(SAVEDATA_KEY, &s_savedata, sizeof(s_savedata) - (sizeof(s_savedata.last_battery_percent)+sizeof(s_savedata.last_is_charging)+sizeof(s_savedata.endpoint_configured)));
          break;
        case 7:
        default:
          persist_read_data(SAVEDATA_KEY, &s_savedata, sizeof(s_savedata));
          break;
      }
    }
  }

  // Initialize static variables from persisted data
  last_battery_percent = s_savedata.last_battery_percent;
  last_is_charging = s_savedata.last_is_charging;
  endpoint_configured = s_savedata.endpoint_configured;

  // Defensively sanitize persisted data before it is used below. Torn/partial
  // persist writes (a known issue on some watches) can leave string fields with no
  // null terminator or numeric fields out of range. A non-terminated string is a
  // crash-on-open: strlen() inside the MyTupletCString() macros in initial_values[]
  // runs off the end of the field, producing a bogus tuple length that overflows
  // sync_buffer in app_sync_init(); the same string later faults in the text renderer.
  s_savedata.status[sizeof(s_savedata.status) - 1] = '\0';
  s_savedata.city[sizeof(s_savedata.city) - 1] = '\0';
  s_savedata.curr_temp[sizeof(s_savedata.curr_temp) - 1] = '\0';
  s_savedata.sun_rise_set[sizeof(s_savedata.sun_rise_set) - 1] = '\0';
  s_savedata.forecast_day[sizeof(s_savedata.forecast_day) - 1] = '\0';
  s_savedata.high_temp[sizeof(s_savedata.high_temp) - 1] = '\0';
  s_savedata.low_temp[sizeof(s_savedata.low_temp) - 1] = '\0';
  s_savedata.condition[sizeof(s_savedata.condition) - 1] = '\0';
  s_savedata.wind_speed[sizeof(s_savedata.wind_speed) - 1] = '\0';

  // icon indexes WEATHER_ICONS[] directly; an out-of-range value reads past the
  // array and hands a garbage resource id to gbitmap_create_with_resource().
  if (s_savedata.icon >= ARRAY_LENGTH(WEATHER_ICONS)) s_savedata.icon = 0;

  GRect bounds = layer_get_bounds(window_layer);
  
  // Setup 'current' layer (time, date, current temp, battery, bluetooth)
  current_layer = layer_create(LAYOUT_CURRENT_FRAME); 
  
  clock_layer = text_layer_create(LAYOUT_CLOCK_FRAME);
  text_layer_set_text_color(clock_layer, GColorWhite);
  text_layer_set_background_color(clock_layer, GColorClear);
#ifdef LAYOUT_CLOCK_CUSTOM_FONT
  clock_font = fonts_load_custom_font(resource_get_handle(LAYOUT_CLOCK_CUSTOM_FONT));
#else
  clock_font = fonts_get_system_font(LAYOUT_CLOCK_FONT);
#endif
  text_layer_set_font(clock_layer, clock_font);
  text_layer_set_text_alignment(clock_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(clock_layer, GTextOverflowModeFill);
  layer_add_child(current_layer, text_layer_get_layer(clock_layer));
  
  pm_layer = text_layer_create(LAYOUT_PM_FRAME);
  text_layer_set_text_color(pm_layer, GColorWhite);
  text_layer_set_background_color(pm_layer, GColorClear);
  text_layer_set_font(pm_layer, fonts_get_system_font(LAYOUT_PM_FONT));
  text_layer_set_text_alignment(pm_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(pm_layer, GTextOverflowModeFill);
  layer_add_child(current_layer, text_layer_get_layer(pm_layer));
  
  week_layer = text_layer_create(LAYOUT_WEEK_FRAME);
  text_layer_set_text_color(week_layer, GColorWhite);
  text_layer_set_background_color(week_layer, GColorClear);
  text_layer_set_font(week_layer, fonts_get_system_font(LAYOUT_WEEK_FONT));
  text_layer_set_text_alignment(week_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(week_layer, GTextOverflowModeFill);
  layer_add_child(current_layer, text_layer_get_layer(week_layer));
  
  date_layer = text_layer_create(LAYOUT_DATE_FRAME);
  text_layer_set_text_color(date_layer, GColorWhite);
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_font(date_layer, fonts_get_system_font(LAYOUT_DATE_FONT));
  text_layer_set_text_alignment(date_layer, LAYOUT_DATE_ALIGN);
  text_layer_set_overflow_mode(date_layer, GTextOverflowModeFill);
  layer_add_child(current_layer, text_layer_get_layer(date_layer));
  
  bt_layer = bitmap_layer_create(LAYOUT_BT_FRAME);
  layer_add_child(current_layer, bitmap_layer_get_layer(bt_layer));
  bitmap_layer_set_bitmap(bt_layer, bt_icon);
  bt_connected = bluetooth_connection_service_peek();
  update_bt_icon(bt_connected);
  
  batt_layer = bitmap_layer_create(LAYOUT_BATT_FRAME);
  layer_add_child(current_layer, bitmap_layer_get_layer(batt_layer));
  BatteryChargeState batt_state = battery_state_service_peek();
  layer_set_hidden(bitmap_layer_get_layer(batt_layer), s_savedata.show_batt);
  handle_batt_update(batt_state);
  battery_state_service_subscribe(handle_batt_update);
  
  curr_temp_layer = text_layer_create(LAYOUT_CURR_TEMP_FRAME);
  text_layer_set_text_color(curr_temp_layer, GColorWhite);
  text_layer_set_background_color(curr_temp_layer, GColorClear);
  text_layer_set_font(curr_temp_layer, fonts_get_system_font(LAYOUT_CURR_TEMP_FONT));
  text_layer_set_text_alignment(curr_temp_layer, LAYOUT_CURR_TEMP_ALIGN);
  text_layer_set_overflow_mode(curr_temp_layer, GTextOverflowModeFill);
  layer_add_child(current_layer, text_layer_get_layer(curr_temp_layer));
  
  wind_speed_layer = text_layer_create(LAYOUT_WIND_FRAME);
  text_layer_set_text_color(wind_speed_layer, GColorWhite);
  text_layer_set_background_color(wind_speed_layer, GColorClear);
  text_layer_set_font(wind_speed_layer, fonts_get_system_font(LAYOUT_WIND_FONT));
  text_layer_set_text_alignment(wind_speed_layer, LAYOUT_WIND_ALIGN);
  text_layer_set_overflow_mode(wind_speed_layer, GTextOverflowModeFill);
  layer_set_hidden(text_layer_get_layer(wind_speed_layer), !s_savedata.show_wind);
#if LAYOUT_SHOW_WIND_LAYER
  // Only added where the design has room for it (not on the round displays)
  layer_add_child(current_layer, text_layer_get_layer(wind_speed_layer));
#endif
  
  layer_add_child(window_layer, current_layer);
  
  // Setup forecast layer (High/Low Temp, conditions, sunrise/sunset)
  forecast_layer = layer_create(LAYOUT_FORECAST_FRAME);

  status_bg_layer = text_layer_create(LAYOUT_STATUS_BG_FRAME);
  text_layer_set_background_color(status_bg_layer, GColorWhite);
  layer_add_child(forecast_layer, text_layer_get_layer(status_bg_layer));
  
#if (defined(PBL_HEALTH) && defined(PBL_COLOR))
  steps_layer = layer_create(LAYOUT_STEPS_FRAME);
  layer_add_child(forecast_layer, steps_layer);
  layer_set_update_proc(steps_layer, draw_steps_progress);
#endif
  
  forecast_day_layer = text_layer_create(LAYOUT_FORECAST_DAY_FRAME);
  text_layer_set_text_color(forecast_day_layer, GColorBlack);
  text_layer_set_background_color(forecast_day_layer, GColorClear);
  text_layer_set_font(forecast_day_layer, fonts_get_system_font(LAYOUT_FORECAST_DAY_FONT));
  text_layer_set_text_alignment(forecast_day_layer, GTextAlignmentLeft);
  text_layer_set_overflow_mode(forecast_day_layer, GTextOverflowModeFill);
  layer_add_child(forecast_layer, text_layer_get_layer(forecast_day_layer));
  
  status_layer = text_layer_create(LAYOUT_STATUS_FRAME);
  text_layer_set_text_color(status_layer, GColorBlack);
  text_layer_set_background_color(status_layer, GColorClear);
  text_layer_set_font(status_layer, fonts_get_system_font(LAYOUT_STATUS_FONT));
  text_layer_set_text_alignment(status_layer, GTextAlignmentRight);
  text_layer_set_overflow_mode(status_layer, GTextOverflowModeTrailingEllipsis);
  layer_add_child(forecast_layer, text_layer_get_layer(status_layer));
  
  high_label_layer = text_layer_create(LAYOUT_HIGH_LABEL_FRAME);
  text_layer_set_text_color(high_label_layer, GColorWhite);
  text_layer_set_background_color(high_label_layer, GColorClear);
  text_layer_set_font(high_label_layer, fonts_get_system_font(LAYOUT_TEMP_FONT));
  text_layer_set_text_alignment(high_label_layer, GTextAlignmentLeft);
  text_layer_set_overflow_mode(high_label_layer, GTextOverflowModeFill);
  text_layer_set_text(high_label_layer, "H");
  layer_set_hidden(text_layer_get_layer(high_label_layer), true);
  layer_add_child(forecast_layer, text_layer_get_layer(high_label_layer));
  
  high_temp_layer = text_layer_create(LAYOUT_HIGH_TEMP_FRAME);
  text_layer_set_text_color(high_temp_layer, GColorWhite);
  text_layer_set_background_color(high_temp_layer, GColorClear);
  text_layer_set_font(high_temp_layer, fonts_get_system_font(LAYOUT_TEMP_FONT));
  text_layer_set_text_alignment(high_temp_layer, GTextAlignmentRight);
  text_layer_set_overflow_mode(high_temp_layer, GTextOverflowModeFill);
  layer_add_child(forecast_layer, text_layer_get_layer(high_temp_layer));
  
  low_label_layer = text_layer_create(LAYOUT_LOW_LABEL_FRAME);
  text_layer_set_text_color(low_label_layer, GColorWhite);
  text_layer_set_background_color(low_label_layer, GColorClear);
  text_layer_set_font(low_label_layer, fonts_get_system_font(LAYOUT_TEMP_FONT));
  text_layer_set_text_alignment(low_label_layer, GTextAlignmentLeft);
  text_layer_set_overflow_mode(low_label_layer, GTextOverflowModeFill);
  text_layer_set_text(low_label_layer, "L");
  layer_set_hidden(text_layer_get_layer(low_label_layer), true);
  layer_add_child(forecast_layer, text_layer_get_layer(low_label_layer));
  
  low_temp_layer = text_layer_create(LAYOUT_LOW_TEMP_FRAME);
  text_layer_set_text_color(low_temp_layer, GColorWhite);
  text_layer_set_background_color(low_temp_layer, GColorClear);
  text_layer_set_font(low_temp_layer, fonts_get_system_font(LAYOUT_TEMP_FONT));
  text_layer_set_text_alignment(low_temp_layer, GTextAlignmentRight);
  text_layer_set_overflow_mode(low_temp_layer, GTextOverflowModeFill);
  layer_add_child(forecast_layer, text_layer_get_layer(low_temp_layer));
  
  sun_rise_set_layer = text_layer_create(LAYOUT_SUN_TEXT_FRAME);
  text_layer_set_text_color(sun_rise_set_layer, GColorWhite);
  text_layer_set_background_color(sun_rise_set_layer, GColorClear);
  text_layer_set_font(sun_rise_set_layer, fonts_get_system_font(LAYOUT_SUN_TEXT_FONT));
  text_layer_set_text_alignment(sun_rise_set_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(sun_rise_set_layer, GTextOverflowModeFill);
  layer_add_child(forecast_layer, text_layer_get_layer(sun_rise_set_layer));
  
  icon_layer = bitmap_layer_create(LAYOUT_ICON_FRAME);
  bitmap_layer_set_alignment(icon_layer, GAlignCenter);
  bitmap_layer_set_background_color(icon_layer, GColorBlack);
  layer_add_child(forecast_layer, bitmap_layer_get_layer(icon_layer));
  
  sun_layer = bitmap_layer_create(LAYOUT_SUN_ICON_FRAME);
  layer_add_child(forecast_layer, bitmap_layer_get_layer(sun_layer));
  
  condition_layer = text_layer_create(LAYOUT_CONDITION_FRAME);
  text_layer_set_text_color(condition_layer, GColorWhite);
  text_layer_set_background_color(condition_layer, GColorClear);
  text_layer_set_font(condition_layer, fonts_get_system_font(LAYOUT_CONDITION_FONT));
  text_layer_set_text_alignment(condition_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(condition_layer, GTextOverflowModeTrailingEllipsis);
  layer_add_child(forecast_layer, text_layer_get_layer(condition_layer));
  
  layer_add_child(window_layer, forecast_layer);
  
#ifdef PBL_ROUND
  text_layer_enable_screen_text_flow_and_paging(forecast_day_layer, 1);
  text_layer_enable_screen_text_flow_and_paging(status_layer, 1);
  text_layer_enable_screen_text_flow_and_paging(condition_layer, 1);
#endif
  
  // Setup 3 week calendar layer
  cal_layer = layer_create(LAYOUT_CAL_FRAME);
  layer_add_child(window_layer, cal_layer);
  
  layer_set_update_proc(cal_layer, cal_layer_draw);
  
  daymode_layer = effect_layer_create(bounds);
  effect_layer_add_effect(daymode_layer, effect_invert_bw_only, NULL);
  
  brightness_inverter = effect_layer_create(layer_get_bounds(bitmap_layer_get_layer(bt_layer)));
  effect_layer_add_effect(brightness_inverter, effect_invert_brightness, NULL);
  set_daymode(s_savedata.daymode);
  
  layer_add_child(window_layer, effect_layer_get_layer(daymode_layer));
  layer_add_child(window_layer, effect_layer_get_layer(brightness_inverter));
  
  // Get current time
  struct tm *t;
  time_t temp;
  temp = time(NULL);
  t = localtime(&temp);
  
  // Init time and date
  handle_tick(t, MINUTE_UNIT | HOUR_UNIT | DAY_UNIT);

  // Use configured interval (in minutes), default to 60 if not set
  uint16_t update_interval_minutes = (s_savedata.weather_update_interval > 0) ? s_savedata.weather_update_interval : 60;
  time_t update_interval_seconds = (time_t)(update_interval_minutes * 60);

  // If it has been longer than the configured interval since last update or last update is not known
  // or we're not showing the correct forecast day then we need to run an update
  bool need_update = (s_savedata.last_update == 0 || ((temp - s_savedata.last_update) >= update_interval_seconds) ||
                     strcmp(s_savedata.forecast_day, show_forecast_tomorrow() ? "Tomorrow" : "Today") != 0);
  
  // Initialize weather data UI
  // Note: WIND_SPEED_KEY is NOT included in initial_values - it will be sent by JS when weather is fetched
  // Including it here causes a crash on restart (possibly due to buffer/sync issues)
  Tuplet initial_values[] = {
    MyTupletCString(WEATHER_STATUS_KEY, s_savedata.status),
    MyTupletCString(WEATHER_CURR_TEMP_KEY, s_savedata.curr_temp),
    MyTupletCString(WEATHER_FORECAST_DAY_KEY, s_savedata.forecast_day),
    MyTupletCString(WEATHER_HIGH_TEMP_KEY, s_savedata.high_temp),
    MyTupletCString(WEATHER_LOW_TEMP_KEY, s_savedata.low_temp),
    TupletInteger(WEATHER_ICON_KEY, s_savedata.icon),
    MyTupletCString(WEATHER_CONDITION_KEY, s_savedata.condition),
    TupletInteger(WEATHER_DAYMODE_KEY, s_savedata.daymode ? 1 : 0),
    TupletCString(WEATHER_CITY_KEY, (need_update ? "Fetching..." : s_savedata.city)),
    TupletInteger(WEATHER_SUN_RISE_HOUR_KEY, s_savedata.sun_rise_hour),
    TupletInteger(WEATHER_SUN_RISE_MIN_KEY, s_savedata.sun_rise_min),
    TupletInteger(WEATHER_SUN_SET_HOUR_KEY, s_savedata.sun_set_hour),
    TupletInteger(WEATHER_SUN_SET_MIN_KEY, s_savedata.sun_set_min),
    TupletInteger(WEATHER_AUTO_DAYMODE_KEY, s_savedata.auto_daymode ? 1 : 0),
    TupletInteger(CAL_FIRST_DAY_KEY, s_savedata.startday),
    TupletInteger(CAL_OFFSET_KEY, s_savedata.cal_offset),
    TupletInteger(SHOW_BT_KEY, s_savedata.show_bt ? 1 : 0),
    TupletInteger(BT_VIBES_KEY, s_savedata.bt_vibes ? 1 : 0),
    TupletInteger(SHOW_BATT_KEY, s_savedata.show_batt ? 1 : 0),
    TupletInteger(SHOW_WEEK_KEY, s_savedata.show_week ? 1 : 0),
    TupletInteger(SHOW_STEPS_KEY, s_savedata.show_steps ? 1 : 0),
    TupletInteger(TIME_24HR_KEY, clock_is_24h_style() ? 1 : 0),
    TupletInteger(LOC_CHANGED_KEY, 0),
    TupletInteger(DATE_FORMAT_KEY, s_savedata.date_format),
    TupletInteger(SHOW_WIND_KEY, s_savedata.show_wind ? 1 : 0),
    // WIND_SPEED_KEY removed - causes crash on restart
    TupletInteger(FORECAST_HOUR_KEY, s_savedata.forecast_hour),
    TupletInteger(FORECAST_MIN_KEY, s_savedata.forecast_min),
    TupletInteger(QT_START_HOUR_KEY, s_savedata.qt_start_hour),
    TupletInteger(QT_START_MIN_KEY, s_savedata.qt_start_min),
    TupletInteger(QT_END_HOUR_KEY, s_savedata.qt_end_hour),
    TupletInteger(QT_END_MIN_KEY, s_savedata.qt_end_min),
    TupletInteger(QT_BT_VIBES_KEY, s_savedata.qt_bt_vibes),
    TupletInteger(QT_FETCH_WEATHER_KEY, s_savedata.qt_fetch_weather),
    TupletInteger(WEATHER_UPDATE_INTERVAL_KEY, s_savedata.weather_update_interval),
    TupletInteger(WEATHER_FETCHED_KEY, 0),
    TupletInteger(REQUEST_BATTERY_KEY, 0),
    TupletInteger(BATTERY_PERCENT_KEY, s_savedata.last_battery_percent),
    TupletInteger(ENDPOINT_CONFIGURED_KEY, s_savedata.endpoint_configured ? 1 : 0),
    TupletInteger(IS_CHARGING_KEY, s_savedata.last_is_charging ? 1 : 0)
  };
  
  // Initialize comms with phone
  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);
  
  if (need_update) {
    last_update_attempt = temp - (temp % 60);
    s_savedata.last_update = last_update_attempt;
    
    // Fire timer 2 seconds after the app_sync_init to mark everything 
    // as loaded once the initial tuple callback is done and init the first real weather update
    weatherinit_timer = app_timer_register(2000, handle_weatherinit_timer, NULL);
  } else {
    loading = false;
  }
}

static void window_unload(Window *window) {
  // Save last weather data is we got past the initial loading stage
  if (!loading) {
    persist_write_int(SAVE_VER_KEY, SAVE_VER);
    persist_write_data(SAVEDATA_KEY, &s_savedata, sizeof(s_savedata));
  }
  
  // Release event based resources
  app_sync_deinit(&sync);
  
  if (bt_timer)
    app_timer_cancel(bt_timer);
  
  if (status_timer)
    app_timer_cancel(status_timer);
  
  if (weatherinit_timer)
    app_timer_cancel(weatherinit_timer);

  battery_state_service_unsubscribe();
  
  // Release image resources
  if (icon_bitmap)
    gbitmap_destroy(icon_bitmap);
  
  if (sun_bitmap)
    gbitmap_destroy(sun_bitmap);
  
  if (batt_icon)
    gbitmap_destroy(batt_icon);
  
  // Release UI resources
  text_layer_destroy(clock_layer);
#ifdef LAYOUT_CLOCK_CUSTOM_FONT
  // Unloaded after its text layer, so nothing can still reference it.
  if (clock_font) {
    fonts_unload_custom_font(clock_font);
    clock_font = NULL;
  }
#endif
  text_layer_destroy(date_layer);
  text_layer_destroy(pm_layer);
  text_layer_destroy(curr_temp_layer);
  text_layer_destroy(wind_speed_layer);
  text_layer_destroy(sun_rise_set_layer);
  bitmap_layer_destroy(bt_layer);
  bitmap_layer_destroy(batt_layer);
  layer_destroy(current_layer);
  
#if (defined(PBL_HEALTH) && defined(PBL_COLOR))
  if (steps_on) health_service_events_unsubscribe();
  layer_destroy(steps_layer);
#endif
  
  text_layer_destroy(forecast_day_layer);
  text_layer_destroy(status_layer);
  text_layer_destroy(high_label_layer);
  text_layer_destroy(high_temp_layer);
  text_layer_destroy(low_label_layer);
  text_layer_destroy(low_temp_layer);
  text_layer_destroy(condition_layer);
  bitmap_layer_destroy(icon_layer);
  bitmap_layer_destroy(sun_layer);
  text_layer_destroy(status_bg_layer);
  layer_destroy(forecast_layer);
  
  layer_destroy(cal_layer);
  
  effect_layer_destroy(daymode_layer);
  effect_layer_destroy(brightness_inverter);
}

static void init(void) {
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });

  tick_timer_service_subscribe(MINUTE_UNIT | HOUR_UNIT | DAY_UNIT, handle_tick);
  
  bt_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT);
  bluetooth_connection_service_subscribe(handle_bt_update);
  
  app_message_open(inbound_size, outbound_size);

  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  if (bt_icon) {
    gbitmap_destroy(bt_icon);
  }
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
