#pragma once

#include <pebble.h>

// The whole watchface is two colors: a background, and a foreground for the
// time, date, graph and icons. There is one shade in between, used for the
// minimum-precipitation bars. Which way round the two go is a setting, chosen
// on the config page and remembered across launches.

void theme_load(void);

bool theme_is_dark(void);

// Returns true only if the setting actually changed, so the caller knows
// whether there is anything to redraw.
bool theme_set_dark(bool dark);

GColor theme_bg(void);
GColor theme_fg(void);
GColor theme_fg_dim(void);

// Recolors a flat icon to the current foreground, leaving its alpha alone.
// Safe to call repeatedly on the same bitmap, and with NULL.
void theme_tint_bitmap(GBitmap *bitmap);
