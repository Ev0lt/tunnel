#ifndef CRYPTO_H
#define CRYPTO_H

int AesEncrypt(const unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext);

int AesDecrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *plaintext);

#endif