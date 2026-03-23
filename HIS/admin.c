#define _CRT_SECURE_NO_WARNINGS
#include "admin.h"
#include <stdio.h>
#include <string.h>

Admin admin;

void loadAdminData(void) {
    FILE* fp = fopen("admin.txt", "r");
    if (!fp)
        return;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;
        char* token = strtok(line, ",");
        if (token) strcpy(admin.username, token); else admin.username[0] = '\0';
        token = strtok(NULL, ",");
        if (token) strcpy(admin.password, token); else admin.password[0] = '\0';
        token = strtok(NULL, ",");
        if (token) strcpy(admin.phone, token); else admin.phone[0] = '\0';
        token = strtok(NULL, ",");
        if (token) strcpy(admin.email, token); else admin.email[0] = '\0';
    }
    fclose(fp);
}

void saveAdminData(void) {
    FILE* fp = fopen("admin.txt", "w");
    fprintf(fp, "%s,%s,%s,%s\n", admin.username, admin.password, admin.phone, admin.email);
    fclose(fp);
}

void changePassword(void) {
    char old[20] = { '\0' }, new1[20] = { '\0' }, new2[20] = { '\0' };
    for (int i = 0; i < 20; i++) {
        old[i] = '\0';
        new1[i] = '\0';
        new2[i] = '\0';
    }
    printf("请输入旧密码: ");

    scanf("%19s", old);
    if (old[19] != '\0') {
        printf("旧密码错误\n");
        return;
    }
    if (strcmp(old, admin.password) != 0) {
        printf("旧密码错误！\n");
        return;
    }
    printf("请输入新密码: ");
    scanf("%19s", new1);
    if (new1[19] != '\0') {
        printf("输入过长\n");
        return;
    }
    printf("请确认新密码: ");
    scanf("%19s", new2);
    if (new2[19] != '\0') {
        printf("两次输入不一致！\n");
        return;
    }
    if (strcmp(new1, new2) != 0) {
        printf("两次输入不一致！\n");
        return;
    }
    strcpy(admin.password, new1);
    printf("密码修改成功！\n");
}

void editPersonalInfo(void) {
    printf("当前信息：\n");
    printf("用户名: %s\n", admin.username);
    printf("手机号: %s\n", admin.phone);
    printf("邮箱: %s\n", admin.email);
    printf("请输入新手机号（直接回车保留原值）: ");
    getchar(); // 清除缓冲区
    char newPhone[15];
    fgets(newPhone, 15, stdin);
    if (newPhone[0] != '\n') {
        newPhone[strcspn(newPhone, "\n")] = 0;
        strcpy(admin.phone, newPhone);
    }
    printf("请输入新邮箱（直接回车保留原值）: ");
    char newEmail[30];
    fgets(newEmail, 30, stdin);
    if (newEmail[0] != '\n') {
        newEmail[strcspn(newEmail, "\n")] = 0;
        strcpy(admin.email, newEmail);
    }
    printf("个人信息更新成功！\n");
}

void personalMenu() {
    int choice;
    do {
        printf("\n========== 个人设置 ==========\n");
        printf("1. 修改密码\n");
        printf("2. 个人信息编辑\n");
        printf("0. 返回主菜单\n");
        printf("请选择: ");
        scanf("%d", &choice);
        switch (choice) {
        case 1: changePassword(); break;
        case 2: editPersonalInfo(); break;
        case 0: break;
        default: printf("无效选项。\n");
        }
    } while (choice != 0);
}