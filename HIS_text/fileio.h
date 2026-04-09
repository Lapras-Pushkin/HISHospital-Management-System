#pragma once
#ifndef FILEIO_H
#define FILEIO_H

// 1. 全局基础数据读写 (原位于 utils.c)
void loadAllDataFromTxt();
void saveAllDataToTxt();

// 2. 药品与库存读写 (原位于 drug.c)
void loadDrugs();
void saveDrugs();
void loadDrugHistory();
void saveDrugHistory();

// 3. 医生档案读写 (原位于 doctor.c)
void loadDoctors();
void saveDoctors();

// 4. 管理员数据读写 (原位于 admin.c)
void loadAdminData();
void saveAdminData();

// 5. 排班与流水读写 (原位于 schedule.c 和 transaction.c)
void loadSchedules();
void saveSchedules();
void loadTransactions();
void saveTransactions();

#endif
