#pragma once
#ifndef STAFF_H
#define STAFF_H

#include "models.h"

// ==========================================
// 医护端对外的公共接口声明
// ==========================================

// 医护端主菜单 (包含门诊和住院入口，需传入已登录的医生/护士信息)
void staffMenu(User* staff);

#endif