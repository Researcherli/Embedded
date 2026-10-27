#include <stdio.h>
#include <stdlib.h>
#include "list.h"

#define configMAX_PRIORITIES 4 // 假设系统支持 0~3 共 4 个优先级

/* ================= 1. 模拟 tasks.c 的核心数据结构 ================= */
typedef struct tskTaskControlBlock {
    const char *pcTaskName;       // 任务名
    uint16_t uxPriority;          // 任务优先级 (0~3)
    ListItem_t xStateListItem;    // 状态链表节点（用于挂载到就绪/延时链表）
} TCB_t;

// 核心全局变量（与 FreeRTOS tasks.c 完全同名！）
List_t pxReadyTasksLists[configMAX_PRIORITIES]; // 就绪链表数组
List_t xDelayedTaskList;                        // 延时链表
TCB_t *pxCurrentTCB = NULL;                     // 当前正在运行的任务指针
uint32_t uxTopReadyPriority = 0;                // 就绪优先级位图 (BitMap)
uint16_t xTickCount = 0;                        // 系统时钟节拍计数器

/* ================= 2. 模拟 tasks.c 的位图与链表宏 ================= */
// 将任务放入对应优先级的就绪链表，并将位图对应 bit 置 1
#define prvAddTaskToReadyList(pxTCB)                                           \
    do {                                                                       \
        uxTopReadyPriority |= (1UL << (pxTCB)->uxPriority);                    \
        vListInsertEnd(&(pxReadyTasksLists[(pxTCB)->uxPriority]),              \
                       &((pxTCB)->xStateListItem));                            \
    } while (0)

// 遍历就绪链表（移动 pxIndex 指针，实现同优先级“时间片轮转”）
#define listGET_OWNER_OF_NEXT_ENTRY(pxTCB, pxList)                             \
    do {                                                                       \
        List_t *const pxConstList = (pxList);                                  \
        (pxConstList)->pxIndex = (pxConstList)->pxIndex->pxNext;               \
        if ((void *)(pxConstList)->pxIndex == (void *)&((pxConstList)->xListEnd)) { \
            (pxConstList)->pxIndex = (pxConstList)->pxIndex->pxNext;           \
        }                                                                      \
        (pxTCB) = (TCB_t *)(pxConstList)->pxIndex->pvOwner;                    \
    } while (0)

/* ================= 3. 模拟 tasks.c 的四大核心函数 ================= */

// 初始化调度器链表
void vSchedulerInit(void) {
    for (int i = 0; i < configMAX_PRIORITIES; i++) {
        vListInitialise(&pxReadyTasksLists[i]);
    }
    vListInitialise(&xDelayedTaskList);
}

// 【动作1】创建任务并加入就绪列表
void vTaskCreate_Sim(TCB_t *pxNewTCB, const char *name, uint16_t prio) {
    pxNewTCB->pcTaskName = name;
    pxNewTCB->uxPriority = prio;

    vListInitialiseItem(&(pxNewTCB->xStateListItem));
    // 关键：把链表节点的 Owner 指向 TCB 自身！
    pxNewTCB->xStateListItem.pvOwner = (void *)pxNewTCB;

    prvAddTaskToReadyList(pxNewTCB);
}

// 【动作2】上下文切换：从最高优先级就绪链表中挑出下一个要跑的任务
void vTaskSwitchContext_Sim(void) {
    // 利用 Clang/GCC 内置的前导零指令 __builtin_clz (对应 ARM 的 CLZ 汇编指令)
    // O(1) 时间复杂度瞬间算出最高就绪优先级！
    uint32_t uxTopPriority = 31 - __builtin_clz(uxTopReadyPriority);

    // 在最高优先级的就绪链表里，顺序取出下一个任务（同优先级自动时间片轮转）
    listGET_OWNER_OF_NEXT_ENTRY(pxCurrentTCB, &(pxReadyTasksLists[uxTopPriority]));
    printf("  [调度器切换] ---> 当前运行任务: %-10s (优先级: %u)\n",
           pxCurrentTCB->pcTaskName, pxCurrentTCB->uxPriority);
}

// 【动作3】当前任务调用延时阻塞 vTaskDelay()
void vTaskDelay_Sim(uint16_t xTicksToWait) {
    printf("\n>>> 任务 [%s] 调用 vTaskDelay(%u)，进入阻塞态...\n",
           pxCurrentTCB->pcTaskName, xTicksToWait);

    // 1. 先从就绪链表中摘除，如果该优先级下没任务了，把位图对应 bit 清零！
    if (uxListRemove(&(pxCurrentTCB->xStateListItem)) == 0) {
        uxTopReadyPriority &= ~(1UL << pxCurrentTCB->uxPriority);
    }

    // 2. 计算唤醒时间点，并按升序插入延时链表
    pxCurrentTCB->xStateListItem.xItemValue = xTickCount + xTicksToWait;
    vListInsert(&xDelayedTaskList, &(pxCurrentTCB->xStateListItem));

    // 3. 当前任务阻塞了，立刻触发重调度！
    vTaskSwitchContext_Sim();
}

// 【动作4】模拟 SysTick 中断：时钟节拍加 1，检查是否有任务到期
void xTaskIncrementTick_Sim(void) {
    xTickCount++;
    printf("\n[SysTick = %u] ", xTickCount);

    // 如果延时链表不为空，只检查排在最前面的第 1 个节点！
    while (xDelayedTaskList.uxNumberOfItems > 0) {
        ListItem_t *pxHead = xDelayedTaskList.xListEnd.pxNext;
        if (xTickCount >= pxHead->xItemValue) {
            TCB_t *pxWokenTCB = (TCB_t *)pxHead->pvOwner;
            printf("叮！任务 [%s] 延时到期唤醒！\n", pxWokenTCB->pcTaskName);

            // 从延时链表移除，重新塞回就绪链表
            uxListRemove(pxHead);
            prvAddTaskToReadyList(pxWokenTCB);
        } else {
            // 第一个节点都没到期，后面的更不可能到期，直接退出！(O(1) 效率)
            break;
        }
    }
    vTaskSwitchContext_Sim();
}

/* ================= 4. 主函数演练 ================= */
/* ================= 4. 主函数演练（多调度扩展版） ================= */
int main(void) {
    system("chcp 65001 > nul");
    vSchedulerInit();

    TCB_t taskIdle, taskLed1, taskLed2, taskHigh, taskLed3, taskLed4, taskHigh2,
        taskLed5, taskLed6, taskHigh3, taskLed7, taskLed8, taskHigh4;
    vTaskCreate_Sim(&taskIdle, "Task_Idle", 0);
    vTaskCreate_Sim(&taskLed1, "Task_LED1", 1);
    vTaskCreate_Sim(&taskLed2, "Task_LED2", 1);
    vTaskCreate_Sim(&taskHigh, "Task_High", 3);
    vTaskCreate_Sim(&taskLed3, "Task_LED3", 1);
    vTaskCreate_Sim(&taskLed4, "Task_LED4", 1);
    vTaskCreate_Sim(&taskHigh2, "Task_High2", 3);
    vTaskCreate_Sim(&taskLed5, "Task_LED5", 1);
    vTaskCreate_Sim(&taskLed6, "Task_LED6", 1);
    vTaskCreate_Sim(&taskHigh3, "Task_High3", 3);
    vTaskCreate_Sim(&taskLed7, "Task_LED7", 1);
    vTaskCreate_Sim(&taskLed8, "Task_LED8", 1);
    vTaskCreate_Sim(&taskHigh4, "Task_High4", 3);

    printf("=== 启动调度器 ===\n");
    vTaskSwitchContext_Sim();          // 选中优先级 3 的 Task_High

    /* ---------- 场景 A：高优先级任务延时让出 CPU ---------- */
    printf("\n===== 场景 A：Task_High 延时 2 tick =====\n");
    vTaskDelay_Sim(2);                 // Task_High 阻塞 -> 落到优先级 1

    /* ---------- 场景 B：连续 Tick，观察优先级 1 的时间片轮转 ---------- */
    printf("\n===== 场景 B：Tick 1~4，优先级 1 轮转（High 未到期）=====\n");
    for (int i = 0; i < 4; i++) {
        xTaskIncrementTick_Sim();      // 每次都会触发一次调度
    }

    /* ---------- 场景 C：让当前任务再延时，制造多个延时任务 ---------- */
    printf("\n===== 场景 C：当前任务延时 3 tick，制造延时队列 =====\n");
    vTaskDelay_Sim(3);

    printf("\n===== 场景 C2：再延时 1 tick，形成多任务不同到期时间 =====\n");
    vTaskDelay_Sim(1);

    /* ---------- 场景 D：连续 Tick，观察到期唤醒 + 抢占 ---------- */
    printf("\n===== 场景 D：Tick 连续推进，观察到期唤醒与抢占 =====\n");
    for (int i = 0; i < 6; i++) {
        xTaskIncrementTick_Sim();
    }

    /* ---------- 场景 E：高优先级任务自己延时，验证位图清零 ---------- */
    printf("\n===== 场景 E：高优先级延时，位图 bit 动态清零 =====\n");
    if (pxCurrentTCB->uxPriority == 3) {
        vTaskDelay_Sim(2);
    } else {
        // 若当前不是高优先级，先切到高优先级再延时
        vTaskSwitchContext_Sim();
        vTaskDelay_Sim(2);
    }

    printf("\n===== 场景 E2：Tick 推进，验证高优先级到期后重新置位 =====\n");
    for (int i = 0; i < 3; i++) {
        xTaskIncrementTick_Sim();
    }

    /* ---------- 场景 F：长时间连续调度，观察稳定轮转 ---------- */
    printf("\n===== 场景 F：连续 10 个 Tick，观察稳定轮转 =====\n");
    for (int i = 0; i < 10; i++) {
        xTaskIncrementTick_Sim();
    }

    printf("\n=== 演练结束 ===\n");
    return 0;
}
