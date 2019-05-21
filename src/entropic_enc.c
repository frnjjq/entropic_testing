#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include <limits.h>
#include <stdint.h>
#include "../include/entropic_enc.h"

#define testBit(A,k) ((A & (1UL<<(k)))>>k)

#define LENGTH sizeof(mask)*CHAR_BIT
#define HUFFMAN 0
#define RLC1    1
#define RLC2    2


#ifndef AV_WB32
#   define AV_WB32(p, val) do {                 \
        uint32_t d = (val);                     \
        ((uint8_t*)(p))[3] = (d);               \
        ((uint8_t*)(p))[2] = (d)>>8;            \
        ((uint8_t*)(p))[1] = (d)>>16;           \
        ((uint8_t*)(p))[0] = (d)>>24;           \
    } while(0)
#endif

/**
 * Initialize the PutBitContext s.
 *
 * @param buffer the buffer where to put bits
 * @param buffer_size the size in bytes of buffer
 */
void init_put_bits(PutBitContext *s, uint8_t *buffer,
                                 int buffer_size)
{
    if (buffer_size < 0) {
        buffer_size = 0;
        buffer      = NULL;
    }

    s->size_in_bits = 9 * buffer_size; // con 9 deberia valer pues nuestro simbolo mas largo mide 9 bits
    s->buf          = buffer;
    s->buf_end      = s->buf + buffer_size*2;
    s->buf_ptr      = s->buf;
    s->bit_left     = 32;
    s->bit_buf      = 0;
}

/**
 * @return the total number of bits written to the bitstream.
 */
int put_bits_count(PutBitContext *s)
{
    return (s->buf_ptr - s->buf) * 8 +32 - s->bit_left;
}


/**
 * Write up to 31 bits into a bitstream.
 * Use put_bits32 to write 32 bits.
 */
void put_bits(PutBitContext *s, int n, unsigned int value)
{
    unsigned int bit_buf;
    int bit_left;

    if (n <= 31 && value < (1U << n)){}
    else {
        printf("Error!!!!!!!! n > 31\n");
        abort();
    }

    bit_buf  = s->bit_buf;
    bit_left = s->bit_left;

    if (n < bit_left) {
        bit_buf     = (bit_buf << n) | value;
        bit_left   -= n;
    } else {
        bit_buf   <<= bit_left;
        bit_buf    |= value >> (n - bit_left);
        if (3 < s->buf_end - s->buf_ptr) {
            AV_WB32(s->buf_ptr, bit_buf);
            s->buf_ptr += 4;
        } else {
            printf("Internal error, put_bits buffer too small\n");
            abort();
        }
        bit_left   += 32 - n;
        bit_buf     = value;
    }

    s->bit_buf  = bit_buf;
    s->bit_left = bit_left;

}

void flush_put_bits(PutBitContext *s)
{


#ifndef BITSTREAM_WRITER_LE
    if (s->bit_left < 32)
       s->bit_buf <<= s->bit_left;
#endif
   while (s->bit_left < 32) {
#ifdef BITSTREAM_WRITER_LE
        *s->buf_ptr++ = s->bit_buf;
        s->bit_buf  >>= 8;
#else
        *s->buf_ptr++ = s->bit_buf >> 24;
        s->bit_buf  <<= 8;
#endif
        s->bit_left  += 8;
    }
    s->bit_left = 32;
    s->bit_buf  = 0;
}

int entropic_enc(unsigned char *hops, uint8_t *bits, unsigned int line_width) {

    PutBitContext s;

    int mode = HUFFMAN, h0_counter = 0;
    int condition_length = 7;
    int rlc_length = 4;
    
    uint8_t hop = 0;
    uint8_t number[9] = { 1,1,1,1,1,1,1,1,1 };
    uint8_t longi[9] = { 9,7,5,3,1,2,4,6,8 };

    init_put_bits(&s, bits, line_width);

    for (unsigned int x = 0; x < line_width;) {

        hop = hops[x];
        if (hop == 4) h0_counter++;

        switch(mode){
            case HUFFMAN:
                put_bits(&s, longi[hop], number[hop]);
                if(hop != 4) h0_counter = 0;
                if (h0_counter == condition_length) {
                    mode = RLC1;
                    h0_counter = 0;
                }
            break;
            case RLC1:
                if (hop == 4 && h0_counter == 15) {
                    put_bits(&s, 1, 1);
                    mode = RLC2;
                    rlc_length++;
                    h0_counter = 0;
                } else if (hop != 4) {
                    put_bits(&s, rlc_length+1, h0_counter);
                    put_bits(&s, longi[hop]-1, number[hop]);
                    h0_counter = 0;
                    mode = HUFFMAN;
                }
            break;
            case RLC2:
                if (hop == 4 && h0_counter == 31) {
                    put_bits(&s, 1, 1);
                    h0_counter = 0;
                } else if (hop != 4) {
                    put_bits(&s, rlc_length+1, h0_counter);
                    put_bits(&s, longi[hop]-1, number[hop]);
                    rlc_length = 4;
                    h0_counter = 0;
                    mode = HUFFMAN;
                }
            break;
        }
#ifdef JUMP_TO_EVENS

        if (hop >= 5 || hop <= 3)
            x++;
        else
            x = (x + 2) & ~(1);
#else /* !JUMP_TO_EVENS */
		x++;
#endif /* JUMP_TO_EVENS */
    }

    if (h0_counter != 0 && mode != HUFFMAN)
		put_bits(&s, rlc_length+1, h0_counter);

    flush_put_bits(&s);
    return put_bits_count(&s);

}
