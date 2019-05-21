#include <stdlib.h>
#include <stdio.h>

#include "../include/entropic_enc.h"
#include "../include/entropic_decoder.h"

int main()
{
    const unsigned char data[10] = {4, 4, 4, 4, 4, 6, 7, 3, 4, 5};
    unsigned char result[10];
    unsigned char bits[100];
    size_t bits_size, result_size;

    bits_size = entropic_enc(data, bits, sizeof(data));
    bits_size = bits_size % 8 == 0 ? bits_size / 8 : (bits_size / 8) + 1;
    printf("%lu symbols compressed in %lu bytes\n", sizeof(data), bits_size);

    result_size = decode_line_entropic(bits, result, bits_size);

    if (result_size != sizeof(data))
    {
        printf("Not same size\n");
        exit(1);
    }

    for (unsigned int i = 0; i< sizeof(data); i++)
    {
        if (data[i] != result[i])
        {
            printf("Not same for i %d\n", i);
            exit(1);          
        }
    }

    return 0;
}