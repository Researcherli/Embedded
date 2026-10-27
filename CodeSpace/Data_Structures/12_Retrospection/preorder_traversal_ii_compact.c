#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 100

typedef struct TreeNode {
    int val;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

TreeNode *path[MAX_SIZE];          // 当前路径
int pathSize = 0;
TreeNode *res[MAX_SIZE][MAX_SIZE]; // 记录解（每个解是一条路径）
int resSize = 0;
int resPathSizes[MAX_SIZE]; // 每条解路径的长度

/* 前序遍历：例题二 */
void preOrder(TreeNode *root) {
    if (root == NULL) {
        return;
    }
    // 尝试
    path[pathSize++] = root;
    if (root->val == 7) {
        // 记录解
        for (int i = 0; i < pathSize; ++i) {
            res[resSize][i] = path[i];
        }
        resPathSizes[resSize] = pathSize;
        resSize++;
    }
    preOrder(root->left);
    preOrder(root->right);
    // 回退
    pathSize--;
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
    printf("从根到值为 7 的节点的路径共 %d 条\n", resSize);
    for (int i = 0; i < resSize; i++) {
        printf("  路径 %d: ", i + 1);
        for (int j = 0; j < resPathSizes[i]; j++) printf("%d ", res[i][j]->val);
        printf("\n");
    }
    return 0;
}
