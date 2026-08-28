#!/usr/bin/env python3
"""Recolor the watchface icons to a single flat color, keeping their alpha.

Every icon in resources/images is a flat THEME_FG silhouette with an alpha
channel, composited onto the background with GCompOpSet. Flipping the face
between dark and light means recoloring them, or they end up invisible against
their own background.

    python3 tools/recolor_icons.py 000000     # icons for a light face
    python3 tools/recolor_icons.py ffffff     # icons for a dark face

Only the images listed in package.json are touched. Alpha is left alone, so
antialiased edges keep their softness.
"""

import json
import os
import struct
import sys
import zlib

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def read_rgba(path):
    """Decode an 8-bit RGBA PNG into (width, height, pixel bytes)."""
    data = open(path, "rb").read()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError("{}: not a PNG".format(path))

    pos, idat, width, height = 8, b"", None, None
    while pos < len(data):
        (length,), kind = struct.unpack(">I", data[pos:pos + 4]), data[pos + 4:pos + 8]
        pos += 8
        chunk = data[pos:pos + length]
        pos += length + 4

        if kind == b"IHDR":
            width, height, depth, color, _, _, interlace = struct.unpack(">IIBBBBB", chunk)
            if (depth, color, interlace) != (8, 6, 0):
                raise ValueError("{}: expected non-interlaced 8-bit RGBA".format(path))
        elif kind == b"IDAT":
            idat += chunk

    raw = zlib.decompress(idat)
    stride = width * 4
    out, prev, pos = bytearray(), bytearray(stride), 0

    for _ in range(height):
        method = raw[pos]
        pos += 1
        line = bytearray(raw[pos:pos + stride])
        pos += stride

        for x in range(stride):
            left = line[x - 4] if x >= 4 else 0
            up = prev[x]
            upleft = prev[x - 4] if x >= 4 else 0

            if method == 1:
                line[x] = (line[x] + left) & 0xFF
            elif method == 2:
                line[x] = (line[x] + up) & 0xFF
            elif method == 3:
                line[x] = (line[x] + (left + up) // 2) & 0xFF
            elif method == 4:
                estimate = left + up - upleft
                da, db, dc = (abs(estimate - left), abs(estimate - up), abs(estimate - upleft))
                if da <= db and da <= dc:
                    line[x] = (line[x] + left) & 0xFF
                elif db <= dc:
                    line[x] = (line[x] + up) & 0xFF
                else:
                    line[x] = (line[x] + upleft) & 0xFF
            elif method != 0:
                raise ValueError("{}: unknown filter {}".format(path, method))

        out += line
        prev = line

    return width, height, out


def write_rgba(path, width, height, pixels):
    def chunk(kind, payload):
        return (struct.pack(">I", len(payload)) + kind + payload
                + struct.pack(">I", zlib.crc32(kind + payload) & 0xFFFFFFFF))

    stride = width * 4
    raw = bytearray()
    for y in range(height):
        raw += b"\x00" + pixels[y * stride:(y + 1) * stride]

    png = (b"\x89PNG\r\n\x1a\n"
           + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0))
           + chunk(b"IDAT", zlib.compress(bytes(raw), 9))
           + chunk(b"IEND", b""))
    open(path, "wb").write(png)


def recolor(path, rgb):
    width, height, pixels = read_rgba(path)
    for i in range(0, len(pixels), 4):
        if pixels[i + 3]:
            pixels[i:i + 3] = rgb
    write_rgba(path, width, height, pixels)


def main():
    if len(sys.argv) != 2 or len(sys.argv[1].lstrip("#")) != 6:
        sys.exit("usage: recolor_icons.py RRGGBB")

    value = sys.argv[1].lstrip("#")
    rgb = bytes(int(value[i:i + 2], 16) for i in (0, 2, 4))

    package = json.load(open(os.path.join(ROOT, "package.json")))
    for media in package["pebble"]["resources"]["media"]:
        if media["type"] != "bitmap":
            continue
        recolor(os.path.join(ROOT, "resources", media["file"]), rgb)
        print("recolored {}".format(media["file"]))


if __name__ == "__main__":
    main()
