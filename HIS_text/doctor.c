#define _CRT_SECURE_NO_WARNINGS
#include "doctor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "schedule.h"
#include "utils.h"  
#include "models.h"  
#include "fileio.h"

Doctor* doctorList = NULL;


static void displayAllDoctors() {
    if (doctorList == NULL || doctorList->next == NULL) {
        printf("医生列表为空。\n");
        return;
    }
    printf("\n--- 医生列表 ---\n");
    printf("%-5s %-15s %-15s %-15s %-10s\n", "ID", "姓名", "科室", "职称", "性别");
    Doctor* p = doctorList->next;
    while (p) {
        printf("%-5d %-15s %-15s %-15s %-10s\n", p->id, p->name, p->department, p->title, p->sex);
        p = p->next;
    }
}

static void addDoctor() {
    Doctor d;
    printf("请输入医生ID: ");
    while (scanf("%d", &d.id) != 1) {
        while (getchar() != '\n');
        printf("输入格式错误，请重新输入: ");
    }

    if (doctorList == NULL) { doctorList = (Doctor*)malloc(sizeof(Doctor)); doctorList->next = NULL; }

    Doctor* p = doctorList->next;
    while (p) {
        if (p->id == d.id) { printf("ID已存在！\n"); return; }
        p = p->next;
    }

    printf("请输入姓名: "); scanf("%49s", d.name); while (getchar() != '\n');
    printf("请输入科室: "); scanf("%29s", d.department); while (getchar() != '\n');
    printf("请输入职称: "); scanf("%19s", d.title); while (getchar() != '\n');
    printf("请输入性别（男/女）: ");
    while (1) {
        scanf("%9s", d.sex); while (getchar() != '\n');
        if (strcmp(d.sex, "男") == 0 || strcmp(d.sex, "女") == 0) break;
        printf("无效输入，请输入 '男' 或 '女': ");
    }

    Doctor* node = (Doctor*)malloc(sizeof(Doctor));
    *node = d; node->next = doctorList->next; doctorList->next = node;

    // 【极其关键】：双端挂载！新建医生的同时自动建立Staff登录账号，初始密码 123456
    Staff* sNode = (Staff*)malloc(sizeof(Staff));
    sprintf(sNode->id, "%d", d.id);
    strcpy(sNode->password, "123456");
    strcpy(sNode->name, d.name);
    strcpy(sNode->department, d.department);
    strcpy(sNode->level, d.title);
    sNode->next = staffHead->next;
    staffHead->next = sNode;

    saveDoctors();
    printf("医生添加成功，默认登录密码为 123456。\n");
}

static void deleteDoctor() {
    int id;
    printf("请输入要删除的医生ID: ");
    while (scanf("%d", &id) != 1) {
        while (getchar() != '\n');
        printf("输入格式错误，请重新输入: ");
    }

    if (doctorList == NULL) return;

    Doctor* prev = doctorList; Doctor* curr = doctorList->next;
    while (curr) {
        if (curr->id == id) {
            prev->next = curr->next;
            free(curr);

            // 【修改点】：同步吊销对应 Staff 的登录权限
            char idStr[20]; sprintf(idStr, "%d", id);
            Staff* prevS = staffHead; Staff* currS = staffHead->next;
            while (currS) {
                if (strcmp(currS->id, idStr) == 0) {
                    prevS->next = currS->next;
                    free(currS);
                    break;
                }
                prevS = currS; currS = currS->next;
            }

            deleteScheduleByDoctorId(id);
            printf("档案及登录权限删除成功。\n");
            saveDoctors();
            saveSchedules();
            return;
        }
        prev = curr; curr = curr->next;
    }
    printf("未找到该医生。\n");
}

static void updateDoctor() {
    int id;
    printf("请输入要修改的医生ID: ");
    while (1) {
        if (scanf("%d", &id) == 1) break;
        while (getchar() != '\n');
        printf("输入格式错误，请重新输入: ");
    }

    if (doctorList == NULL) return;

    Doctor* p = doctorList->next;
    while (p) {
        if (p->id == id) {
            // 【修改点】：提取对应映射的登录实体，以同步改动
            char idStr[20]; sprintf(idStr, "%d", p->id);
            Staff* sMatch = NULL;
            for (Staff* st = staffHead->next; st != NULL; st = st->next) {
                if (strcmp(st->id, idStr) == 0) { sMatch = st; break; }
            }

            printf("当前医生信息：\n");
            printf("1. 姓名: %s\n2. 科室: %s\n3. 职称: %s\n4. 性别: %s\n", p->name, p->department, p->title, p->sex);
            printf("请选择要修改的单个字段 (1.姓名 2.科室 3.职称 4.性别 | 0.结束保存): ");

            int ch;
            while (1) {
                ch = safeGetInt();
                if (ch >= 0 && ch <= 4) break;
                printf("  [!] 输入格式不合法，请正确输入菜单中提供的数字编号！\n请重新选择: ");
            }
            if (ch == 0) { printf("修改已取消或结束。\n"); return; }

            switch (ch) {
            case 1:
                printf("请输入新姓名: "); scanf("%49s", p->name); while (getchar() != '\n');
                if (sMatch) strcpy(sMatch->name, p->name); // 双向同步
                break;
            case 2:
                printf("请输入新科室: "); scanf("%29s", p->department); while (getchar() != '\n');
                if (sMatch) strcpy(sMatch->department, p->department); // 双向同步
                break;
            case 3:
                printf("请输入新职称: "); scanf("%19s", p->title); while (getchar() != '\n');
                if (sMatch) strcpy(sMatch->level, p->title); // 双向同步
                break;
            case 4:
                printf("请输入新性别(男/女): ");
                while (1) {
                    scanf("%9s", p->sex); while (getchar() != '\n');
                    if (strcmp(p->sex, "男") == 0 || strcmp(p->sex, "女") == 0) break;
                    printf("无效输入，请输入 '男' 或 '女': ");
                }
                break;
            }
            saveDoctors();
            printf("医生信息修改成功并已同步至门禁。\n");
            return;
        }
        p = p->next;
    }
    printf("未找到该医生。\n");
}


static void queryDoctor() {
    int choice;
    printf("查询方式：1-按ID  2-按姓名模糊 3-按职称 0-返回\n请选择: ");

    // 【修改点】
    while (1) {
        choice = safeGetInt();
        if (choice >= 0 && choice <= 3) break;
        printf("  [!] 输入格式不合法，请正确输入菜单中提供的数字编号！\n请重新选择: ");
    }
    if (choice == 0) return;

    if (doctorList == NULL) return;

    if (choice == 1) {
        int id;
        printf("请输入ID: ");
        while (scanf("%d", &id) != 1) {
            while (getchar() != '\n');
            printf("输入格式错误，请重新输入: ");
        }
        Doctor* p = doctorList->next;
        while (p) {
            if (p->id == id) {
                printf("ID: %d, 姓名: %s, 科室: %s, 职称: %s, 性别: %s\n",
                    p->id, p->name, p->department, p->title, p->sex);
                return;
            }
            p = p->next;
        }
        printf("未找到。\n");
    }
    else if (choice == 2) {
        char name[20];
        printf("请输入姓名关键字: ");
        scanf("%49s", name); while (getchar() != '\n');
        int found = 0;
        Doctor* p = doctorList->next;
        while (p) {
            if (strstr(p->name, name)) {
                if (!found) printf("\n--- 查询结果 ---\n");
                printf("ID: %d, 姓名: %s, 科室: %s, 职称: %s\n",
                    p->id, p->name, p->department, p->title);
                found = 1;
            }
            p = p->next;
        }
        if (!found) printf("未找到。\n");
    }
    else if (choice == 3) {
        char title[20];
        printf("请输入职称关键字: ");
        scanf("%19s", title); while (getchar() != '\n');
        int found = 0;
        Doctor* p = doctorList->next;
        while (p) {
            if (strstr(p->title, title)) {
                if (!found) printf("\n--- 查询结果 ---\n");
                printf("ID: %d, 姓名: %s, 科室: %s, 职称: %s\n",
                    p->id, p->name, p->department, p->title);
                found = 1;
            }
            p = p->next;
        }
        if (!found) printf("未找到。\n");
    }
}

void doctorMenu() {
    int choice;
    do {
        printf("\n===== 医生信息管理 =====\n");
        printf("1. 查看全部医生\n");
        printf("2. 添加医生\n");
        printf("3. 删除医生\n");
        printf("4. 修改医生信息\n");
        printf("5. 查询医生\n");
        printf("0. 返回主菜单\n");
        printf("请选择: ");

        // 【修改点】
        while (1) {
            choice = safeGetInt();
            if (choice >= 0 && choice <= 5) break;
            printf("  [!] 输入格式不合法，请正确输入菜单中提供的数字编号！\n请重新选择: ");
        }

        switch (choice) {
        case 1: displayAllDoctors(); break;
        case 2: addDoctor(); break;
        case 3: deleteDoctor(); break;
        case 4: updateDoctor(); break;
        case 5: queryDoctor(); break;
        case 0: break;
        }
    } while (choice != 0);
}