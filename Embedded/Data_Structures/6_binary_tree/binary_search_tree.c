#include <stdio.h>
#include <stdlib.h>

/* 二叉搜索树节点结构体 */
typedef struct TreeNode {
    int val;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

/* 构造函数 */
TreeNode *newTreeNode(int val) {
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
    node->val = val;
    node->left = NULL;
    node->right = NULL;
    return node;
}

/* 二叉搜索树 */
typedef struct {
    TreeNode *root;
} BinarySearchTree;

/* 查找节点 */
TreeNode *search(BinarySearchTree *bst, int num) {
    TreeNode *cur = bst->root;
    // 循环查找，越过叶节点后跳出
    while (cur != NULL) {
        if (cur->val < num) {
            // 目标节点在 cur 的右子树中
            cur = cur->right;
        } else if (cur->val > num) {
            // 目标节点在 cur 的左子树中
            cur = cur->left;
        } else {
            // 找到目标节点，跳出循环
            break;
        }
    }
    // 返回目标节点
    return cur;
}

/* 插入节点 */
void insert(BinarySearchTree *bst, int num) {
    // 若树为空，则初始化根节点
    if (bst->root == NULL) {
        bst->root = newTreeNode(num);
        return;
    }
    TreeNode *cur = bst->root, *pre = NULL;
    // 循环查找，越过叶节点后跳出
    while (cur != NULL) {
        // 找到重复节点，直接返回
        if (cur->val == num) {
            return;
        }
        pre = cur;
        if (cur->val < num) {
            // 插入位置在 cur 的右子树中
            cur = cur->right;
        } else {
            // 插入位置在 cur 的左子树中
            cur = cur->left;
        }
    }
    // 插入节点
    TreeNode *node = newTreeNode(num);
    if (pre->val < num) {
        pre->right = node;
    } else {
        pre->left = node;
    }
}

/* 删除节点 */
// 由于引入了 stdio.h ，此处无法使用 remove 关键词
void removeItem(BinarySearchTree *bst, int num) {
    // 若树为空，直接提前返回
    if (bst->root == NULL)
        return;
    TreeNode *cur = bst->root, *pre = NULL;
    // 循环查找，越过叶节点后跳出
    while (cur != NULL) {
        // 找到待删除节点，跳出循环
        if (cur->val == num)
            break;
        pre = cur;
        if (cur->val < num) {
            // 待删除节点在 root 的右子树中
            cur = cur->right;
        } else {
            // 待删除节点在 root 的左子树中
            cur = cur->left;
        }
    }
    // 若无待删除节点，则直接返回
    if (cur == NULL)
        return;
    // 判断待删除节点是否存在子节点
    if (cur->left == NULL || cur->right == NULL) {
        /* 子节点数量 = 0 or 1 */
        // 当子节点数量 = 0 / 1 时， child = nullptr / 该子节点
        TreeNode *child = cur->left != NULL ? cur->left : cur->right;
        // 删除节点 cur
        if (pre == NULL) {
            bst->root = child;
        } else if (pre->left == cur) {
            pre->left = child;
        } else {
            pre->right = child;
        }
        // 释放内存
        free(cur);
    } else {
        /* 子节点数量 = 2 */
        // 获取中序遍历中 cur 的下一个节点
        TreeNode *tmp = cur->right;
        while (tmp->left != NULL) {
            tmp = tmp->left;
        }
        int tmpVal = tmp->val;
        // 递归删除节点 tmp
        removeItem(bst, tmp->val);
        // 用 tmp 覆盖 cur
        cur->val = tmpVal;
    }
}

/* 中序遍历打印 */
void printInOrder(TreeNode *node) {
    if (node == NULL)
        return;
    printInOrder(node->left);
    printf("%d ", node->val);
    printInOrder(node->right);
}

int main(void) {
    printf("1. 创建二叉搜索树\n");
    BinarySearchTree *bst = (BinarySearchTree *)malloc(sizeof(BinarySearchTree));
    bst->root = NULL;

    printf("2. 依次插入 5, 3, 8, 1, 4\n");
    insert(bst, 5);
    insert(bst, 3);
    insert(bst, 8);
    insert(bst, 1);
    insert(bst, 4);
    printf("   中序遍历（有序）: ");
    printInOrder(bst->root);
    printf("\n");

    printf("3. 查找 4: ");
    TreeNode *found = search(bst, 4);
    if (found != NULL) {
        printf("找到，值=%d\n", found->val);
    } else {
        printf("未找到\n");
    }

    printf("4. 删除 3（有两个子节点，用后继节点 4 替换）\n");
    removeItem(bst, 3);
    printf("   中序遍历: ");
    printInOrder(bst->root);
    printf("\n");

    printf("5. 删除 1（叶节点）\n");
    removeItem(bst, 1);
    printf("   中序遍历: ");
    printInOrder(bst->root);
    printf("\n");
    return 0;
}


