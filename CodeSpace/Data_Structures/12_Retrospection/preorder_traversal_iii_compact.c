#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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
int resPathSizes[MAX_SIZE];        // 每条解路径的长度

/* 前序遍历：例题三 */
void preOrder(TreeNode *root) {
    // 剪枝
    if (root == NULL || root->val == 3) {
        return;
    }
    // 尝试
    path[pathSize++] = root;
    if (root->val == 7) {
        // 记录解
        for (int i = 0; i < pathSize; i++) {
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

#if 0 // 通用框架仅作说明：State、Choice 等类型未定义，且与下方 backtrack 同名，故不参与编译
/* 回溯算法框架 */
void backtrack(State *state, Choice *choices, int numChoices, State *res,
               int numRes) {
    // 判断是否为解
    if (isSolution(state)) {
        // 记录解
        recordSolution(state, res, numRes);
        // 不再继续搜索
        return;
    }
    // 遍历所有选择
    for (int i = 0; i < numChoices; i++) {
        // 剪枝：判断选择是否合法
        if (isValid(state, &choices[i])) {
            // 尝试：做出选择，更新状态
            makeChoice(state, &choices[i]);
            backtrack(state, choices, numChoices, res, numRes);
            // 回退：撤销选择，恢复到之前的状态
            undoChoice(state, &choices[i]);
        }
    }
}

#endif

/* 判断当前状态是否为解 */
bool isSolution(void) { return pathSize > 0 && path[pathSize - 1]->val == 7; }

/* 记录解 */
void recordSolution(void) {
    for (int i = 0; i < pathSize; i++) {
        res[resSize][i] = path[i];
    }
    resPathSizes[resSize] = pathSize;
    resSize++;
}

/* 判断在当前状态下，该选择是否合法 */
bool isValid(TreeNode *choice) { return choice != NULL && choice->val != 3; }

/* 更新状态 */
void makeChoice(TreeNode *choice) { path[pathSize++] = choice; }

/* 恢复状态 */
void undoChoice(void) { pathSize--; }

/* 回溯算法：例题三 */
void backtrack(TreeNode *choices[2]) {
    // 检查是否为解
    if (isSolution()) {
        // 记录解
        recordSolution();
    }
    // 遍历所有选择
    for (int i = 0; i < 2; i++) {
        TreeNode *choice = choices[i];
        // 剪枝：检查选择是否合法
        if (isValid(choice)) {
            // 尝试：做出选择，更新状态
            makeChoice(choice);
            // 进行下一轮选择
            TreeNode *nextChoices[2] = {choice->left, choice->right};
            backtrack(nextChoices);
            // 回退：撤销选择，恢复到之前的状态
            undoChoice();
        }
    }
}

TreeNode *newNode(int val, TreeNode *left, TreeNode *right) {
    TreeNode *node = malloc(sizeof(TreeNode));
    node->val = val;
    node->left = left;
    node->right = right;
    return node;
}

void printResults(void) {
    for (int i = 0; i < resSize; i++) {
        printf("  路径 %d: ", i + 1);
        for (int j = 0; j < resPathSizes[i]; j++) printf("%d ", res[i][j]->val);
        printf("\n");
    }
}

int main(void) {
    // 树: 1 -> (7 -> (4, 7), 3 -> (6, 7))，路径遇到 3 时剪枝
    TreeNode *root = newNode(1, newNode(7, newNode(4, NULL, NULL), newNode(7, NULL, NULL)),
                             newNode(3, newNode(6, NULL, NULL), newNode(7, NULL, NULL)));
    preOrder(root);
    printf("方法一 preOrder: 共 %d 条路径\n", resSize);
    printResults();

    resSize = 0;
    pathSize = 0;
    TreeNode *choices[2] = {root, NULL};
    backtrack(choices);
    printf("方法二 backtrack: 共 %d 条路径\n", resSize);
    printResults();
    return 0;
}
