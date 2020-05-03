#!/usr/bin/env python3

from common import *


## Simple scenario
def print_simple():
  ## we need to generate 256 shuffle masks
  ## assumes little endian layout
  t = []
  s = []
  for i in [0x10]:##range(1<<8):
    m = [255 for i in range(16)]
    pos = 0
    for j in range(8):
        # we detect non-ascii, so ascii => 0 bits
        if((i & (1<<j)) != (1<<j)): # ascii
            m[pos] = j * 2 # ascii is always at even locations 0,1,...
            m[pos] = 0
            pos += 1
        else:
            # need to copy two bytes
            m[pos] = (j * 2 + 1) | 0b11100000 # first byte is more significant
            m[pos] = 0b11100000 
            m[pos + 1] = j * 2 | 0b11000000
            m[pos + 1] = 0b11100000 

            pos += 2
    s.append(pos)
    t.append(m)
  print("static const uint8_t simple_compress_16bit_to_8bit_lookup[256][16] = ")
  print(cpp_arrayarray_initializer(t) + ";")
  print("static const uint8_t simple_compress_16bit_to_8bit_len[256] = ")
  print(fill(cpp_array_initializer(s)) + ";")


if __name__ == '__main__':
    print_simple()