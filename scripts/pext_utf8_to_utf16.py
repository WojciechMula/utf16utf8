#!/usr/bin/env python3

import sys
from pathlib import Path


def encode_first_byte(b):
    assert 0 <= b <= 0xff
    if (b & 0x80) == 0:
        # 1 byte
        return (0x7f, 1)

    if (b & 0b11100000) == 0b11000000:
        # 2 bytes code
        return (0b00011111001111110000000000000000, 2)

    if (b & 0b11110000) == 0b11100000:
        # 3 bytes code
        return (0b00001111001111110011111100000000, 3)

    if (b & 0b11111000) == 0b11110000:
        # 3 bytes code
        return (0b00001111001111110011111100111111, 4)

    return (0, 0)


CPP_CODE = """
#include <cstdint>
#include <cstddef>

struct pext_lookup_type {
    uint32_t    mask;
    uint8_t     utf8length;
    uint8_t     padding[2];

};

static_assert(sizeof(pext_lookup_type) == 8, "Lookup entry has to occupy 8 bytes");

pext_lookup_type pext_lookup[256] = {
%s
};
"""

ENTRY = "  {{0x{mask:08x}, {length}, 0, 0}}"


def main():
    path = Path(sys.argv[1])

    entries = []
    for b in range(256):
        mask, length = encode_first_byte(b)
        entries.append(ENTRY.format(mask=mask, length=length))

    path.write_text(CPP_CODE % (',\n'.join(entries)))


if __name__ == '__main__':
    main()
