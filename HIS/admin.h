#ifndef ADMIN_H
#define ADMIN_H

typedef struct Admin {
    char username[20];
    char password[20];
    char phone[11];
    char email[30];
} Admin;

extern Admin admin;

void loadAdminData(void);
void saveAdminData(void);
void changePassword(void);
void editPersonalInfo(void);
void personalMenu(void);

#endif
