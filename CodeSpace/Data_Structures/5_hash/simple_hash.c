#include <stdio.h>
#include <string.h>

/* 加法哈希 */
int addHash(char *key) {
    long long hash = 0;
    const int MODULUS = 1000000007;
    for (int i = 0; i < strlen(key); i++) {
        hash = (hash + (unsigned char)key[i]) % MODULUS;
    }
    return (int)hash;
}

/* 乘法哈希 */
int mulHash(char *key) {
    long long hash = 0;
    const int MODULUS = 1000000007;
    for (int i = 0; i < strlen(key); i++) {
        hash = (31 * hash + (unsigned char)key[i]) % MODULUS;
    }
    return (int)hash;
}

/* 异或哈希 */
int xorHash(char *key) {
    int hash = 0;
    const int MODULUS = 1000000007;

    for (int i = 0; i < strlen(key); i++) {
        hash ^= (unsigned char)key[i];
    }
    return hash & MODULUS;
}

/* 旋转哈希 */
int rotHash(char *key) {
    long long hash = 0;
    const int MODULUS = 1000000007;
    for (int i = 0; i < strlen(key); i++) {
        hash = ((hash << 4) ^ (hash >> 28) ^ (unsigned char)key[i]) % MODULUS;
    }

    return (int)hash;
}

int main(void) {
    char *key = "hello";
    printf("1. 对字符串 \"%s\" 计算各种哈希值\n", key);
    printf("2. 加法哈希: %d\n", addHash(key));
    printf("3. 乘法哈希: %d\n", mulHash(key));
    printf("4. 异或哈希: %d\n", xorHash(key));
    printf("5. 旋转哈希: %d\n", rotHash(key));

    printf("6. 换成字符串 \"world\"，再算一次乘法哈希: %d\n", mulHash("world"));
    return 0;
}
