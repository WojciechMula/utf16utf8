#!/usr/bin/env python3

from common import *


# input: 8-bit entity consits 4 x 2-bit words
#        bit 0: value < 0x0080
#        bit 1: value < 0x0800
def pshufb_const_aux(pattern):
    assert 0x00 << pattern <= 0xff
    result = []

    for k in range(0, 4):
        lt0080 = bool(pattern & (1 << (2*k)))
        ge0800 = bool(pattern & (1 << (2*k + 1)))

        byte_index = 4 * k
        if lt0080 and ge0800:
            # never generate in code
            return []
        elif lt0080:
            result.append(byte_index) # 1 byte
        elif not lt0080 and not ge0800:
            result.append(byte_index + 1) # 2 bytes
            result.append(byte_index)
        elif ge0800:
            result.append(byte_index + 2)  # 3 bytes
            result.append(byte_index + 1)
            result.append(byte_index)

    return result


def pshufb_const(pattern):
    result = pshufb_const_aux(pattern)

    while len(result) != 16:
        result.append(-1)

    return result


def cpp_array_initializer(arr):
    return '{%s}' % (', '.join(map(str, arr)))


def generate():
    print("const int8_t compress_dwords_utf8_lookup[256][16] = {")
    for pattern in range(256):
        arr = pshufb_const(pattern)
        cpp = cpp_array_initializer(arr)
        if pattern < 255:
            comma = ","
        else:
            comma = ""

        print(f"{indent}{cpp}{comma}")

    print("};")

    print()

    arr = []
    for pattern in range(256):
        tmp = pshufb_const(pattern)
        arr.append(16 - tmp.count(-1))

    print("const uint8_t compress_dwords_utf8_length[256] = ")
    print(fill(cpp_array_initializer(arr)) + ";")


if __name__ == '__main__':
    generate()

