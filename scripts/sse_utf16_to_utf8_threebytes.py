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
    arrs = []
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
      arrs.append(lent)
      idx = 0
      for z in range(4):
        idx += 3 ** z * m[z]
      
      if(idx in bigmap):
        assert m == bigmap[idx]
      else :
        bigmap[idx] = m
      arr.append(idx)
    t.append(arr)
    s.append(arrs)
  print("static const uint8_t twobytes_16bit_to_8bit_firstlookup[16][16] = ")
  print(cpp_arrayarray_initializer(t) + ";")
  print("static const uint8_t twobytes_16bit_to_8bit_len[16][16] = ")
  print(cpp_arrayarray_initializer(s)  + ";")

def shuf():
  t = []
  for i in range(81):
    if(not (i in bigmap)): continue
    m = bigmap[i]
    pos = 0
    M = [255 for i in range(16)]

    for j in range(4):
      if(m[j] == 0):
        # ascii always at 0,4,...
        M[pos] = j * 4 # ascii is always at even locations 0,1,...
        pos += 1
      elif (m[j] == 1):
        M[pos] = (j * 4 + 1) | 0b11100000
        M[pos + 1] = (j * 4) | 0b11000000
        pos += 2      
      else:
        M[pos] = (j * 4 + 3)| 0b11110000
        M[pos + 1] = (j * 4 + 1) | 0b11000000
        M[pos + 2] = (j * 4) | 0b11000000
        pos += 3
    t.append(M)
  print("static const uint8_t simple_compress_16bit_to_8bit_finallookup[81][16] = ")
  print(cpp_arrayarray_initializer(t) + ";")



if __name__ == '__main__':
    ascii_twobytes()
    shuf()