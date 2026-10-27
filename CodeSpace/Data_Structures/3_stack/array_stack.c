#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 5

/* 基于数组实现的栈 */
typedef struct {
    int *data;
    int size;
} ArrayStack;

/* 构造函数 */
ArrayStack *newArrayStack() {
    ArrayStack *stack = malloc(sizeof(ArrayStack));
    // 初始化一个大容量，避免扩容
    stack->data = malloc(sizeof(int) * MAX_SIZE);
    stack->size = 0;
    return stack;
}

/* 析构函数 */
void delArrayStack(ArrayStack *stack) {
    free(stack->data);
    free(stack);
}

/* 获取栈的长度 */
int size(ArrayStack *stack) {
    return stack->size;
}

/* 判断栈是否为空 */
bool isEmpty(ArrayStack *stack) {
    return stack->size == 0;
}

/* 入栈 */
void push(ArrayStack *stack, int num) {
    if (stack->size == MAX_SIZE) {
        printf("栈已满\n");
        return;
    }
    stack->data[stack->size] = num;
    stack->size++;
}

/* 访问栈顶元素 */
int peek(ArrayStack *stack) {
    if (stack->size == 0) {
        printf("栈为空\n");
        return INT_MAX;
    }
    return stack->data[stack->size - 1];
}

/* 出栈 */
int pop(ArrayStack *stack) {
    int val = peek(stack);
    if (stack->size == 0)
        return val;
    stack->size--;
    return val;
}

static void printStack(ArrayStack *s) {
    printf("栈(底->顶): ");
    for (int i = 0; i < s->size; i++)
        printf("%d ", s->data[i]);
    printf("| size=%d\n", size(s));
}

int main(void) {
    printf("=== newArrayStack: 创建栈 ===\n");
    ArrayStack *s = newArrayStack();
    printf("isEmpty = %d\n", isEmpty(s));

    printf("\n=== push: 入栈 1~6 (容量为 %d, 第 6 个会溢出) ===\n", MAX_SIZE);
    for (int i = 1; i <= 6; i++)
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

    printf("\n=== delArrayStack: 释放栈 ===\n");
    delArrayStack(s);
    printf("已释放\n");
    return 0;
}
