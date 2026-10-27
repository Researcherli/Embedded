#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct ListNode {
    int val;
    struct ListNode *next;
} ListNode;

/* 基于链表实现的栈 */
typedef struct {
    ListNode *top; // 将头节点作为栈顶
    int size;      // 栈的长度
} LinkedListStack;

/* 构造函数 */
LinkedListStack *newLinkedListStack() {
    LinkedListStack *s = malloc(sizeof(LinkedListStack));
    s->top = NULL;
    s->size = 0;
    return s;
}

/* 析构函数 */
void delLinkedListStack(LinkedListStack *s) {
    while (s->top) {
        ListNode *n = s->top->next;
        free(s->top);
        s->top = n;
    }
    free(s);
}

/* 获取栈的长度 */
int size(LinkedListStack *s) {
    return s->size;
}

/* 判断栈是否为空 */
bool isEmpty(LinkedListStack *s) {
    return size(s) == 0;
}

/* 入栈 */
void push(LinkedListStack *s, int num) {
    ListNode *node = (ListNode *)malloc(sizeof(ListNode));
    node->next = s->top; // 更新新加节点指针域
    node->val = num;     // 更新新加节点数据域
    s->top = node;       // 更新栈顶
    s->size++;           // 更新栈大小
}

/* 访问栈顶元素 */
int peek(LinkedListStack *s) {
    if (s->size == 0) {
        printf("栈为空\n");
        return INT_MAX;
    }
    return s->top->val;
}

/* 出栈 */
int pop(LinkedListStack *s) {
    int val = peek(s);
    if (s->size == 0)
        return val;
    ListNode *tmp = s->top;
    s->top = s->top->next;
    // 释放内存
    free(tmp);
    s->size--;
    return val;
}

static void printStack(LinkedListStack *s) {
    printf("栈(顶->底): ");
    for (ListNode *n = s->top; n; n = n->next)
        printf("%d ", n->val);
    printf("| size=%d\n", size(s));
}

int main(void) {
    printf("=== newLinkedListStack: 创建栈 ===\n");
    LinkedListStack *s = newLinkedListStack();
    printf("isEmpty = %d\n", isEmpty(s));

    printf("\n=== push: 入栈 1~5 (链表栈无容量上限) ===\n");
    for (int i = 1; i <= 5; i++)
        push(s, i);
    printStack(s);

    printf("\n=== peek: 查看栈顶(不弹出) ===\n");
    printf("栈顶 = %d\n", peek(s));
    printStack(s);

    printf("\n=== pop: 出栈 ===\n");
    printf("弹出 %d\n", pop(s));
    printf("弹出 %d\n", pop(s));
    printStack(s);

    printf("\n=== size / isEmpty ===\n");
    printf("size = %d, isEmpty = %d\n", size(s), isEmpty(s));

    printf("\n=== 清空后再 pop/peek (空栈保护) ===\n");
    while (!isEmpty(s))
        pop(s);
    printf("isEmpty = %d\n", isEmpty(s));
    pop(s);
    printStack(s);

    printf("\n=== delLinkedListStack: 释放栈 ===\n");
    push(s, 100);
    push(s, 200);
    delLinkedListStack(s);
    printf("已释放(含剩余节点)\n");
    return 0;
}
