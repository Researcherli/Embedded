// 顺序表
#include <stdio.h>

#define MAXSIZE 100

typedef int ElemType;

// 顺序表定义
typedef struct {
    ElemType data[MAXSIZE];
    int Length;
} SeqList;

// 顺序表初始化
void initList(SeqList *L) { L->Length = 0; }

// 顺序表-在尾部添加元素
int appendElem(SeqList *L, ElemType e) {
    if (L->Length >= MAXSIZE) {
        printf("顺序表已满\n");
        return 0;
    }

    L->data[L->Length] = e;
    L->Length++;
    return 1;
}

// 顺序表-遍历
void listElem(SeqList *L) {
    for (int i = 0; i < L->Length; i++) {
        printf("%d ", L->data[i]);
    }
    printf("\n");
}

// 顺序表-插入元素
int insertElem(SeqList *L, int pos, ElemType e) {
    if (L->Length >= MAXSIZE) {
        printf("顺序表已满\n");
        return 0;
    }

    if (pos < 1 || pos > L->Length) {
        printf("插入位置错误\n");
        return 0;
    }

    if (pos <= L->Length) {
        for (int i = L->Length; i >= pos - 1; i--) {
            L->data[i + 1] = L->data[i];
        }
        L->data[pos - 1] = e;
        L->Length++;
    }
    return 1;
}

// 顺序表-删除元素
int deleteElem(SeqList *L, int pos, ElemType *e) {
    if (L->Length == 0) {
        printf("空表\n");
        return 0;
    }

    if (pos < 1 || pos > L->Length) {
        printf("删除位置错误\n");
        return 0;
    }
    *e = L->data[pos - 1];
    if (pos < L->Length) {
        for (int i = pos; i < L->Length; i++) {
            L->data[i - 1] = L->data[i];
        }
    }
    L->Length--;
    return 1;
}

// 顺序表-查找元素
int findElem(SeqList *L, ElemType e) {
    if (L->Length == 0) {
        printf("空表\n");
        return 0;
    }

    for (int i = 0; i < L->Length; i++) {
        if (L->data[i] == e) {
            return i + 1;
        }
    }
    return 0;
}

int main(int argc, char const *argv[]) {
    (void)argc;
    (void)argv;
    // 声明一个顺序表并初始化
    SeqList list;
    initList(&list);

    printf("初始化成功，目前长度是: %d\n", list.Length);
    printf("目前占用空间是: %zu\n", sizeof(list.data));

    appendElem(&list, 11);
    appendElem(&list, 22);
    appendElem(&list, 33);
    appendElem(&list, 44);
    appendElem(&list, 55);
    listElem(&list);
    insertElem(&list, 2, 20);
    listElem(&list);
    ElemType delData;
    deleteElem(&list, 2, &delData);
    printf("被删除的数据是: %d\n", delData);
    listElem(&list);
    findElem(&list, 33);
    printf("查找数据的下标是:%d\n", findElem(&list, 33));
    return 0;
}
