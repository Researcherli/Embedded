#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 键值对 int->string */
typedef struct {
    int key;
    char *val;
} Pair;

/* 链表节点 */
typedef struct Node {
    Pair *pair;
    struct Node *next;
} Node;

/* 链式地址哈希表 */
typedef struct {
    int size;         // 键值对数量
    int capacity;     // 哈希表容量
    double loadThres; // 触发扩容的负载因子阈值
    int extendRatio;  // 扩容倍数
    Node **buckets;   // 桶数组
} HashMapChaining;

void extend(HashMapChaining *hashMap);

/* 构造函数 */
HashMapChaining *newHashMapChaining() {
    HashMapChaining *hashMap = (HashMapChaining *)malloc(sizeof(HashMapChaining));
    hashMap->size = 0;
    hashMap->capacity = 4;
    hashMap->loadThres = 2.0 / 3.0;
    hashMap->extendRatio = 2;
    hashMap->buckets = (Node **)malloc(hashMap->capacity * sizeof(Node *));
    for (int i = 0; i < hashMap->capacity; i++) {
        hashMap->buckets[i] = NULL;
    }
    return hashMap;
}

/* 析构函数 */
void delHashMapChaining(HashMapChaining *hashMap) {
    for (int i = 0; i < hashMap->capacity; i++) {
        Node *cur = hashMap->buckets[i];
        while (cur) {
            Node *tmp = cur;
            cur = cur->next;
            free(tmp->pair);
            free(tmp);
        }
    }
    free(hashMap->buckets);
    free(hashMap);
}

/* 哈希函数 */
int hashFunc(HashMapChaining *hashMap, int key) {
    return key % hashMap->capacity;
}

/* 负载因子 */
double loadFactor(HashMapChaining *hashMap) {
    return (double)hashMap->size / (double)hashMap->capacity;
}

/* 查询操作 */
char *get(HashMapChaining *hashMap, int key) {
    int index = hashFunc(hashMap, key);
    // 遍历桶，若找到 key ，则返回对应 val
    Node *cur = hashMap->buckets[index];
    while (cur) {
        if (cur->pair->key == key) {
            return cur->pair->val;
        }
        cur = cur->next;
    }
    return ""; // 若未找到 key ，则返回空字符串
}

/* 添加操作 */
void put(HashMapChaining *hashMap, int key, const char *val) {
    // 当负载因子超过阈值时，执行扩容
    if (loadFactor(hashMap) > hashMap->loadThres) {
        extend(hashMap);
    }
    int index = hashFunc(hashMap, key);
    // 遍历桶，若遇到指定 key ，则更新对应 val 并返回
    Node *cur = hashMap->buckets[index];
    while (cur) {
        if (cur->pair->key == key) {
            free(cur->pair->val);
            cur->pair->val = (char *)malloc(strlen(val) + 1);
            strcpy(cur->pair->val, val); // 若遇到指定 key ，则更新对应 val 并返回
            return;
        }
        cur = cur->next;
    }
    // 若无该 key ，则将键值对添加至链表头部
    Pair *newPair = (Pair *)malloc(sizeof(Pair));
    newPair->key = key;
    newPair->val = (char *)malloc(strlen(val) + 1);
    strcpy(newPair->val, val);
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->pair = newPair;
    newNode->next = hashMap->buckets[index];
    hashMap->buckets[index] = newNode;
    hashMap->size++;
}

/* 扩容哈希表 */
void extend(HashMapChaining *hashMap) {
    // 暂存原哈希表
    int oldCapacity = hashMap->capacity;
    Node **oldBuckets = hashMap->buckets;
    // 初始化扩容后的新哈希表
    hashMap->capacity *= hashMap->extendRatio;
    hashMap->buckets = (Node **)malloc(hashMap->capacity * sizeof(Node *));
    for (int i = 0; i < hashMap->capacity; i++) {
        hashMap->buckets[i] = NULL;
    }
    hashMap->size = 0;
    // 将键值对从原哈希表搬运至新哈希表
    for (int i = 0; i < oldCapacity; i++) {
        Node *cur = oldBuckets[i];
        while (cur) {
            put(hashMap, cur->pair->key, cur->pair->val);
            Node *temp = cur;
            cur = cur->next;
            // 释放内存
            free(temp->pair);
            free(temp);
        }
    }

    free(oldBuckets);
}

/* 删除操作 */
void removeItem(HashMapChaining *hashMap, int key) {
    int index = hashFunc(hashMap, key);
    Node *cur = hashMap->buckets[index];
    Node *pre = NULL;
    while (cur) {
        if (cur->pair->key == key) {
            // 从中删除键值对
            if (pre) {
                pre->next = cur->next;
            } else {
                hashMap->buckets[index] = cur->next;
            }
            // 释放内存
            free(cur->pair);
            free(cur);
            hashMap->size--;
            return;
        }
        pre = cur;
        cur = cur->next;
    }
}

/* 打印哈希表 */
void print(HashMapChaining *hashMap) {
    for (int i = 0; i < hashMap->capacity; i++) {
        Node *cur = hashMap->buckets[i];
        printf("[");
        while (cur) {
            printf("%d -> %s, ", cur->pair->key, cur->pair->val);
            cur = cur->next;
        }
        printf("]\n");
    }
}

int main(void) {
    printf("1. 创建链式哈希表（初始容量 4）\n");
    HashMapChaining *hm = newHashMapChaining();

    printf("2. 依次添加 1->apple, 2->banana, 3->cherry, 4->date（第 4 个会触发扩容）\n");
    put(hm, 1, "apple");
    put(hm, 2, "banana");
    put(hm, 3, "cherry");
    put(hm, 4, "date");

    printf("3. 当前容量=%d, 元素个数=%d\n", hm->capacity, hm->size);
    printf("4. 打印哈希表（每行是一个桶）:\n");
    print(hm);

    printf("5. 查询键 2: %s\n", get(hm, 2));

    printf("6. 把键 2 的值改为 blueberry\n");
    put(hm, 2, "blueberry");
    printf("   再次查询键 2: %s\n", get(hm, 2));

    printf("7. 删除键 2\n");
    removeItem(hm, 2);
    printf("8. 删除后再次查询键 2: '%s'（空字符串表示不存在）\n", get(hm, 2));
    printf("9. 删除后打印哈希表:\n");
    print(hm);

    printf("10. 释放哈希表\n");
    delHashMapChaining(hm);
    return 0;
}
