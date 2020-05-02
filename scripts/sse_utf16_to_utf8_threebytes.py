#!/usr/bin/env python3

from common import *


def is_set(w,i):
  return (w & (1<<i)) == (1<<i)

bigmap = {}

def ascii_twobytes():
  t = []
  s = []
  for i in range(1<<4):
    # so bits in i mean "notascii"
    arr = []
    for j in range(1<<4):
      lent = 0
      # so bits in j mean "first in two bytes"
      m =  [0 for i in range(4)]
      for z in range(4):
        if(not is_set(i,z)):
          m[z] = 0 # ascii
          lent += 1
        else:
          if(is_set(j,z)):
            m[z] = 1
            lent += 2
          else :
            m[z] = 2
            lent += 3
      s.append(lent)
      idx = 0
      for z in range(4):
        idx += 3 ** z * m[z]
      
      if(idx in bigmap):
        assert m == bigmap[idx]
      else :
        bigmap[idx] = m
      arr.append(idx)
    t.append(arr)
  print("static const uint8_t twobytes_16bit_to_8bit_firstlookup[16][16] = ")
  print(cpp_arrayarray_initializer(t) + ";")
  print("static const uint8_t twobytes_compress_16bit_to_8bit_len[16] = ")
  print(fill(cpp_array_initializer(s)) + ";")


if __name__ == '__main__':
    ascii_twobytes()
    print(bigmap)