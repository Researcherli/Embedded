#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 100

typedef struct TreeNode {
    int val;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

TreeNode *res[MAX_SIZE]; // 记录解
int resSize = 0;

/* 前序遍历：例题一 */
void preOrder(TreeNode *root) {
    if (root == NULL) {
        return;
    }
    if (root->val == 7) {
        // 记录解
        res[resSize++] = root;
    }
    preOrder(root->left);
    preOrder(root->right);
}

TreeNode *newNode(int val, TreeNode *left, TreeNode *right) {
    TreeNode *node = malloc(sizeof(TreeNode));
    node->val = val;
    node->left = left;
    node->right = right;
    return node;
}

int main(void) {
    // 树: 1 -> (7 -> (4, 7), 3 -> (6, 7))
    TreeNode *root = newNode(1, newNode(7, newNode(4, NULL, NULL), newNode(7, NULL, NULL)),
                             newNode(3, newNode(6, NULL, NULL), newNode(7, NULL, NULL)));
    preOrder(root);
    printf("值为 7 的节点共 %d 个\n", resSize);
    for (int i = 0; i < resSize; i++) printf("  节点 %d: val=%d\n", i + 1, res[i]->val);
    return 0;
}
