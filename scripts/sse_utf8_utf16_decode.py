#!/usr/bin/env python3
from common import *


def is_bit_set(mask, i):
    return (mask & ( 1<<i )) == ( 1<<i )

# computes the location of the 0 bits (index starts at zero)
def compute_locations(mask):
    answer = []
    i = 0
    while( (mask >> i) > 0 ):
        if(is_bit_set(mask,i)):
            answer.append(i)
        i += 1
    return answer


# computes the gaps between the 1
def compute_code_point_size(mask):
    positions = compute_locations(mask)
    answer = []
    oldx = -1
    for i in range(len(positions)):
        x = positions[i]
        answer.append(x-oldx)
        oldx = x
    return answer

## check that we have 8 1-2 byte
def easy_case12(code_point_size):
    if(len(code_point_size)<6):
        return False
    return max(code_point_size[:6])<=2


def grab_easy_case12_code_point_size(code_point_size):
    return code_point_size[:6]

def buildshuf12_twobytes(sizes):
    answer = [0 for i in range(16)]
    pos = 0
    for i in range(len(sizes)):
        if(sizes[i] == 1):
            answer[2*i] = pos
            answer[2*i+1] = 0xFF
            pos += 1
        else:
            answer[2*i] = pos + 1
            answer[2*i+1] = pos
            pos += 2
    return answer



def main12():
  easycase12 = set()
  for x in range(1<<12):
    sizes = compute_code_point_size(x)
    if(easy_case12(sizes)):
        z1 = grab_easy_case12_code_point_size(sizes)
        easycase12.add(tuple(z1))
  easycase12sorted = [x for x in easycase12]
  easycase12sorted.sort()
  print("#include <cstdint>")
  print("const uint8_t shufutf8twobytes["+str(len(easycase12sorted))+"][16] = ")
  print(cpp_arrayarray_initializer([buildshuf12_twobytes(z) for z in  easycase12sorted]), end=";\n")
  c = 0
  index = {}
  for t in easycase12sorted:
      index[t] = c
      c = c + 1
  arrg=[]
  for x in range(1<<12):
    sizes = compute_code_point_size(x)
    if(easy_case12(sizes)):
        z1 = grab_easy_case12_code_point_size(sizes)
        idx = index[tuple(z1)]
        s = sum(z1)
        arrg.append((idx,s))
    else:
        arrg.append((0,0))
  print("const uint8_t utf8twobytesbigindex["+str(len(arrg))+"][2] = ")
  print(cpp_arrayarray_initializer(arrg), end=";\n")

if __name__ == '__main__':
    main12()