#!/usr/bin/env python3
"""Generate packaging/icon.png — a simple 256x256 LightTouch placeholder icon.

Pure stdlib (no Pillow): dark rounded square + concentric "touch" rings.
Replace with real branding artwork when available.
"""
import math
import struct
import zlib

W = H = 256


def rounded_rect_mask(x, y, r):
    """True if (x, y) is inside a rounded rect inset by r on all sides."""
    x0 = y0 = r
    x1 = y1 = W - r
    cx = min(max(x, x0 + r), x1 - r)
    cy = min(max(y, y0 + r), y1 - r)
    return (x - cx) ** 2 + (y - cy) ** 2 <= r * r


def main():
    px = bytearray()
    for y in range(H):
        px.append(0)  # filter byte
        for x in range(W):
            if not rounded_rect_mask(x, y, 56):
                px += b"\x00\x00\x00\x00"  # transparent
                continue
            # vertical gradient: #232c44 -> #141a2b
            t = y / (H - 1)
            br, bg, bb = (0x23, 0x2C, 0x44)
            dr, dg, db = (0x14, 0x1A, 0x2B)
            r = int(br + (dr - br) * t)
            g = int(bg + (dg - bg) * t)
            b = int(bb + (db - bb) * t)
            # concentric touch rings around center
            cx = cy = 128
            d = math.hypot(x - cx, y - cy)
            ring = None
            for rad, wdt, col in (
                (74, 10, (0x7C, 0xC4, 0xFF)),
                (48, 10, (0x7C, 0xC4, 0xFF)),
                (22, 0, (0xE8, 0xF4, 0xFF)),  # filled dot
            ):
                if wdt == 0:
                    if d <= rad:
                        ring = col
                        break
                elif abs(d - rad) <= wdt / 2:
                    ring = col
                    break
            if ring:
                r, g, b = ring
            px += bytes((r, g, b, 0xFF))

    def chunk(typ, data):
        c = struct.pack(">I", len(data)) + typ + data
        c += struct.pack(">I", zlib.crc32(typ + data) & 0xFFFFFFFF)
        return c

    ihdr = struct.pack(">IIBBBBB", W, H, 8, 6, 0, 0, 0)
    png = (
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", ihdr)
        + chunk(b"IDAT", zlib.compress(bytes(px), 9))
        + chunk(b"IEND", b"")
    )
    with open("icon.png", "wb") as f:
        f.write(png)
    print(f"wrote icon.png ({len(png)} bytes)")


if __name__ == "__main__":
    main()
