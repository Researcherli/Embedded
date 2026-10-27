#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* 数组中第 i 个节点的左子节点索引 */
int left(int i) {
    return 2 * i + 1;
}

/* 数组中第 i 个节点的右子节点索引 */
int right(int i) {
    return 2 * i + 2;
}

/* 数组表示下的二叉树结构体 */
typedef struct {
    int *tree;
    int size;
} ArrayBinaryTree;

/* 构造函数 */
ArrayBinaryTree *newArrayBinaryTree(int *arr, int arrSize) {
    ArrayBinaryTree *abt = (ArrayBinaryTree *)malloc(sizeof(ArrayBinaryTree));
    abt->tree = malloc(sizeof(int) * arrSize);
    memcpy(abt->tree, arr, sizeof(int) * arrSize);
    abt->size = arrSize;
    return abt;
}

/* 析构函数 */
void delArrayBinaryTree(ArrayBinaryTree *abt) {
    free(abt->tree);
    free(abt);
}

/* 列表容量 */
int size(ArrayBinaryTree *abt) { return abt->size; }

/* 获取索引为 i 节点的值 */
int val(ArrayBinaryTree *abt, int i) {
    // 若索引越界，则返回 INT_MAX ，代表空位
    if (i < 0 || i >= size(abt))
        return INT_MAX;
    return abt->tree[i];
}

/* 层序遍历 */
int *levelOrder(ArrayBinaryTree *abt, int *returnSize) {
    int *res = (int *)malloc(sizeof(int) * size(abt));
    int index = 0;
    // 直接遍历数组
    for (int i = 0; i < size(abt); i++) {
        if (val(abt, i) != INT_MAX)
            res[index++] = val(abt, i);
    }
    *returnSize = index;
    return res;
}

/* 深度优先遍历 */
void dfs(ArrayBinaryTree *abt, int i, char *order, int *res, int *index) {
    // 若为空位，则返回
    if (val(abt, i) == INT_MAX)
        return;
    // 前序遍历
    if (strcmp(order, "pre") == 0)
        res[(*index)++] = val(abt, i);
    dfs(abt, left(i), order, res, index);
    // 中序遍历
    if (strcmp(order, "in") == 0)
        res[(*index)++] = val(abt, i);
    dfs(abt, right(i), order, res, index);
    // 后序遍历
    if (strcmp(order, "post") == 0)
        res[(*index)++] = val(abt, i);
}

/* 前序遍历 */
int *preOrder(ArrayBinaryTree *abt, int *returnSize) {
    int *res = (int *)malloc(sizeof(int) * size(abt));
    int index = 0;
    dfs(abt, 0, "pre", res, &index);
    *returnSize = index;
    return res;
}

/* 中序遍历 */
int *inOrder(ArrayBinaryTree *abt, int *returnSize) {
    int *res = (int *)malloc(sizeof(int) * size(abt));
    int index = 0;
    dfs(abt, 0, "in", res, &index);
    *returnSize = index;
    return res;
}

/* 后序遍历 */
int *postOrder(ArrayBinaryTree *abt, int *returnSize) {
    int *res = (int *)malloc(sizeof(int) * size(abt));
    int index = 0;
    dfs(abt, 0, "post", res, &index);
    *returnSize = index;
    return res;
}

/* 打印序列 */
void printSeq(int *seq, int n) {
    for (int i = 0; i < n; i++) {
        printf("%d ", seq[i]);
    }
    printf("\n");
}

int main(void) {
    printf("1. 用数组 {1,2,3,4,5,6,7} 构造二叉树（下标 i 的子节点在 2i+1、2i+2）\n");
    int arr[] = {1, 2, 3, 4, 5, 6, 7};
    ArrayBinaryTree *abt = newArrayBinaryTree(arr, 7);

    printf("2. 节点数量=%d\n", size(abt));

    int n;
    int *res;

    res = levelOrder(abt, &n);
    printf("3. 层序遍历: ");
    printSeq(res, n);
    free(res);

    res = preOrder(abt, &n);
    printf("4. 前序遍历: ");
    printSeq(res, n);
    free(res);

    res = inOrder(abt, &n);
    printf("5. 中序遍历: ");
    printSeq(res, n);
    free(res);

    res = postOrder(abt, &n);
    printf("6. 后序遍历: ");
    printSeq(res, n);
    free(res);

    printf("7. 释放二叉树\n");
    delArrayBinaryTree(abt);
    return 0;
}

