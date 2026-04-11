#pragma once
#ifndef DRUG_H
#define DRUG_H

#include "utils.h"

typedef struct Drug {
    int id;
    char name[50];
    int stock;
    double price;    // 修复：float精度不足，改为double
    char batch[30];
    char expiry[30];
    char last_in[30];
    char last_out[30];
    struct Drug* next;
} Drug;

typedef struct DrugHistory {
    int drug_id;
    int type;           
    int quantity;
    char time[15];
    struct DrugHistory* next;
} DrugHistory;

extern Drug* drugList;
extern DrugHistory* drugHistoryList;

void drugMenu(void);
void initDrugList();

#endif

