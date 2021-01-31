#!/usr/bin/env python3

import sys
from pathlib import Path


def encode_first_byte(b):
    assert 0 <= b <= 0xff
    if (b & 0x80) == 0:
        # 1 byte
        return (0x7f, 1, 0x7f, 0x7f)

    if (b & 0b11100000) == 0b11000000:
        # 2 bytes code
        return (0b00011111001111110000000000000000,
                0b11100000110000000000000000000000,
                0b11000000100000000000000000000000,
                2)

    if (b & 0b11110000) == 0b11100000:
        # 3 bytes code
        return (0b00001111001111110011111100000000,
                0b11110000110000001100000000000000,
                0b11100000100000001000000000000000,
                3)

    if (b & 0b11111000) == 0b11110000:
        # 4 bytes code
        return (0b00001111001111110011111100111111,
                0b11110000110000001100000011000000,
                0b11100000100000001000000010000000,
                4)

    return (0, 0, 0, 0)


CPP_CODE = """
#include <cstdint>
#include <cstddef>

struct pext_lookup_type {
    uint32_t    pext_mask;
    uint32_t    utf8_bits_mask;
    uint32_t    utf8_bits_expected;
    uint8_t     utf8_length;
    uint8_t     padding[3];

};

static_assert(sizeof(pext_lookup_type) == 16, "Lookup entry has to occupy 16 bytes");

pext_lookup_type pext_lookup[256] = {
%s
};
"""

ENTRY = "  {{0x{pext_mask:08x}, 0x{utf8_bits_mask:08x}, 0x{utf8_bits_expected:08x}, {utf8_length}, 0, 0, 0}}"


def main():
    path = Path(sys.argv[1])

    entries = []
    for b in range(256):
        entry = encode_first_byte(b)
        d = {
            'pext_mask': entry[0],
            'utf8_bits_mask': entry[1],
            'utf8_bits_expected': entry[2],
            'utf8_length': entry[3],
        }

        entries.append(ENTRY.format(**d))

    path.write_text(CPP_CODE % (',\n'.join(entries)))


if __name__ == '__main__':
    main()
