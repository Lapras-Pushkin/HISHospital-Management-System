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
        printf("  [!] 医生列表为空。\n");
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
    printf("请输入医生ID (必须为纯数字): ");
    while (1) {
        d.id = safeGetPositiveInt();
        if (d.id == 0) return; // 0代表取消

        int exists = 0;
        for (Doctor* p = doctorList->next; p != NULL; p = p->next) {
            if (p->id == d.id) { exists = 1; break; }
        }
        if (!exists) break;
        printf("  [!] 该ID已存在！请重新输入新的医生ID: ");
    }

    printf("请输入姓名: "); safeGetString(d.name, 50);
    printf("请输入科室: "); safeGetString(d.department, 30);
    printf("请输入职称: "); safeGetString(d.title, 20);
    printf("请输入性别（男/女）: "); safeGetGender(d.sex, 10);

    Doctor* node = (Doctor*)malloc(sizeof(Doctor));
    *node = d; node->next = doctorList->next; doctorList->next = node;

    Staff* sNode = (Staff*)malloc(sizeof(Staff));
    sprintf(sNode->id, "%d", d.id);
    strcpy(sNode->password, "123456");
    strcpy(sNode->name, d.name);
    strcpy(sNode->department, d.department);
    strcpy(sNode->level, d.title);
    sNode->next = staffHead->next;
    staffHead->next = sNode;

    saveDoctors();
    printf("  [√] 医生添加成功，默认登录密码为 123456。\n");
    system("pause");
}

static void deleteDoctor() {
    printf("请输入要删除的医生ID (输入0取消): ");
    int id = safeGetPositiveInt();
    if (id == 0 || doctorList == NULL) return;

    Doctor* prev = doctorList; Doctor* curr = doctorList->next;
    while (curr) {
        if (curr->id == id) {
            prev->next = curr->next;
            free(curr);

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
            printf("  [√] 档案及登录权限删除成功。\n");
            saveDoctors();
            saveSchedules();
            system("pause");
            return;
        }
        prev = curr; curr = curr->next;
    }
    printf("  [!] 未找到该医生。\n");
    system("pause");
}

static void updateDoctor() {
    printf("请输入要修改的医生ID (输入0取消): ");
    int id = safeGetPositiveInt();
    if (id == 0 || doctorList == NULL) return;

    Doctor* p = doctorList->next;
    while (p) {
        if (p->id == id) {
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
            if (ch == 0) return;

            switch (ch) {
            case 1:
                printf("请输入新姓名: "); safeGetString(p->name, 50);
                if (sMatch) strcpy(sMatch->name, p->name);
                break;
            case 2:
                printf("请输入新科室: "); safeGetString(p->department, 30);
                if (sMatch) strcpy(sMatch->department, p->department);
                break;
            case 3:
                printf("请输入新职称: "); safeGetString(p->title, 20);
                if (sMatch) strcpy(sMatch->level, p->title);
                break;
            case 4:
                printf("请输入新性别(男/女): "); safeGetGender(p->sex, 10);
                break;
            }
            saveDoctors();
            printf("  [√] 医生信息修改成功并已同步至门禁。\n");
            system("pause");
            return;
        }
        p = p->next;
    }
    printf("  [!] 未找到该医生。\n");
    system("pause");
}

static void queryDoctor() {
    int choice;
    printf("\n查询方式：1-按ID  2-按姓名模糊 3-按职称 0-返回\n请选择: ");

    while (1) {
        choice = safeGetInt();
        if (choice >= 0 && choice <= 3) break;
        printf("  [!] 输入格式不合法，请正确输入菜单中提供的数字编号！\n请重新选择: ");
    }
    if (choice == 0) return;

    if (doctorList == NULL) return;

    if (choice == 1) {
        printf("请输入医生ID: ");
        int id = safeGetPositiveInt();
        Doctor* p = doctorList->next;
        while (p) {
            if (p->id == id) {
                printf("ID: %d, 姓名: %s, 科室: %s, 职称: %s, 性别: %s\n",
                    p->id, p->name, p->department, p->title, p->sex);
                system("pause");
                return;
            }
            p = p->next;
        }
        printf("  [!] 未找到匹配记录。\n");
    }
    else if (choice == 2) {
        char name[50];
        printf("请输入姓名关键字: ");
        safeGetString(name, 50);
        int found = 0;
        Doctor* p = doctorList->next;
        while (p) {
            if (strstr(p->name, name)) {
                if (!found) printf("\n--- 查询结果 ---\n");
                printf("ID: %d, 姓名: %s, 科室: %s, 职称: %s\n", p->id, p->name, p->department, p->title);
                found = 1;
            }
            p = p->next;
        }
        if (!found) printf("  [!] 未找到匹配记录。\n");
    }
    else if (choice == 3) {
        char title[50];
        printf("请输入职称关键字: ");
        safeGetString(title, 50);
        int found = 0;
        Doctor* p = doctorList->next;
        while (p) {
            if (strstr(p->title, title)) {
                if (!found) printf("\n--- 查询结果 ---\n");
                printf("ID: %d, 姓名: %s, 科室: %s, 职称: %s\n", p->id, p->name, p->department, p->title);
                found = 1;
            }
            p = p->next;
        }
        if (!found) printf("  [!] 未找到匹配记录。\n");
    }
    system("pause");
}

void doctorMenu() {
    int choice;
    do {
        system("cls");
        printf("\n===== 医生信息管理 =====\n");
        printf("1. 查看全部医生\n");
        printf("2. 添加医生\n");
        printf("3. 删除医生\n");
        printf("4. 修改医生信息\n");
        printf("5. 查询医生\n");
        printf("0. 返回主菜单\n");
        printf("请选择: ");

        while (1) {
            choice = safeGetInt();
            if (choice >= 0 && choice <= 5) break;
            printf("  [!] 输入格式不合法，请正确输入菜单中提供的数字编号！\n请重新选择: ");
        }

        switch (choice) {
        case 1: displayAllDoctors(); system("pause"); break;
        case 2: addDoctor(); break;
        case 3: deleteDoctor(); break;
        case 4: updateDoctor(); break;
        case 5: queryDoctor(); break;
        case 0: break;
        }
    } while (choice != 0);
}