#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <openssl/aes.h>
using namespace std;

static const unsigned char key[] = {
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

int main()
{
    unsigned char text[]="d1C4 B1 6A 1A994";
    unsigned char enc_out[17];
    unsigned char dec_out[17]; 

    AES_KEY enc_key, dec_key;

    AES_set_encrypt_key(key, 128, &enc_key);
    AES_encrypt(text, enc_out, &enc_key);  


    AES_set_decrypt_key(key,128,&dec_key);
    AES_decrypt(enc_out, dec_out, &dec_key);

    int i;

    printf("original:\t");
    for(i=0;*(text+i)!=0x00;i++)
        printf("%X ",*(text+i));
    printf("\nencrypted:\t");
    for(i=0;*(enc_out+i)!=0x00;i++)
        printf("%X ",*(enc_out+i));
    printf("\ndecrypted:\t");
    for(i=0;*(dec_out+i)!=0x00;i++)
        printf("%X ",*(dec_out+i));
    printf("\n");

    return 0;
} 

