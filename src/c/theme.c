#include <pebble.h>
#include "theme.h"

#define PERSIST_KEY_THEME_DARK 7201

static bool s_dark = false;

void theme_load(void) {
  if (persist_exists(PERSIST_KEY_THEME_DARK)) {
    s_dark = persist_read_bool(PERSIST_KEY_THEME_DARK);
  }
}

bool theme_is_dark(void) {
  return s_dark;
}

bool theme_set_dark(bool dark) {
  if (dark == s_dark) return false;

  s_dark = dark;
  persist_write_bool(PERSIST_KEY_THEME_DARK, dark);

  return true;
}

GColor theme_bg(void) {
  return s_dark ? GColorBlack : GColorWhite;
}

GColor theme_fg(void) {
  return s_dark ? GColorWhite : GColorBlack;
}

GColor theme_fg_dim(void) {
  return s_dark ? GColorLightGray : GColorDarkGray;
}

static int prv_palette_size(GBitmapFormat format) {
  switch (format) {
    case GBitmapFormat1BitPalette: return 2;
    case GBitmapFormat2BitPalette: return 4;
    case GBitmapFormat4BitPalette: return 16;
    default: return 0;
  }
}

// A GColor packs two bits each of alpha, red, green and blue into one byte.
// Keeping the top two and replacing the rest recolors a pixel without touching
// how opaque it is.
#define ALPHA_BITS 0xC0
#define COLOR_BITS 0x3F

// The icons are flat silhouettes plus an alpha channel, so recoloring one only
// means replacing each pixel's color and keeping its alpha. A palettized bitmap
// needs nothing but its palette rewritten; an 8-bit one needs a pass over the
// pixels. Anything else -- a plain 1-bit bitmap, say -- has no color to change,
// and is left as the resource was compiled.
void theme_tint_bitmap(GBitmap *bitmap) {
  if (!bitmap) return;

  const GColor fg = theme_fg();
  const GBitmapFormat format = gbitmap_get_format(bitmap);
  const int palette_size = prv_palette_size(format);

  if (palette_size > 0) {
    GColor *palette = gbitmap_get_palette(bitmap);
    if (!palette) return;

    for (int i = 0; i < palette_size; i++) {
      palette[i].argb = (palette[i].argb & ALPHA_BITS) | (fg.argb & COLOR_BITS);
    }

    return;
  }

  if (format == GBitmapFormat8Bit) {
    uint8_t *data = gbitmap_get_data(bitmap);
    if (!data) return;

    const GRect bounds = gbitmap_get_bounds(bitmap);
    const uint16_t stride = gbitmap_get_bytes_per_row(bitmap);

    for (int y = 0; y < bounds.size.h; y++) {
      uint8_t *row = &data[y * stride];

      for (int x = 0; x < bounds.size.w; x++) {
        row[x] = (row[x] & ALPHA_BITS) | (fg.argb & COLOR_BITS);
      }
    }
  }
}
