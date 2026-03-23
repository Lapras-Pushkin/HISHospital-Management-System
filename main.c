#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "admin.h"
#include "drug.h"
#include "doctor.h"
#include "schedule.h"
#include "transaction.h"
#include "decision.h"

int main() {
    // 加载数据
    loadDrugs();
    loadDrugHistory();
    loadDoctors();
    loadSchedules();
    loadTransactions();
    loadAdminData();

    // 登录验证
	//默认为 admin 用户，密码为 123456
    char username[20], password[20];
    printf("========== 管理端系统 ==========\n");
    printf("用户名: ");
    scanf("%s", username);
    printf("密码: ");
    scanf("%s", password);
    if (strcmp(username, admin.username) != 0 || strcmp(password, admin.password) != 0) {
        printf("用户名或密码错误！\n");
        return 1;
    }
    printf("登录成功！\n");

    int choice;
    do {
        printf("\n========== 管理端系统主菜单 ==========\n");
        printf("1. 药房管理\n");
        printf("2. 医生排班管理\n");
        printf("3. 统计报表中心\n");
        printf("4. 个人设置\n");
        printf("5. 智能辅助决策\n");
        printf("0. 退出系统\n");
        printf("请选择: ");
        scanf("%d", &choice);
        switch (choice) {
        case 1: drugMenu(); break;
        case 2: scheduleMenu(); break;
        case 3: reportMenu(); break;
        case 4: personalMenu(); break;
        case 5: decisionMenu(); break;
        case 0: break;
        default: printf("无效选项。\n");
        }
    } while (choice != 0);

    // 保存数据
    saveDrugs();
    saveDrugHistory();
    saveDoctors();
    saveSchedules();
    saveTransactions();
    saveAdminData();

    // 释放内存（可选，程序结束时会自动回收）
    // 实际可添加链表释放函数，但此处省略

    printf("系统已退出，数据已保存。\n");
    return 0;
}