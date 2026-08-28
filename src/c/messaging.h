#pragma once
#include <pebble.h>

// Called when the config page hands back a theme the watchface was not already
// using.
typedef void (*ThemeChangedCallback)(void);

// Open AppMessage and register an internal inbox handler.
void messaging_open(ThemeChangedCallback on_theme_changed);

// Request a weather refresh via AppMessage
void messaging_request_weather_refresh(void);
