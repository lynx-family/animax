#!/usr/bin/env python3
# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.
# cspell:ignore IIIIHHQ Qhhhh HHHHI maxp subtable glyf loca

"""Regenerate font_test_data.h using only Python's standard library.

The two original test fonts map printable ASCII to a space or a rectangle.
Their different advances make stale text layout observable without relying on
installed fonts, downloaded assets, or third-party font licenses.
"""

from pathlib import Path
import struct


def pack(fmt, *values):
    return struct.pack(">" + fmt, *values)


def checksum(data):
    padded = data + bytes(-len(data) % 4)
    return sum(struct.unpack(">" + "I" * (len(padded) // 4), padded)) & 0xFFFFFFFF


def make_font(advance, family):
    rectangle = (pack("hhhhhHH", 1, 40, 0, advance - 40, 700, 3, 0)
                 + bytes([1] * 4)
                 + pack("hhhhhhhh", 40, 0, advance - 80, 0, 0, 700, 0, -700))
    head = pack("IIIIHHQQhhhhHHhhh", 0x10000, 0x10000, 0, 0x5F0F3CF5,
                3, 1000, 0, 0, 40, 0, advance - 40, 700, 0, 8, 2, 0, 0)
    hhea = pack("I16h", 0x10000, 800, -200, 0, advance, 0, 40,
                advance - 40, 1, 0, 0, 0, 0, 0, 0, 0, 3)
    maxp = pack("I14H", 0x10000, 3, 4, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0)
    cmap_subtable = (pack("7H", 4, 222, 0, 4, 4, 1, 0)
                     + pack("9H", 126, 0xFFFF, 0, 32, 0xFFFF, 0, 1, 4, 0)
                     + pack("95H", 1, *([2] * 94)))
    cmap = pack("HHHHI", 0, 1, 3, 1, 12) + cmap_subtable
    names = {1: family, 2: "Regular", 4: family, 6: family}
    records, strings = b"", b""
    for name_id, value in names.items():
        encoded = value.encode("utf-16-be")
        records += pack("6H", 3, 1, 0x409, name_id, len(encoded), len(strings))
        strings += encoded
    name = pack("3H", 0, len(names), 6 + len(records)) + records + strings
    os2 = bytearray(78)
    struct.pack_into(">HhHH", os2, 0, 0, advance, 400, 5)
    struct.pack_into(">hhhhhhhhhh", os2, 10,
                     650, 650, 0, 0, 650, 650, 0, 350, 50, 300)
    struct.pack_into(">I", os2, 42, 1)
    os2[58:62] = b"LYNX"
    struct.pack_into(">HHHhhhHH", os2, 62,
                     0x40, 32, 126, 800, -200, 0, 800, 200)
    tables = {
        "OS/2": bytes(os2), "cmap": cmap, "glyf": rectangle * 2,
        "head": head, "hhea": hhea,
        "hmtx": pack("HhHhHh", advance, 40, advance // 2, 0, advance, 40),
        "loca": pack("4H", 0, len(rectangle) // 2, len(rectangle) // 2,
                     len(rectangle)),
        "maxp": maxp, "name": name,
        "post": pack("IIhhIIIII", 0x30000, 0, -100, 50, 0, 0, 0, 0, 0),
    }
    header = pack("I4H", 0x10000, len(tables), 128, 3, 32)
    directory, contents = b"", b""
    head_offset = 0
    for tag, data in sorted(tables.items()):
        offset = len(header) + 16 * len(tables) + len(contents)
        directory += tag.encode("ascii") + pack("III", checksum(data), offset, len(data))
        if tag == "head":
            head_offset = offset
        contents += data + bytes(-len(data) % 4)
    result = bytearray(header + directory + contents)
    struct.pack_into(">I", result, head_offset + 8,
                     (0xB1B0AFBA - checksum(result)) & 0xFFFFFFFF)
    return result


def byte_array(name, data):
    lines = ["  static constexpr uint8_t " + name + "[] = {"]
    for i in range(0, len(data), 12):
        lines.append("      " + ", ".join(f"0x{b:02x}" for b in data[i:i + 12]) + ",")
    return "\n".join(lines + ["  };"])


def main():
    narrow = byte_array("kNarrow", make_font(400, "AnimaXTestNarrow"))
    wide = byte_array("kWide", make_font(700, "AnimaXTestWide"))
    header = """// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_TESTING_FONT_TEST_DATA_H_
#define ANIMAX_SRC_TESTING_FONT_TEST_DATA_H_

#include <cstdint>
#include <memory>
#include <vector>

#include "include/resource/raw_data.h"

namespace lynx {
namespace animax {

// Original rectangle fonts, covering printable ASCII with different advances.
// Regenerate the embedded bytes with generate_font_test_data.py.
enum class TestFontVariant { kNarrow, kWide };

inline std::vector<uint8_t> MakeTestFontBytes(
    TestFontVariant variant = TestFontVariant::kNarrow) {
""" + narrow + "\n" + wide + """
  if (variant == TestFontVariant::kWide) {
    return {kWide, kWide + sizeof(kWide)};
  }
  return {kNarrow, kNarrow + sizeof(kNarrow)};
}

inline std::unique_ptr<RawData> MakeTestFontData(
    TestFontVariant variant = TestFontVariant::kNarrow) {
  auto* bytes = new std::vector<uint8_t>(MakeTestFontBytes(variant));
  return RawData::MakeRawData(
      bytes->data(), bytes->size(),
      [](const void* context) {
        delete static_cast<const std::vector<uint8_t>*>(context);
      },
      bytes);
}

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_TESTING_FONT_TEST_DATA_H_
"""
    Path(__file__).with_name("font_test_data.h").write_text(header)


if __name__ == "__main__":
    main()
