#include <stdio.h>
#include <stdlib.h>

/* 方法一：暴力枚举 */
int *twoSumBruteForce(int *nums, int numsSize, int target, int *returnSize) {
    for (int i = 0; i < numsSize; ++i) {
        for (int j = i + 1; j < numsSize; ++j) {
            if (nums[i] + nums[j] == target) {
                int *res = malloc(sizeof(int) * 2);
                res[0] = i, res[1] = j;
                *returnSize = 2;
                return res;
            }
        }
    }
    *returnSize = 0;
    return NULL;
}

/* 哈希表（拉链法，不依赖第三方库） */
#define HASH_SIZE 101

typedef struct HashTable {
    int key;
    int val;
    struct HashTable *next;
} HashTable;

/* 哈希函数：处理负数键值 */
int hashKey(int key) {
    return ((key % HASH_SIZE) + HASH_SIZE) % HASH_SIZE;
}

/* 哈希表查询 */
HashTable *find(HashTable *buckets[], int key) {
    for (HashTable *t = buckets[hashKey(key)]; t != NULL; t = t->next) {
        if (t->key == key)
            return t;
    }
    return NULL;
}

/* 哈希表元素插入 */
void insert(HashTable *buckets[], int key, int val) {
    HashTable *t = find(buckets, key);
    if (t == NULL) {
        t = malloc(sizeof(HashTable));
        t->key = key;
        t->next = buckets[hashKey(key)];
        buckets[hashKey(key)] = t;
    }
    t->val = val;
}

/* 方法二：辅助哈希表 */
int *twoSumHashTable(int *nums, int numsSize, int target, int *returnSize) {
    HashTable *buckets[HASH_SIZE] = {NULL};
    for (int i = 0; i < numsSize; i++) {
        HashTable *t = find(buckets, target - nums[i]);
        if (t != NULL) {
            int *res = malloc(sizeof(int) * 2);
            res[0] = t->val, res[1] = i;
            *returnSize = 2;
            return res;
        }
        insert(buckets, nums[i], i);
    }
    *returnSize = 0;
    return NULL;
}

int main(void) {
    int nums[] = {2, 7, 11, 15};
    int n = 4, target = 9, returnSize = 0;
    printf("数组: ");
    for (int i = 0; i < n; i++) printf("%d ", nums[i]);
    printf("\n目标和: %d\n", target);

    int *res = twoSumBruteForce(nums, n, target, &returnSize);
    printf("方法一（暴力枚举）下标: %d %d\n", res[0], res[1]);
    free(res);

    res = twoSumHashTable(nums, n, target, &returnSize);
    printf("方法二（哈希表）下标: %d %d\n", res[0], res[1]);
    free(res);
    return 0;
}


