#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 100

typedef struct TreeNode {
    int val;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

/* 构建二叉树：分治 */
TreeNode *dfs(int *preorder, int *inorderMap, int i, int l, int r, int size) {
    // 子树区间为空时终止
    if (r - l < 0)
        return NULL;
    // 初始化根节点
    TreeNode *root = (TreeNode *)malloc(sizeof(TreeNode));
    root->val = preorder[i];
    root->left = NULL;
    root->right = NULL;
    // 查询 m ，从而划分左右子树
    int m = inorderMap[preorder[i]];
    // 子问题：构建左子树
    root->left = dfs(preorder, inorderMap, i + 1, l, m - 1, size);
    // 子问题：构建右子树
    root->right = dfs(preorder, inorderMap, i + 1 + m - l, m + 1, r, size);
    // 返回根节点
    return root;
}

/* 构建二叉树 */
TreeNode *buildTree(int *preorder, int preorderSize, int *inorder,
                    int inorderSize) {
    // 初始化哈希表，存储 inorder 元素到索引的映射
    (void)preorderSize;
    int *inorderMap = (int *)malloc(sizeof(int) * MAX_SIZE);
    for (int i = 0; i < inorderSize; i++) {
        inorderMap[inorder[i]] = i;
    }
    TreeNode *root =
        dfs(preorder, inorderMap, 0, 0, inorderSize - 1, inorderSize);
    free(inorderMap);
    return root;
}

void printPreorder(TreeNode *node) {
    if (node == NULL) return;
    printf("%d ", node->val);
    printPreorder(node->left);
    printPreorder(node->right);
}

void printInorder(TreeNode *node) {
    if (node == NULL) return;
    printInorder(node->left);
    printf("%d ", node->val);
    printInorder(node->right);
}

void freeTree(TreeNode *node) {
    if (node == NULL) return;
    freeTree(node->left);
    freeTree(node->right);
    free(node);
}

int main(void) {
    int preorder[] = {3, 9, 2, 1, 7};
    int inorder[] = {9, 3, 1, 2, 7};
    printf("前序: 3 9 2 1 7\n中序: 9 3 1 2 7\n");
    TreeNode *root = buildTree(preorder, 5, inorder, 5);
    printf("重建后的前序: ");
    printPreorder(root);
    printf("\n重建后的中序: ");
    printInorder(root);
    printf("\n根节点 = %d\n", root->val);
    freeTree(root);
    return 0;
}
