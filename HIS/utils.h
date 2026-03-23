#pragma once
#ifndef UTILS_H
#define UTILS_H

// ==========================================
// 核心工具接口层
// ==========================================

// 1. 安全输入接口 (替代不安全的 scanf，防止死循环和缓冲区溢出)
void safeGetString(char* buffer, int size);
int safeGetInt();
double safeGetDouble();

// 2. 数据持久化(本地 TXT)接口
void loadAllData();
void saveRecordToFile(const char* filename, int type, const char* typeName, const char* patientId, const char* staffId, const char* desc, double cost, int isPaid);

#endif