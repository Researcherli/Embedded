#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 100

/* 遍历结果数组（前序、中序、后序遍历函数向其中写入结果） */
int *arr;

/* 二叉树节点结构体 */
typedef struct TreeNode {
    int val;                // 节点值
    int height;             // 节点高度
    struct TreeNode *left;  // 左子节点指针
    struct TreeNode *right; // 右子节点指针
} TreeNode;

/* 构造函数 */
TreeNode *newTreeNode(int val) {
    TreeNode *node;

    node = (TreeNode *)malloc(sizeof(TreeNode));
    node->val = val;
    node->height = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

/* 层序遍历 */
int *levelOrder(TreeNode *root, int *size) {
    /* 辅助队列 */
    int front, rear;
    int index, *arr;
    TreeNode *node;
    TreeNode **queue;

    /* 辅助队列 */
    queue = (TreeNode **)malloc(sizeof(TreeNode *) * MAX_SIZE);
    // 队列指针
    front = 0, rear = 0;
    // 加入根节点
    queue[rear++] = root;
    // 初始化一个列表，用于保存遍历序列
    /* 辅助数组 */
    arr = (int *)malloc(sizeof(int) * MAX_SIZE);
    // 数组指针
    index = 0;
    while (front < rear) {
        // 队列出队
        node = queue[front++];
        // 保存节点值
        arr[index++] = node->val;
        if (node->left != NULL) {
            // 左子节点入队
            queue[rear++] = node->left;
        }
        if (node->right != NULL) {
            // 右子节点入队
            queue[rear++] = node->right;
        }
    }
    // 更新数组长度的值
    *size = index;
    arr = realloc(arr, sizeof(int) * (*size));

    // 释放辅助数组空间
    free(queue);
    return arr;
}

/* 前序遍历 */
void preOrder(TreeNode *root, int *size) {
    if (root == NULL)
        return;
    // 访问优先级：根节点 -> 左子树 -> 右子树
    arr[(*size)++] = root->val;
    preOrder(root->left, size);
    preOrder(root->right, size);
}

/* 中序遍历 */
void inOrder(TreeNode *root, int *size) {
    if (root == NULL)
        return;
    // 访问优先级：左子树 -> 根节点 -> 右子树
    inOrder(root->left, size);
    arr[(*size)++] = root->val;
    inOrder(root->right, size);
}

/* 后序遍历 */
void postOrder(TreeNode *root, int *size) {
    if (root == NULL)
        return;
    // 访问优先级：左子树 -> 右子树 -> 根节点
    postOrder(root->left, size);
    postOrder(root->right, size);
    arr[(*size)++] = root->val;
}

/* 打印序列 */
void printSeq(int *seq, int n) {
    for (int i = 0; i < n; i++) {
        printf("%d ", seq[i]);
    }
    printf("\n");
}

int main(void) {
    arr = (int *)malloc(sizeof(int) * MAX_SIZE);
    int size = 0;

    printf("1. 初始化二叉树：1 为根，2、3 为子节点，4、5 为 2 的子节点\n");
    TreeNode *n1 = newTreeNode(1);
    TreeNode *n2 = newTreeNode(2);
    TreeNode *n3 = newTreeNode(3);
    TreeNode *n4 = newTreeNode(4);
    TreeNode *n5 = newTreeNode(5);
    n1->left = n2;
    n1->right = n3;
    n2->left = n4;
    n2->right = n5;

    printf("2. 在 n1 和 n2 之间插入节点 0\n");
    TreeNode *P = newTreeNode(0);
    n1->left = P;
    P->left = n2;

    printf("3. 删除节点 0，恢复 n1 -> n2\n");
    n1->left = n2;
    free(P);

    printf("4. 层序遍历: ");
    int levelSize;
    int *levelRes = levelOrder(n1, &levelSize);
    printSeq(levelRes, levelSize);
    free(levelRes);

    printf("5. 前序遍历: ");
    preOrder(n1, &size);
    printSeq(arr, size);

    size = 0;
    printf("6. 中序遍历: ");
    inOrder(n1, &size);
    printSeq(arr, size);

    size = 0;
    printf("7. 后序遍历: ");
    postOrder(n1, &size);
    printSeq(arr, size);

    free(arr);
    return 0;
}



