#define _CRT_SECURE_NO_WARNINGS
#include "schedule.h"
#include "doctor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "time_t.h"
#include "fileio.h"
#include "utils.h" // 【核心】引入安全工具库

Schedule* scheduleList;

//---------------------------------------------------------
// 删除医生排班，当医生被删除时调用
//---------------------------------------------------------
void deleteScheduleByDoctorId(int doctorId) {
    Schedule* prev = NULL;
    Schedule* curr = scheduleList->next;
    while (curr != NULL) {
        if (curr->doctor_id == doctorId) {
            Schedule* temp = curr;
            if (prev == NULL) {
                scheduleList->next = curr->next;
            }
            else {
                prev->next = curr->next;
            }
            curr = curr->next;
            free(temp);
        }
        else {
            prev = curr;
            curr = curr->next;
        }
    }
    saveSchedules();
}

//--------------------------------------------------------------
// 检查排班是否冲突
//--------------------------------------------------------------
static int checkConflict(int doctor_id, char* date) {
    Schedule* s = scheduleList->next;
    while (s) {
        if (s->doctor_id == doctor_id && strcmp(s->date, date) == 0) return 1;
        s = s->next;
    }
    return 0;
}

//-----------------------------------------------------------------
// 查看排班
//-----------------------------------------------------------------
static void viewSchedule() {
    char start[20], end[20];
    printf("请输入开始日期 (YYYY-MM-DD, 输入0取消): ");
    judgetime(start);
    if (strcmp(start, "0") == 0) return;

    printf("请输入结束日期 (YYYY-MM-DD, 输入0取消): ");
    judgetime(end);
    if (strcmp(end, "0") == 0) return;

    printf("\n--- 排班表 (%s 至 %s) ---\n", start, end);
    printf("%-8s | %-12s | %-15s | %-12s | %-10s\n", "排班ID", "日期", "医生姓名", "科室", "班次");
    printf("----------------------------------------------------------------------\n");

    Schedule* s = scheduleList->next;
    int found = 0;
    while (s) {
        if (strcmp(s->date, start) >= 0 && strcmp(s->date, end) <= 0) {
            Doctor* d = doctorList->next;
            char docName[50] = "未知";
            char docDept[30] = "未知";
            while (d) {
                if (d->id == s->doctor_id) {
                    strcpy(docName, d->name);
                    strcpy(docDept, d->department);
                    break;
                }
                d = d->next;
            }
            printf("%-8d | %-12s | %-15s | %-12s | %-10s\n", s->schedule_id, s->date, docName, docDept, s->shift);
            found = 1;
        }
        s = s->next;
    }
    if (!found) printf("  [!] 该时间段内暂无任何排班记录。\n");
    system("pause");
}

//-------------------------------------------------------------------------------
// 添加排班
//-------------------------------------------------------------------------------
static void addSchedule() {
    int doc_id;
    char date[20], shift[20];

    printf("请输入要排班的医生ID (输入0取消): ");
    doc_id = safeGetPositiveInt();
    if (doc_id == 0) return;

    // 校验医生是否存在
    Doctor* d = doctorList->next;
    int exists = 0;
    while (d) {
        if (d->id == doc_id) { exists = 1; break; }
        d = d->next;
    }
    if (!exists) {
        printf("  [!] 错误：医生ID [%d] 在系统中不存在！\n", doc_id);
        system("pause");
        return;
    }

    printf("请输入出诊日期 (YYYY-MM-DD): ");
    judgetime(date);

    if (checkConflict(doc_id, date)) {
        printf("  [!] 冲突：该医生在该日期已有排班记录，请勿重复添加！\n");
        system("pause");
        return;
    }

    // 【修改点】：强制校验班次输入
    while (1) {
        printf("请输入班次 (早班/晚班/休息): ");
        safeGetString(shift, 20);
        if (strcmp(shift, "早班") == 0 || strcmp(shift, "晚班") == 0 || strcmp(shift, "休息") == 0) {
            break;
        }
        printf("  [!] 输入无效：班次只能是 '早班'、'晚班' 或 '休息'，请重新输入。\n");
    }

    Schedule* node = (Schedule*)malloc(sizeof(Schedule));
    int max_id = 0;
    Schedule* temp = scheduleList->next;
    while (temp) {
        if (temp->schedule_id > max_id) max_id = temp->schedule_id;
        temp = temp->next;
    }

    node->schedule_id = max_id + 1;
    node->doctor_id = doc_id;
    strcpy(node->date, date);
    strcpy(node->shift, shift);
    node->next = NULL;

    // 挂载到链表尾部
    Schedule* p = scheduleList;
    while (p->next) p = p->next;
    p->next = node;

    saveSchedules();
    printf("  [√] 排班添加成功！系统分配排班唯一流水号: 【 %d 】\n", node->schedule_id);
    system("pause");
}

//-------------------------------------------------------------------------------
// 删除排班
//-------------------------------------------------------------------------------
static void deleteSchedule() {
    printf("请输入要删除的排班ID (输入0取消): ");
    int sid = safeGetPositiveInt();
    if (sid == 0) return;

    Schedule* prev = scheduleList;
    Schedule* curr = scheduleList->next;
    while (curr) {
        if (curr->schedule_id == sid) {
            prev->next = curr->next;
            free(curr);
            saveSchedules();
            printf("  [√] 排班记录删除成功。\n");
            system("pause");
            return;
        }
        prev = curr;
        curr = curr->next;
    }
    printf("  [!] 定位失败：未找到排班ID为 [%d] 的记录。\n", sid);
    system("pause");
}

//-------------------------------------------------------------------------------
// 修改排班
//-------------------------------------------------------------------------------
static void modifySchedule() {
    printf("请输入需调整的排班ID (输入0取消): ");
    int sid = safeGetPositiveInt();
    if (sid == 0) return;

    Schedule* p = scheduleList->next;
    while (p) {
        if (p->schedule_id == sid) {
            printf("\n--- 原始排班信息 ---\n");
            printf("医生ID: %d | 日期: %s | 班次: %s\n", p->doctor_id, p->date, p->shift);
            printf("--------------------\n");

            int new_id;
            char new_date[20], new_shift[20];

            printf("请输入新的医生ID: ");
            while (1) {
                new_id = safeGetPositiveInt();
                Doctor* doc = doctorList->next;
                int doc_exist = 0;
                while (doc) {
                    if (doc->id == new_id) { doc_exist = 1; break; }
                    doc = doc->next;
                }
                if (doc_exist) break;
                printf("  [!] 该医生ID不存在，请重新输入有效的医生ID: ");
            }

            printf("请输入新的日期 (YYYY-MM-DD): ");
            judgetime(new_date);

            // 排班冲突检查（排除当前正在修改的这一条）
            int conflict = 0;
            Schedule* s_check = scheduleList->next;
            while (s_check) {
                if (s_check != p && s_check->doctor_id == new_id && strcmp(s_check->date, new_date) == 0) {
                    conflict = 1; break;
                }
                s_check = s_check->next;
            }
            if (conflict) {
                printf("  [!] 修改失败：该医生在 %s 已有其他排班任务。\n", new_date);
                system("pause");
                return;
            }

            while (1) {
                printf("请输入新班次 (早班/晚班/休息): ");
                safeGetString(new_shift, 20);
                if (strcmp(new_shift, "早班") == 0 || strcmp(new_shift, "晚班") == 0 || strcmp(new_shift, "休息") == 0) {
                    break;
                }
                printf("  [!] 输入无效，请重新输入正确的班次名称。\n");
            }

            // 应用修改
            p->doctor_id = new_id;
            strcpy(p->date, new_date);
            strcpy(p->shift, new_shift);

            saveSchedules();
            printf("  [√] 排班信息已成功更新并保存。\n");
            system("pause");
            return;
        }
        p = p->next;
    }
    printf("  [!] 错误：未找到该排班ID。\n");
    system("pause");
}

//-------------------------------------------------------------------------------
// 排班菜单
//-------------------------------------------------------------------------------
void scheduleMenu() {
    int choice;
    do {
        system("cls");
        printf("\n========== 医院临床医生排班调度中心 ==========\n");
        printf("  [1] 全院排班视图检索\n");
        printf("  [2] 发布新排班计划\n");
        printf("  [3] 撤回已发布排班\n");
        printf("  [4] 调整现有排班明细\n");
        printf("  [0] 返回高管业务大厅\n");
        printf("----------------------------------------------\n");
        printf("  请选择业务: ");

        // 【核心点】：使用 safeGetInt 并强制循环拦截
        while (1) {
            choice = safeGetInt();
            if (choice >= 0 && choice <= 4) break;
            printf("  [!] 输入格式不合法，请正确输入菜单中提供的数字编号！\n  请重新选择业务: ");
        }

        switch (choice) {
        case 1: viewSchedule(); break;
        case 2: addSchedule(); break;
        case 3: deleteSchedule(); break;
        case 4: modifySchedule(); break;
        case 0: break;
        }
    } while (choice != 0);
}