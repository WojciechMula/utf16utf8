static const uint8_t simple_compress_16bit_to_8bit_lookup[256][16] = 
{    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
     {0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 255},
     {0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 255},
     {0, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 255, 255},
     {0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 255},
     {0, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 255, 255},
     {0, 1, 2, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 255, 255},
     {0, 2, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 15, 255},
     {0, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 15, 255, 255},
     {0, 1, 2, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 15, 255, 255},
     {0, 2, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 15, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 8, 9, 10, 11, 12, 13, 14, 15, 255, 255},
     {0, 2, 3, 4, 6, 8, 9, 10, 11, 12, 13, 14, 15, 255, 255, 255},
     {0, 1, 2, 4, 6, 8, 9, 10, 11, 12, 13, 14, 15, 255, 255, 255},
     {0, 2, 4, 6, 8, 9, 10, 11, 12, 13, 14, 15, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 15, 255},
     {0, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 15, 255, 255},
     {0, 1, 2, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 15, 255, 255},
     {0, 2, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 15, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 7, 8, 10, 11, 12, 13, 14, 15, 255, 255},
     {0, 2, 3, 4, 6, 7, 8, 10, 11, 12, 13, 14, 15, 255, 255, 255},
     {0, 1, 2, 4, 6, 7, 8, 10, 11, 12, 13, 14, 15, 255, 255, 255},
     {0, 2, 4, 6, 7, 8, 10, 11, 12, 13, 14, 15, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 8, 10, 11, 12, 13, 14, 15, 255, 255},
     {0, 2, 3, 4, 5, 6, 8, 10, 11, 12, 13, 14, 15, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 8, 10, 11, 12, 13, 14, 15, 255, 255, 255},
     {0, 2, 4, 5, 6, 8, 10, 11, 12, 13, 14, 15, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 8, 10, 11, 12, 13, 14, 15, 255, 255, 255},
     {0, 2, 3, 4, 6, 8, 10, 11, 12, 13, 14, 15, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 8, 10, 11, 12, 13, 14, 15, 255, 255, 255, 255},
     {0, 2, 4, 6, 8, 10, 11, 12, 13, 14, 15, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 255},
     {0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 255, 255},
     {0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 255, 255},
     {0, 2, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 12, 13, 14, 15, 255, 255},
     {0, 2, 3, 4, 6, 7, 8, 9, 10, 12, 13, 14, 15, 255, 255, 255},
     {0, 1, 2, 4, 6, 7, 8, 9, 10, 12, 13, 14, 15, 255, 255, 255},
     {0, 2, 4, 6, 7, 8, 9, 10, 12, 13, 14, 15, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 12, 13, 14, 15, 255, 255},
     {0, 2, 3, 4, 5, 6, 8, 9, 10, 12, 13, 14, 15, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, 15, 255, 255, 255},
     {0, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, 15, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 8, 9, 10, 12, 13, 14, 15, 255, 255, 255},
     {0, 2, 3, 4, 6, 8, 9, 10, 12, 13, 14, 15, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 8, 9, 10, 12, 13, 14, 15, 255, 255, 255, 255},
     {0, 2, 4, 6, 8, 9, 10, 12, 13, 14, 15, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 13, 14, 15, 255, 255},
     {0, 2, 3, 4, 5, 6, 7, 8, 10, 12, 13, 14, 15, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 7, 8, 10, 12, 13, 14, 15, 255, 255, 255},
     {0, 2, 4, 5, 6, 7, 8, 10, 12, 13, 14, 15, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 7, 8, 10, 12, 13, 14, 15, 255, 255, 255},
     {0, 2, 3, 4, 6, 7, 8, 10, 12, 13, 14, 15, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 7, 8, 10, 12, 13, 14, 15, 255, 255, 255, 255},
     {0, 2, 4, 6, 7, 8, 10, 12, 13, 14, 15, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 8, 10, 12, 13, 14, 15, 255, 255, 255},
     {0, 2, 3, 4, 5, 6, 8, 10, 12, 13, 14, 15, 255, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 8, 10, 12, 13, 14, 15, 255, 255, 255, 255},
     {0, 2, 4, 5, 6, 8, 10, 12, 13, 14, 15, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 8, 10, 12, 13, 14, 15, 255, 255, 255, 255},
     {0, 2, 3, 4, 6, 8, 10, 12, 13, 14, 15, 255, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 8, 10, 12, 13, 14, 15, 255, 255, 255, 255, 255},
     {0, 2, 4, 6, 8, 10, 12, 13, 14, 15, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 255},
     {0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 255, 255},
     {0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 255, 255},
     {0, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 14, 15, 255, 255},
     {0, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 14, 15, 255, 255, 255},
     {0, 1, 2, 4, 6, 7, 8, 9, 10, 11, 12, 14, 15, 255, 255, 255},
     {0, 2, 4, 6, 7, 8, 9, 10, 11, 12, 14, 15, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 14, 15, 255, 255},
     {0, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 14, 15, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 8, 9, 10, 11, 12, 14, 15, 255, 255, 255},
     {0, 2, 4, 5, 6, 8, 9, 10, 11, 12, 14, 15, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 8, 9, 10, 11, 12, 14, 15, 255, 255, 255},
     {0, 2, 3, 4, 6, 8, 9, 10, 11, 12, 14, 15, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 8, 9, 10, 11, 12, 14, 15, 255, 255, 255, 255},
     {0, 2, 4, 6, 8, 9, 10, 11, 12, 14, 15, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 14, 15, 255, 255},
     {0, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 14, 15, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 7, 8, 10, 11, 12, 14, 15, 255, 255, 255},
     {0, 2, 4, 5, 6, 7, 8, 10, 11, 12, 14, 15, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 7, 8, 10, 11, 12, 14, 15, 255, 255, 255},
     {0, 2, 3, 4, 6, 7, 8, 10, 11, 12, 14, 15, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 7, 8, 10, 11, 12, 14, 15, 255, 255, 255, 255},
     {0, 2, 4, 6, 7, 8, 10, 11, 12, 14, 15, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 8, 10, 11, 12, 14, 15, 255, 255, 255},
     {0, 2, 3, 4, 5, 6, 8, 10, 11, 12, 14, 15, 255, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 8, 10, 11, 12, 14, 15, 255, 255, 255, 255},
     {0, 2, 4, 5, 6, 8, 10, 11, 12, 14, 15, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 8, 10, 11, 12, 14, 15, 255, 255, 255, 255},
     {0, 2, 3, 4, 6, 8, 10, 11, 12, 14, 15, 255, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 8, 10, 11, 12, 14, 15, 255, 255, 255, 255, 255},
     {0, 2, 4, 6, 8, 10, 11, 12, 14, 15, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 15, 255, 255},
     {0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 15, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 12, 14, 15, 255, 255, 255},
     {0, 2, 4, 5, 6, 7, 8, 9, 10, 12, 14, 15, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 12, 14, 15, 255, 255, 255},
     {0, 2, 3, 4, 6, 7, 8, 9, 10, 12, 14, 15, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 7, 8, 9, 10, 12, 14, 15, 255, 255, 255, 255},
     {0, 2, 4, 6, 7, 8, 9, 10, 12, 14, 15, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 12, 14, 15, 255, 255, 255},
     {0, 2, 3, 4, 5, 6, 8, 9, 10, 12, 14, 15, 255, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 14, 15, 255, 255, 255, 255},
     {0, 2, 4, 5, 6, 8, 9, 10, 12, 14, 15, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 8, 9, 10, 12, 14, 15, 255, 255, 255, 255},
     {0, 2, 3, 4, 6, 8, 9, 10, 12, 14, 15, 255, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 8, 9, 10, 12, 14, 15, 255, 255, 255, 255, 255},
     {0, 2, 4, 6, 8, 9, 10, 12, 14, 15, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 15, 255, 255, 255},
     {0, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 15, 255, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 7, 8, 10, 12, 14, 15, 255, 255, 255, 255},
     {0, 2, 4, 5, 6, 7, 8, 10, 12, 14, 15, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 7, 8, 10, 12, 14, 15, 255, 255, 255, 255},
     {0, 2, 3, 4, 6, 7, 8, 10, 12, 14, 15, 255, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 7, 8, 10, 12, 14, 15, 255, 255, 255, 255, 255},
     {0, 2, 4, 6, 7, 8, 10, 12, 14, 15, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 8, 10, 12, 14, 15, 255, 255, 255, 255},
     {0, 2, 3, 4, 5, 6, 8, 10, 12, 14, 15, 255, 255, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 8, 10, 12, 14, 15, 255, 255, 255, 255, 255},
     {0, 2, 4, 5, 6, 8, 10, 12, 14, 15, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 8, 10, 12, 14, 15, 255, 255, 255, 255, 255},
     {0, 2, 3, 4, 6, 8, 10, 12, 14, 15, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 8, 10, 12, 14, 15, 255, 255, 255, 255, 255, 255},
     {0, 2, 4, 6, 8, 10, 12, 14, 15, 255, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 255},
     {0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 255, 255},
     {0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 255, 255},
     {0, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 255, 255},
     {0, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 255, 255, 255},
     {0, 1, 2, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 255, 255, 255},
     {0, 2, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 255, 255},
     {0, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 255, 255, 255},
     {0, 2, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 8, 9, 10, 11, 12, 13, 14, 255, 255, 255},
     {0, 2, 3, 4, 6, 8, 9, 10, 11, 12, 13, 14, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 8, 9, 10, 11, 12, 13, 14, 255, 255, 255, 255},
     {0, 2, 4, 6, 8, 9, 10, 11, 12, 13, 14, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 255, 255},
     {0, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 255, 255, 255},
     {0, 2, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 7, 8, 10, 11, 12, 13, 14, 255, 255, 255},
     {0, 2, 3, 4, 6, 7, 8, 10, 11, 12, 13, 14, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 7, 8, 10, 11, 12, 13, 14, 255, 255, 255, 255},
     {0, 2, 4, 6, 7, 8, 10, 11, 12, 13, 14, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 8, 10, 11, 12, 13, 14, 255, 255, 255},
     {0, 2, 3, 4, 5, 6, 8, 10, 11, 12, 13, 14, 255, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 8, 10, 11, 12, 13, 14, 255, 255, 255, 255},
     {0, 2, 4, 5, 6, 8, 10, 11, 12, 13, 14, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 8, 10, 11, 12, 13, 14, 255, 255, 255, 255},
     {0, 2, 3, 4, 6, 8, 10, 11, 12, 13, 14, 255, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 8, 10, 11, 12, 13, 14, 255, 255, 255, 255, 255},
     {0, 2, 4, 6, 8, 10, 11, 12, 13, 14, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 255, 255},
     {0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 255, 255, 255},
     {0, 2, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 12, 13, 14, 255, 255, 255},
     {0, 2, 3, 4, 6, 7, 8, 9, 10, 12, 13, 14, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 7, 8, 9, 10, 12, 13, 14, 255, 255, 255, 255},
     {0, 2, 4, 6, 7, 8, 9, 10, 12, 13, 14, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 12, 13, 14, 255, 255, 255},
     {0, 2, 3, 4, 5, 6, 8, 9, 10, 12, 13, 14, 255, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, 255, 255, 255, 255},
     {0, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 8, 9, 10, 12, 13, 14, 255, 255, 255, 255},
     {0, 2, 3, 4, 6, 8, 9, 10, 12, 13, 14, 255, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 8, 9, 10, 12, 13, 14, 255, 255, 255, 255, 255},
     {0, 2, 4, 6, 8, 9, 10, 12, 13, 14, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 13, 14, 255, 255, 255},
     {0, 2, 3, 4, 5, 6, 7, 8, 10, 12, 13, 14, 255, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 7, 8, 10, 12, 13, 14, 255, 255, 255, 255},
     {0, 2, 4, 5, 6, 7, 8, 10, 12, 13, 14, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 7, 8, 10, 12, 13, 14, 255, 255, 255, 255},
     {0, 2, 3, 4, 6, 7, 8, 10, 12, 13, 14, 255, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 7, 8, 10, 12, 13, 14, 255, 255, 255, 255, 255},
     {0, 2, 4, 6, 7, 8, 10, 12, 13, 14, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 8, 10, 12, 13, 14, 255, 255, 255, 255},
     {0, 2, 3, 4, 5, 6, 8, 10, 12, 13, 14, 255, 255, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 8, 10, 12, 13, 14, 255, 255, 255, 255, 255},
     {0, 2, 4, 5, 6, 8, 10, 12, 13, 14, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 8, 10, 12, 13, 14, 255, 255, 255, 255, 255},
     {0, 2, 3, 4, 6, 8, 10, 12, 13, 14, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 8, 10, 12, 13, 14, 255, 255, 255, 255, 255, 255},
     {0, 2, 4, 6, 8, 10, 12, 13, 14, 255, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 255, 255},
     {0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 255, 255, 255},
     {0, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 14, 255, 255, 255},
     {0, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 14, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 7, 8, 9, 10, 11, 12, 14, 255, 255, 255, 255},
     {0, 2, 4, 6, 7, 8, 9, 10, 11, 12, 14, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 14, 255, 255, 255},
     {0, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 14, 255, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 8, 9, 10, 11, 12, 14, 255, 255, 255, 255},
     {0, 2, 4, 5, 6, 8, 9, 10, 11, 12, 14, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 8, 9, 10, 11, 12, 14, 255, 255, 255, 255},
     {0, 2, 3, 4, 6, 8, 9, 10, 11, 12, 14, 255, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 8, 9, 10, 11, 12, 14, 255, 255, 255, 255, 255},
     {0, 2, 4, 6, 8, 9, 10, 11, 12, 14, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 14, 255, 255, 255},
     {0, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 14, 255, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 7, 8, 10, 11, 12, 14, 255, 255, 255, 255},
     {0, 2, 4, 5, 6, 7, 8, 10, 11, 12, 14, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 7, 8, 10, 11, 12, 14, 255, 255, 255, 255},
     {0, 2, 3, 4, 6, 7, 8, 10, 11, 12, 14, 255, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 7, 8, 10, 11, 12, 14, 255, 255, 255, 255, 255},
     {0, 2, 4, 6, 7, 8, 10, 11, 12, 14, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 8, 10, 11, 12, 14, 255, 255, 255, 255},
     {0, 2, 3, 4, 5, 6, 8, 10, 11, 12, 14, 255, 255, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 8, 10, 11, 12, 14, 255, 255, 255, 255, 255},
     {0, 2, 4, 5, 6, 8, 10, 11, 12, 14, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 8, 10, 11, 12, 14, 255, 255, 255, 255, 255},
     {0, 2, 3, 4, 6, 8, 10, 11, 12, 14, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 8, 10, 11, 12, 14, 255, 255, 255, 255, 255, 255},
     {0, 2, 4, 6, 8, 10, 11, 12, 14, 255, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 255, 255, 255},
     {0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 255, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 12, 14, 255, 255, 255, 255},
     {0, 2, 4, 5, 6, 7, 8, 9, 10, 12, 14, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 12, 14, 255, 255, 255, 255},
     {0, 2, 3, 4, 6, 7, 8, 9, 10, 12, 14, 255, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 7, 8, 9, 10, 12, 14, 255, 255, 255, 255, 255},
     {0, 2, 4, 6, 7, 8, 9, 10, 12, 14, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 12, 14, 255, 255, 255, 255},
     {0, 2, 3, 4, 5, 6, 8, 9, 10, 12, 14, 255, 255, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 14, 255, 255, 255, 255, 255},
     {0, 2, 4, 5, 6, 8, 9, 10, 12, 14, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 8, 9, 10, 12, 14, 255, 255, 255, 255, 255},
     {0, 2, 3, 4, 6, 8, 9, 10, 12, 14, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 8, 9, 10, 12, 14, 255, 255, 255, 255, 255, 255},
     {0, 2, 4, 6, 8, 9, 10, 12, 14, 255, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 255, 255, 255, 255},
     {0, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 255, 255, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 7, 8, 10, 12, 14, 255, 255, 255, 255, 255},
     {0, 2, 4, 5, 6, 7, 8, 10, 12, 14, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 7, 8, 10, 12, 14, 255, 255, 255, 255, 255},
     {0, 2, 3, 4, 6, 7, 8, 10, 12, 14, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 7, 8, 10, 12, 14, 255, 255, 255, 255, 255, 255},
     {0, 2, 4, 6, 7, 8, 10, 12, 14, 255, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 5, 6, 8, 10, 12, 14, 255, 255, 255, 255, 255},
     {0, 2, 3, 4, 5, 6, 8, 10, 12, 14, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 4, 5, 6, 8, 10, 12, 14, 255, 255, 255, 255, 255, 255},
     {0, 2, 4, 5, 6, 8, 10, 12, 14, 255, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 3, 4, 6, 8, 10, 12, 14, 255, 255, 255, 255, 255, 255},
     {0, 2, 3, 4, 6, 8, 10, 12, 14, 255, 255, 255, 255, 255, 255, 255},
     {0, 1, 2, 4, 6, 8, 10, 12, 14, 255, 255, 255, 255, 255, 255, 255},
     {0, 2, 4, 6, 8, 10, 12, 14, 255, 255, 255, 255, 255, 255, 255, 255}};
static const uint8_t simple_compress_16bit_to_8bit_len[256] = 
    {16, 15, 15, 14, 15, 14, 14, 13, 15, 14, 14, 13, 14, 13, 13, 12, 15,
    14, 14, 13, 14, 13, 13, 12, 14, 13, 13, 12, 13, 12, 12, 11, 15, 14,
    14, 13, 14, 13, 13, 12, 14, 13, 13, 12, 13, 12, 12, 11, 14, 13, 13,
    12, 13, 12, 12, 11, 13, 12, 12, 11, 12, 11, 11, 10, 15, 14, 14, 13,
    14, 13, 13, 12, 14, 13, 13, 12, 13, 12, 12, 11, 14, 13, 13, 12, 13,
    12, 12, 11, 13, 12, 12, 11, 12, 11, 11, 10, 14, 13, 13, 12, 13, 12,
    12, 11, 13, 12, 12, 11, 12, 11, 11, 10, 13, 12, 12, 11, 12, 11, 11,
    10, 12, 11, 11, 10, 11, 10, 10, 9, 15, 14, 14, 13, 14, 13, 13, 12, 14,
    13, 13, 12, 13, 12, 12, 11, 14, 13, 13, 12, 13, 12, 12, 11, 13, 12,
    12, 11, 12, 11, 11, 10, 14, 13, 13, 12, 13, 12, 12, 11, 13, 12, 12,
    11, 12, 11, 11, 10, 13, 12, 12, 11, 12, 11, 11, 10, 12, 11, 11, 10,
    11, 10, 10, 9, 14, 13, 13, 12, 13, 12, 12, 11, 13, 12, 12, 11, 12, 11,
    11, 10, 13, 12, 12, 11, 12, 11, 11, 10, 12, 11, 11, 10, 11, 10, 10, 9,
    13, 12, 12, 11, 12, 11, 11, 10, 12, 11, 11, 10, 11, 10, 10, 9, 12, 11,
    11, 10, 11, 10, 10, 9, 11, 10, 10, 9, 10, 9, 9, 8};
