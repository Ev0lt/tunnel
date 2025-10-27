#include<openssl/evp.h>
#include<stdio.h>
#include "crypto.h"

// #define encKey "3F7A#9c@2D!5e$8B%1k^6m*0n&4p#QwE"
// #define encIv "Zx$8mL#9nQ!2tV@5r"

unsigned char *encKey = "12345678912345678912345678912345";

unsigned char *encIv = "1234567812345678";

int AesEncrypt(const unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;
    
    // 创建并初始化上下文
    if (!(ctx = EVP_CIPHER_CTX_new()))
        return -1;
    
    // 初始化加密操作
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, encKey, encIv))
        return -1;
    
    // 提供消息文本并获得输出的密文
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        return -1;
    ciphertext_len = len;
    
    // 完成加密
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        return -1;
    ciphertext_len += len;
    
    // 清理
    EVP_CIPHER_CTX_free(ctx);
    
    return ciphertext_len;
}

// AES-CBC解密
int AesDecrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;
    
    // 创建并初始化上下文
    if (!(ctx = EVP_CIPHER_CTX_new()))
        return -1;
    
    // 初始化解密操作
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, encKey, encIv))
        return -1;
    
    // 提供消息密文并获得明文
    if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        return -1;
    plaintext_len = len;
    
    // 完成解密
    if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
        return -1;
    plaintext_len += len;
    
    // 清理
    EVP_CIPHER_CTX_free(ctx);
    
    return plaintext_len;
}