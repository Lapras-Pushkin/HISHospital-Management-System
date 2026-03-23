#pragma once
#ifndef USER_H
#define USER_H

#include "models.h"

// ==========================================
// 患者端对外的公共接口声明
// ==========================================

// 患者端主菜单 (需传入已登录的用户信息)
void patientMenu(User* user);

// 患者自助注册与建档功能
void patientRegister();

#endif