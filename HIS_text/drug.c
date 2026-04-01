#define _CRT_SECURE_NO_WARNINGS
#include "drug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 锟斤拷锟斤拷锟皆硷拷锟斤拷锟斤拷墓锟斤拷锟斤拷硕锟斤拷锟揭╋拷锟斤拷锟斤拷锟斤拷锟斤拷锟绞凤拷锟铰硷拷锟斤拷锟?
Drug* drugList = NULL;
DrugHistory* drugHistoryList = NULL;

// ---------------------------------------------------------
// 锟接憋拷锟截硷拷锟截癸拷锟斤拷锟剿碉拷药品锟斤拷锟斤拷
// ---------------------------------------------------------
void loadDrugs() {
    FILE* fp = fopen("drugs.txt", "r");
    if (!fp) return;  // 锟侥硷拷锟斤拷锟斤拷锟节ｏ拷锟斤拷涌锟斤拷锟斤拷锟斤拷锟绞?

    char line[512];
    Drug d;
    Drug* tail = NULL;
    // 锟斤拷锟叫讹拷取锟斤拷使锟斤拷 strtok 锟斤拷锟秸讹拷锟脚分革拷锟斤拷锟斤拷侄锟?
    while (fgets(line, sizeof(line), fp)) {
        // 去锟斤拷锟斤拷尾锟斤拷锟叫凤拷
        line[strcspn(line, "\n")] = 0;

        char* token = strtok(line, ",");
        if (token) strcpy(d.id,token); else d.id[0] = '\0';
        token = strtok(NULL, ",");
        if (token) strcpy(d.name, token); else d.name[0] = '\0';//����
        token = strtok(NULL, ",");
		if (token) d.stock = atoi(token); else d.stock = 0;//���
        token = strtok(NULL, ",");
		if (token) d.price = atof(token); else d.price = 0.0;//�۸�
        token = strtok(NULL, ",");
		if (token) strcpy(d.batch, token); else d.batch[0] = '\0';//����
        token = strtok(NULL, ",");
		if (token) strcpy(d.expiry, token); else d.expiry[0] = '\0';//��Ч��
        token = strtok(NULL, ",");
		if (token) strcpy(d.last_in, token); else d.last_in[0] = '\0';//������ʱ��
        token = strtok(NULL, ",");
		if (token) strcpy(d.last_out, token); else d.last_out[0] = '\0';//�������ʱ��

        // 尾锟藉法锟斤拷锟斤拷药品锟斤拷锟斤拷
        Drug* node = (Drug*)malloc(sizeof(Drug));
        *node = d;
        node->next = NULL;
        if (!drugList) drugList = tail = node;
        else { tail->next = node; tail = node; }
    }
    fclose(fp);
}

// ---------------------------------------------------------
// 锟斤拷锟斤拷锟斤拷锟斤拷说锟揭┢凤拷锟斤拷莸锟斤拷锟斤拷锟?
// ---------------------------------------------------------
void saveDrugs() {
    FILE* fp = fopen("drugs.txt", "w");
    if (!fp) return;
    Drug* p = drugList;
    while (p) {
        fprintf(fp, "%s,%s,%d,%.2f,%s,%s,%s,%s\n",
            p->id, p->name, p->stock, p->price,
            p->batch, p->expiry, p->last_in, p->last_out);
        p = p->next;
    }
    fclose(fp);
}

// --------------------------------------------------------
// ����ҩƷ�����䶯��ʷ��¼
// --------------------------------------------------------
void loadDrugHistory() {
    FILE* fp = fopen("drug_history.txt", "r");
    if (!fp) return;

    char line[512];
    DrugHistory h;
    DrugHistory* tail = NULL;
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;
        char* token = strtok(line, ",");
        if (token) strcpy(h.drug_id,token); else h.drug_id[0] = '\0';
        token = strtok(NULL, ",");
        if (token) h.type = atoi(token); else h.type = 0;
        token = strtok(NULL, ",");
        if (token) h.quantity = atoi(token); else h.quantity = 0;
        token = strtok(NULL, ",");
        if (token) strcpy(h.time, token); else h.time[0] = '\0';

        DrugHistory* node = (DrugHistory*)malloc(sizeof(DrugHistory));
        *node = h;
        node->next = NULL;
        if (!drugHistoryList) drugHistoryList = tail = node;
        else { tail->next = node; tail = node; }
    }
    fclose(fp);
}

// ---------------------------------------------------------
// 锟斤拷锟斤拷药品锟斤拷锟斤拷锟戒动锟斤拷史锟斤拷录
// ---------------------------------------------------------
void saveDrugHistory() {
    FILE* fp = fopen("drug_history.txt", "w");
    if (!fp) return;
    DrugHistory* p = drugHistoryList;
    while (p) {
        fprintf(fp, "%s,%d,%d,%s\n", p->drug_id, p->type, p->quantity, p->time);
        p = p->next;
    }
    fclose(fp);
}

// ---------------------------------------------------------
// 锟节诧拷锟斤拷锟竭ｏ拷锟斤拷式锟斤拷锟斤拷印锟斤拷锟斤拷药品锟斤拷息
// ---------------------------------------------------------
static void displayAllDrugs() {
    if (!drugList) {
        printf("药品锟斤拷为锟秸★拷\n");
        return;
    }
    printf("\n--- ҩƷ�б� ---\n");
    printf("%-15s %-20s %-10s %-8s %-12s %-12s %-20s %-20s\n", "ID", "����", "���", "�۸�", "����", "��Ч��", "������", "�������");
    Drug* p = drugList;
    while (p) {
        printf("%-15s %-20s %-10d %-8.2f %-12s %-12s %-20s %-20s\n",
            p->id, p->name, p->stock, p->price, p->batch, p->expiry,
            p->last_in, p->last_out);
        p = p->next;
    }
}

// ---------------------------------------------------------
// 业锟斤拷一锟斤拷药品锟斤拷锟斤拷维锟饺诧拷询
// ---------------------------------------------------------
static void drugStockQuery() {
    int choice;
    printf("\n药品锟斤拷锟斤拷询:\n1-锟斤拷ID锟斤拷询\n2-锟斤拷锟斤拷锟斤拷模锟斤拷锟斤拷询\n3-锟介看锟斤拷锟斤拷药品\n锟斤拷选锟斤拷: ");
    if(scanf("%d", &choice) != 1) {
        choice = -1;
    }

    // 锟斤拷锟斤拷确ID锟斤拷锟斤拷
    if (choice == 1) {
        char id[15];
        for(int i=0;i<15;i++)   
			id[i]= '\0'; // ȷ���ַ�����β]
        printf("������ҩƷID: "); scanf("%s", id);
        if (id[14] != '\0') {
            printf("δ�ҵ���ҩƷ��\n");
            return ;
        }
        Drug* p = drugList;
        while (p) {
            if (strcmp(p->id,id)==0){
                printf("ID:%s ����:%s ���:%d �۸�:%.2f ����:%s ��Ч��:%s\n",
                    p->id, p->name, p->stock, p->price, p->batch, p->expiry);
                return;
            }
            p = p->next;
        }
        printf("未锟揭碉拷锟斤拷药品锟斤拷\n");
    }
    // 锟斤拷锟斤拷锟狡关硷拷锟斤拷模锟斤拷锟斤拷锟斤拷
    else if (choice == 2) {
        char name[50]; printf("锟斤拷锟斤拷锟斤拷药品锟斤拷锟狡关硷拷锟斤拷: "); scanf("%s", name);
        int found = 0;
        Drug* p = drugList;
        while (p) {
            if (strstr(p->name, name)) {
                printf("ID:%d 锟斤拷锟斤拷:%s 锟斤拷锟?%d 锟桔革拷:%.2f 锟斤拷锟斤拷:%s 锟斤拷效锟斤拷:%s\n",
                    p->id, p->name, p->stock, p->price, p->batch, p->expiry);
                found = 1;
            }
            p = p->next;
        }
        if (!found) printf("未锟揭碉拷匹锟斤拷药品锟斤拷\n");
    }
    else if (choice == 3) {
        displayAllDrugs();
    }
    else printf("锟斤拷效选锟斤拷\n");
}

// ---------------------------------------------------------
// 业锟斤拷锟斤拷锟斤拷榭达拷锟绞凤拷锟斤拷锟斤拷水锟斤拷录
// ---------------------------------------------------------
static void viewStockRecords() {
    printf("\n--- 锟斤拷锟戒动锟斤拷录 ---\n");
    DrugHistory* h = drugHistoryList;
    if (!h) { printf("���޼�¼��\n"); return; }
    printf("%-15s %-6s %-8s %-20s\n", "ҩƷID", "����", "����", "ʱ��");
    while (h) {
        printf("%-15s %-6s %-8d %-20s\n", h->drug_id,
            (h->type == 1) ? "���" : "����", h->quantity, h->time);
        h = h->next;
    }
}

// ---------------------------------------------------------
// 业锟斤拷锟斤拷锟斤拷药品锟斤拷锟斤拷锟斤拷 (锟斤拷锟接匡拷娌拷锟铰硷拷锟剿?
// ---------------------------------------------------------
static void drugIn() {
    int  quantity;
    char id[15];
    printf("������ҩƷID: ");
    id[14] = '\0'; // ȷ���ַ�����β
    scanf("%s", id);
    if (id[14]!='\0'){
		printf("δ�ҵ���ҩƷ��\n");
        return;
    }
    Drug* p = drugList;
    while (p) {
        if (strcmp(p->id,id)==0) {
            printf("��ǰ���: %d\n", p->stock);
            printf("�������������: ");
            scanf("%d", &quantity);
            if (quantity <= 0) { printf("锟斤拷锟斤拷锟斤拷锟斤拷为锟斤拷锟斤拷\n"); return; }

            p->stock += quantity; // 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟?
            getCurrentTime(p->last_in, 30); // 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷时锟斤拷

            // 锟斤拷锟斤拷头锟藉法锟斤拷锟戒动锟斤拷录锟斤拷锟斤拷锟斤拷史锟斤拷水锟斤拷锟斤拷
            DrugHistory* h = (DrugHistory*)malloc(sizeof(DrugHistory));
            strcpy(h->drug_id,id);
            h->type = 1;
            h->quantity = quantity;
            getCurrentTime(h->time, 30);
            h->next = drugHistoryList;
            drugHistoryList = h;
            printf("锟斤拷锟缴癸拷锟斤拷锟铰匡拷锟? %d\n", p->stock);
            return;
        }
        p = p->next;
    }
    printf("未锟揭碉拷锟斤拷药品锟斤拷\n");
}

// ---------------------------------------------------------
// 业锟斤拷锟侥ｏ拷药品锟剿癸拷锟斤拷锟斤拷锟斤拷锟?
// ---------------------------------------------------------
static void drugOut() {
    int  quantity;
    char id[15];
    printf("������ҩƷID: ");
    id[14] = '\0'; // ȷ���ַ�����β
    scanf("%s", id);
    if (id[14] != '\0') {
        printf("δ�ҵ���ҩƷ��\n");
        return;
	}//��ֹ������������ڴ�Խ��
    Drug* p = drugList;
    while (p) {
        if (strcmp(p->id,id)==0) {
            printf("��ǰ���: %d\n", p->stock);
            printf("�������������: ");
            scanf("%d", &quantity);
            if (quantity <= 0) { printf("锟斤拷锟斤拷锟斤拷锟斤拷为锟斤拷锟斤拷\n"); return; }
            if (p->stock < quantity) { printf("锟斤拷娌伙拷悖n"); return; }

            p->stock -= quantity; // 锟桔硷拷锟斤拷锟斤拷锟斤拷锟?
            getCurrentTime(p->last_out, 30);

            // 锟斤拷锟斤拷锟斤拷水锟斤拷录
            DrugHistory* h = (DrugHistory*)malloc(sizeof(DrugHistory));
            strcpy(h->drug_id ,id);
            h->type = 2; // 2��������
            h->quantity = quantity;
            getCurrentTime(h->time, 30);
            h->next = drugHistoryList;
            drugHistoryList = h;
            printf("锟斤拷锟斤拷晒锟斤拷锟斤拷驴锟斤拷: %d\n", p->stock);
            return;
        }
        p = p->next;
    }
    printf("δ�ҵ���ҩƷ��\n");
}
// ---------------------------------------------------------
// ҵ���壺������ҩƷ��Ϣ
//----------------------------------------------------------
void addDrug() {
    Drug d;
    printf("��������ҩƷID: ");
    d.id[14] = '\0'; // ȷ���ַ�����β
    scanf("%s", d.id);
    if (d.id[14] != '\0') {
        printf("ҩƷID����������ʧ�ܡ�\n");
        return;
    }
    printf("������ҩƷ����: ");
    scanf("%20s", d.name);
    printf("�������ʼ�������: ");
    while (1) {
        if (scanf("%d", &d.stock) == 1)
            break;
        while (getchar() != '\n');
           printf("����Ĳ������֣�����������: ");
    }
    printf("������ҩƷ�۸�: ");
    while (1) {
        if (scanf("%f", &d.price));
            break;
        while (getchar() != '\n');
        printf("����Ĳ������֣�����������: ");
    }
    printf("����������: ");
    scanf("%s", d.batch);
    printf("��������Ч�� (��XXXX-YY-ZZ): ");
	judgetime(d.expiry);// ��֤���ڸ�ʽ
    getCurrentTime(d.last_in, 30); // ��ʼ���ʱ��Ϊ��
    strcpy(d.last_out, ""); // ��ʼ����ʱ��Ϊ��
    // ����ҩƷ���ӵ�����ĩβ
    Drug* node = (Drug*)malloc(sizeof(Drug));
    *node = d;
    node->next = NULL;
    if (!drugList) drugList = node;
    else {
        Drug* p = drugList;
        while (p->next) p = p->next;
        p->next = node;
    }
    printf("��ҩƷ���ӳɹ���\n");
}
// ---------------------------------------------------------
// ҵ���壺������ҩƷ��Ϣ
//----------------------------------------------------------
void addDrug() {
    Drug d;
    printf("��������ҩƷID: ");
    d.id[14] = '\0'; // ȷ���ַ�����β
    scanf("%s", d.id);
    if (d.id[14] != '\0') {
        printf("ҩƷID����������ʧ�ܡ�\n");
        return;
    }
    printf("������ҩƷ����: ");
    scanf("%s", d.name);
    printf("�������ʼ�������: ");
    scanf("%d", &d.stock);
    printf("������ҩƷ�۸�: ");
    scanf("%f", &d.price);
    printf("����������: ");
    scanf("%s", d.batch);
    printf("��������Ч�� (��2025-12-31): ");
    scanf("%s", d.expiry);
    getCurrentTime(d.last_in, 30); // ��ʼ���ʱ��Ϊ��
    strcpy(d.last_out, ""); // ��ʼ����ʱ��Ϊ��
    // ����ҩƷ���ӵ�����ĩβ
    Drug* node = (Drug*)malloc(sizeof(Drug));
    *node = d;
    node->next = NULL;
    if (!drugList) drugList = node;
    else {
        Drug* p = drugList;
        while (p->next) p = p->next;
        p->next = node;
    }
    printf("��ҩƷ���ӳɹ���\n");
}

// ---------------------------------------------------------
// 锟斤拷锟斤拷锟剿ｏ拷药锟斤拷系统锟斤拷路锟斤拷
// ---------------------------------------------------------
void drugMenu() {
    int choice;
    do {
        // ע�⣺�����¼���أ�ֱ�ӳ���ҵ��˵�
        printf("\n========== ҩ������ ==========\n");
        printf("1. ҩƷ����ѯ\n");
        printf("2. ҩƷ���\n");
        printf("3. ҩƷ����\n");
        printf("4. �鿴���䶯��¼\n");
		printf("5.����ҩƷ��Ϣ\n");
        printf("0. �������˵�\n");
        printf("��ѡ��: ");
        if (scanf("%d", &choice) != 1)
            choice = -1;
        switch (choice) {
        case 1: drugStockQuery(); break;
        case 2: drugIn(); break;
        case 3: drugOut(); break;
        case 4: viewStockRecords(); break;
		case 5: addDrug(); break;
        case 0: break;
        default: printf("锟斤拷效选锟筋。\n");
        }
    } while (choice != 0);
}