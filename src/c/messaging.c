#include <pebble.h>
#include "messaging.h"
#include "weather.h"
#include "weather_graph.h"
#include "theme.h"

static ThemeChangedCallback s_on_theme_changed = NULL;

// Clay sends a toggle as an integer, but not always a four-byte one, so read it
// at whatever width it arrived in.
static bool prv_tuple_is_true(const Tuple *tuple) {
  if (tuple->type != TUPLE_INT && tuple->type != TUPLE_UINT) return false;

  switch (tuple->length) {
    case 1:  return tuple->value->uint8 != 0;
    case 2:  return tuple->value->uint16 != 0;
    default: return tuple->value->uint32 != 0;
  }
}

static void prv_inbox_received(DictionaryIterator *iter, void *context) {
  (void)context;

  Tuple *theme_tuple = dict_find(iter, MESSAGE_KEY_THEME_DARK);
  if (theme_tuple && theme_set_dark(prv_tuple_is_true(theme_tuple)) && s_on_theme_changed) {
    s_on_theme_changed();
  }

  weather_inbox_parse(iter);

  Tuple *forecast_tuple = dict_find(iter, MESSAGE_KEY_WEATHER_FORECAST);
  if (forecast_tuple && forecast_tuple->type == TUPLE_CSTRING) {
    weather_graph_parse_forecast(forecast_tuple->value->cstring);
  }
}

void messaging_open(ThemeChangedCallback on_theme_changed) {
  s_on_theme_changed = on_theme_changed;

  app_message_register_inbox_received(prv_inbox_received);
  app_message_open(256, 256);
}

void messaging_request_weather_refresh(void) {
  DictionaryIterator *iter;
  AppMessageResult res = app_message_outbox_begin(&iter);

  if (res != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Outbox begin failed: %d", res);
    return;
  }

  dict_write_uint8(iter, MESSAGE_KEY_WEATHER_REFRESH, 1);
  dict_write_end(iter);
  app_message_outbox_send();
}
