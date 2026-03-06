#ifndef __FRAME_CHECK__
#define __FRAME_CHECK__
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

uint8_t* init_checker(size_t frame_size);

int check_frame(const uint8_t *frame, uint8_t *ref_frame, int frame_size, int frame_index);

void release_checker(uint8_t *ref_frame);

#endif
