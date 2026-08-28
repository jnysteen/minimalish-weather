#pragma once

#include <pebble.h>

// The whole watchface is two colors: THEME_BG behind everything, THEME_FG for
// the time, date, graph and icons. THEME_FG_DIM is the one shade in between,
// used for the minimum-precipitation bars.
//
// Swapping BG and FG flips the face between dark and light. The icon PNGs in
// resources/images are flat THEME_FG pixels plus an alpha channel, so they have
// to be recolored to match -- see tools/recolor_icons.py.
#define THEME_BG      GColorWhite
#define THEME_FG      GColorBlack
#define THEME_FG_DIM  GColorDarkGray
