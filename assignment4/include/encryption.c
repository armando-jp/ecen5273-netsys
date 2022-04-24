#include "encryption.h"

void encrypt_decrypt(char *input, int input_length, char *key, int key_length)
{
    for (int i = 0; i < input_length; i++)
    {
        input[i] ^= key[i % key_length];  
    }
}