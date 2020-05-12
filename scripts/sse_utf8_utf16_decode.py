#!/usr/bin/env python3
from common import *


# So we are going to receive a 15-bit mask which
# represents a 16-bit mask with a 1-bit in the least
# significant bit
#
# The 1-bit match the start of a code point.
# The code points can span 1,2,3,4 bytes.
#
# Logically, the least signifit bit should have
# value 1.
#
# We output UTF16 values (2 bytes each).
# 
# In the "easy" cases, each code point spans 1 or 2
# bytes in UTF8. In such cases, we can take part of
# an 16-byte input (between 8 and 16) and output exactly 
# 16 bytes in UTF16. Easy.
# 
# In the harder part, we have all code points use between
# 1 to 3 bytes. In such cases, we may only be able to
# output 10 bytes (5 UTF16 code points.).
#
# If we detect longer code points, then we must bail out.
#
# To avoid stalling the processor, we need to return both
# the number of bytes consumed as well as a classification index.


def is_bit_set(mask, i):
    return (mask & ( 1<<i )) == ( 1<<i )


def compute_locations(mask15):
    answer = [0]
    for i in range(15):
        if(is_bit_set(mask15,i)):
            answer.append(i + 1)
    return answer


def compute_code_point_size(mask15):
    positions = compute_locations(mask15)
    answer = []
    oldx = positions[0]
    for i in range(1,len(positions)):
        x = positions[i]
        answer.append(x-oldx)
        oldx = x
    return answer

## check that we have 8 1-2 byte
def easy_case(code_point_size):
    if(len(code_point_size)<8):
        return False
    return max(code_point_size[:8])<=2

def third_byte_case(code_point_size):
    return len(code_point_size) > 0 and max(code_point_size)<4

def reject(code_point_size):
    return len(code_point_size) == 0 or max(code_point_size)>4

def grab_easy_case_code_point_size(code_point_size):
    return code_point_size[:8]

def grab_third_byte_case_code_point_size(code_point_size):
    firstbadindex = len(code_point_size)
    for z in range(len(code_point_size)):
        if(code_point_size[z] >= 4):
            firstbadindex = z
            break
    if(firstbadindex > 8): 
        firstbadindex = 8
    return code_point_size[:firstbadindex]

def main():
  easycase = set()
  thirdcase = set()
  for x in range(1<<15):
    sizes = compute_code_point_size(x)
    if(easy_case(sizes)):
        z1 = grab_easy_case_code_point_size(sizes)
        easycase.add(tuple(z1))
    elif(third_byte_case(sizes)):
        z1 = grab_third_byte_case_code_point_size(sizes)
        thirdcase.add(tuple(z1))
  easycasesorted = [x for x in easycase]
  easycasesorted.sort()
  c = 0
  index = {}
  for t in easycasesorted:
      index[t] = c
      c = c + 1
  thirdcasesorted = [x for x in thirdcase]
  thirdcasesorted.sort()
  print(len(easycase))
  print(len(thirdcase))


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
  for x in range(1<<11):
    sizes = compute_code_point_size(x)
    if(easy_case12(sizes)):
        z1 = grab_easy_case12_code_point_size(sizes)
        easycase12.add(tuple(z1))
  easycase12sorted = [x for x in easycase12]
  easycase12sorted.sort()
  #print(easycase12sorted)
  print("#include <cstdint>")
  print("const uint8_t shufutf8twobytes["+str(len(easycase12sorted))+"][16] = ")
  print(cpp_arrayarray_initializer([buildshuf12_twobytes(z) for z in  easycase12sorted]), end=";\n")
  c = 0
  index = {}
  for t in easycase12sorted:
      index[t] = c
      c = c + 1
  arrg=[]
  for x in range(1<<11):
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