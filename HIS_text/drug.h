#pragma once
#ifndef DRUG_H
#define DRUG_H

#include "utils.h"

typedef struct Drug {
    int id;
    char name[50];
    int stock;
    float price;
    char batch[30];
    char expiry[30];
    char last_in[30];
    char last_out[30];
    struct Drug* next;
} Drug;

typedef struct DrugHistory {
    int drug_id;
    int type;           // 1=���, 2=����
    int quantity;
    char time[15];
    struct DrugHistory* next;
} DrugHistory;

extern Drug* drugList;
extern DrugHistory* drugHistoryList;

void drugMenu(void);
static int isDrugIdExists(int id);
void initDrugList();

#endif

