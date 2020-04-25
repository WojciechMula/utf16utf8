#!/usr/bin/env python3

from common import *


# input: 8-bit entity (1 means single UTF8 byte, 0 means two UTF8 bytes)
def pshufb_const(pattern):
    assert 0x00 << pattern <= 0xff
    tmp = {}

    for bit, index in enumerate([0, 4, 1, 5, 2, 6, 3, 7]):
        byte0_index = 2*index
        byte1_index = 2*index + 1

        if pattern & (1 << bit):
            tmp[index] = [byte0_index]
        else:
            tmp[index] = [byte1_index, byte0_index]

    result = []
    for index in range(8):
        result.extend(tmp[index])

    while len(result) != 16:
        result.append(-1)

    return result


def generate():
    print("static const int8_t compress_16bit_lookup[256][16] = {")
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

    print("static const uint8_t compress_16bit_length[256] = ")
    print(fill(cpp_array_initializer(arr)) + ";")


if __name__ == '__main__':
    generate()

