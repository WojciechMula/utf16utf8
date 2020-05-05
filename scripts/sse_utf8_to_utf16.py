#!/usr/bin/env python3

import sys
from common import *


"""
We gathered two MSB bits from input, for instance:

    11 10 10 01 00 11 10 00
    ^^^^^^^^ == == ^^^^^  ==
    3-byte    |  | 2-byte  |
    char      |  |  char   |
              v  v         |
              ASCII <------+
"""
def dump(input_bit7, input_bit6):
    result = []
    for bit in range(8):
        b6 = int(bool(input_bit6 & (1 << bit)))
        b7 = int(bool(input_bit7 & (1 << bit)))
        result.append(f'{b7}{b6}')

    return ' '.join(result)


class WrongInput(Exception):
    pass


def classify(input_bit7, input_bit6):
    try:
        return classify_aux(input_bit7, input_bit6)
    except WrongInput:
        return None

def classify_aux(input_bit7, input_bit6):
    bytes_count = []
    multibyte = 0

    def flush():
        if multibyte > 0:
            if multibyte == 1:
                # malformed input: 11 followed by another 11 or 00/01
                raise WrongInput()
            
            bytes_count.append(multibyte)

    for bit in range(8):
        b6 = int(bool(input_bit6 & (1 << bit)))
        b7 = int(bool(input_bit7 & (1 << bit)))

        if b7 == 0:
            # ASCII, b6 is part of data
            flush()
            bytes_count.append(1)
            multibyte = 0
        else:
            assert b7 == 1
            if b6 == 1:
                # the first byte of multibyte char
                flush()
                multibyte = 1
            else:
                # one of tail bytes of multibyte char
                if multibyte == 0:
                    # wrong sequence: 0x 10 -- no leading byte
                    raise WrongInput()

                multibyte += 1

    # We are done. If there's no ASCII characters at the end of input
    # and some multibyte character has identified, we cannot tell just
    # from the two most significat bits the real length of such character.

    if multibyte > 4:
        # malformed input: head is valid, but the last (unfinished) char is too long
        # like: 00 00 00 11 10 10 10 10 (last char at least 5 chars?)
        raise WrongInput()

    tmp = [count for count in bytes_count if count <= 4]
    if tmp == bytes_count and bytes_count:
        return bytes_count
    else:
        # reject malformed inputs, like 11, 10, 10, 10, 10, 10, 00 [a 6-byte character]
        raise WrongInput()


def process_inputs():
    all_inputs = []
    valid_inputs = []
    valid_inputs.append((None, None, None)) # the sentinel value for wrong inputs 
    sentinel_index = 0

    for input_bit7 in range(256):
        for input_bit6 in range(256):
            lengths = classify(input_bit7, input_bit6)
            if lengths is not None:
                print(dump(input_bit7, input_bit6), lengths, sum(lengths))
                all_inputs.append(len(valid_inputs))
                valid_inputs.append((input_bit7, input_bit6, lengths))
            else:
                all_inputs.append(sentinel_index)

    print(f"valid = {len(valid_inputs)}, all = {len(all_inputs)}")
    return all_inputs, valid_inputs


CPP="""
#include <cstdint>

namespace utf8_to_utf16 {

    constexpr const uint8_t has_1byte_chars = 0x01;
    constexpr const uint8_t has_2byte_chars = 0x02;
    constexpr const uint8_t has_3byte_chars = 0x04;
    constexpr const uint8_t has_4byte_chars = 0x08;

    #pragma pack(1)
    struct Parameters {
        uint8_t chars;          // how many characters are produced
        uint8_t bytes;          // how many bytes are consumed
        uint8_t char_lengths;   // bit-or of has_{1,2,3,4}byte_chars constants
    };
    
const uint16_t lookup[65536] = %(lookup)s;

const Parameters parameters[%(input_size)d] = {
%(inputs)s
};

} // namespace utf8_to_utf16
"""


def generate_code(all_inputs, valid_inputs, path_cpp):
    tmp = []
    tmp.append('{0, 0, 0}') # sentinel

    for (_, _, bytes_count) in valid_inputs[1:]:
        chars = len(bytes_count)
        bytes = sum(bytes_count)
        char_lengths = (f'has_{k}byte_chars' for k in set(bytes_count))
        char_lengths = ' | '.join(char_lengths)
        tmp.append(f'{{{chars}, {bytes}, {char_lengths}}}')

    inputs = ''
    last   = len(tmp) - 1
    for k, input in enumerate(tmp):
        if k == last:
            inputs += f'\t{input}\n'
        else:
            inputs += f'\t{input},\n'

    with open(path_cpp, 'wt') as f:
        d = {}
        d['input_size'] = len(valid_inputs)
        d['inputs']     = inputs
        d['lookup']     = filltab(cpp_array_initializer(all_inputs))
        f.write(CPP % d)

    print(f"{path_cpp} saved")


def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} output.cpp")
        sys.exit(1)

    cpp_path = sys.argv[1]
    all_inputs, valid_inputs = process_inputs()
    generate_code(all_inputs, valid_inputs, cpp_path)


if __name__ == '__main__':
    main()
