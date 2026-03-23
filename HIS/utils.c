#define _CRT_SECURE_NO_WARNINGS
#include "models.h"
#include "utils.h"

// ==========================================
// 安全读取字符串 (自动清理末尾的回车符，并清空缓冲区)
// ==========================================
void safeGetString(char* buffer, int size) {
    if (fgets(buffer, size, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0'; // 将换行符替换为字符串结束符
    }
    else {
        buffer[0] = '\0';
    }
}

// ==========================================
// 安全读取整数 (借用 fgets 后转格式，防止用户乱输入英文字母导致崩溃)
// ==========================================
int safeGetInt() {
    char buffer[100];
    safeGetString(buffer, sizeof(buffer));
    int value = 0;
    sscanf(buffer, "%d", &value);
    return value;
}

// ==========================================
// 安全读取浮点数 (常用于金额计算)
// ==========================================
double safeGetDouble() {
    char buffer[100];
    safeGetString(buffer, sizeof(buffer));
    double value = 0.0;
    sscanf(buffer, "%lf", &value);
    return value;
}

// ==========================================
// 内部函数：往全局用户链表尾部塞一个节点
// ==========================================
void addUserNode(const char* account, const char* pwd, const char* role, const char* name, const char* dept) {
    User* node = (User*)malloc(sizeof(User));
    strcpy(node->account, account);
    strcpy(node->password, pwd);
    strcpy(node->role, role);
    strcpy(node->name, name);
    strcpy(node->department, dept);
    node->next = NULL;

    // 如果是空链表，直接当头节点
    if (!userHead) { userHead = node; return; }

    // 否则循环找尾巴
    User* temp = userHead;
    while (temp->next) temp = temp->next;
    temp->next = node;
}

// ==========================================
// 内部函数：往患者医疗档案链表尾部塞一个节点
// ==========================================
void addPatientNode(const char* id, const char* name, double balance) {
    Patient* p = (Patient*)malloc(sizeof(Patient));
    strcpy(p->id, id);
    strcpy(p->name, name);
    p->balance = balance;
    // 默认补全缺失数据
    strcpy(p->gender, "未知");
    p->age = 0;
    strcpy(p->allergy, "无");
    p->isEmergency = 0;
    p->next = NULL;

    if (!patientHead) { patientHead = p; return; }
    Patient* temp = patientHead;
    while (temp->next) temp = temp->next;
    temp->next = p;
}

// ==========================================
// 核心：启动时统一自动加载基础数据到内存链表
// 原理：使用 fopen 和 fscanf 提取同目录下的 txt 数据
// ==========================================
void loadAllData() {
    FILE* fp;
    char s1[50], s2[50], s3[50], s4[50], s5[50], s6[50];
    double f1; int i1, i2;

    // 1. 加载登录通行证与患者档案 (txt里第6个字段顺便给患者充当钱包余额)
    fp = fopen("user_data.txt", "r");
    if (fp) {
        while (fscanf(fp, "%s %s %s %s %s %lf", s1, s2, s3, s4, s5, &f1) != EOF) {
            addUserNode(s1, s2, s3, s4, s5);
            // 如果读到的是患者权限，联动创建其实名医疗档案
            if (strcmp(s3, "patient") == 0) {
                addPatientNode(s1, s4, f1);
            }
        }
        fclose(fp);
    }
    else printf("[系统提示] user_data.txt 不存在，将启动空环境。\n");

    // 2. 加载药房库存 (适配包含通用名 commonName 的新版格式)
    fp = fopen("medicine_data.txt", "r");
    if (fp) {
        while (fscanf(fp, "%s %s %s %d %lf %s", s1, s2, s3, &i1, &f1, s4) != EOF) {
            Medicine* m = (Medicine*)malloc(sizeof(Medicine));
            strcpy(m->id, s1); strcpy(m->name, s2); strcpy(m->commonName, s3);
            m->stock = i1; m->price = f1; strcpy(m->department, s4); strcpy(m->expiryDate, "2026-12-31");

            // 头插法，效率高
            m->next = medicineHead; medicineHead = m;
        }
        fclose(fp);
    }

    // 3. 加载排班信息 
    fp = fopen("schedule_data.txt", "r");
    if (fp) {
        while (fscanf(fp, "%s %s %s %s %s %s", s1, s2, s3, s4, s5, s6) != EOF) {
            Schedule* s = (Schedule*)malloc(sizeof(Schedule));
            strcpy(s->docId, s1); strcpy(s->name, s2); strcpy(s->department, s3);
            strcpy(s->date, s4); strcpy(s->timeSlot, s5); strcpy(s->status, s6);

            s->next = scheduleHead; scheduleHead = s;
        }
        fclose(fp);
    }

    // 4. 加载住院部病房/床位信息
    fp = fopen("bed_data.txt", "r");
    if (fp) {
        while (fscanf(fp, "%s %s %s %d %d %lf %s", s1, s2, s3, &i1, &i2, &f1, s4) != EOF) {
            Bed* b = (Bed*)malloc(sizeof(Bed));
            strcpy(b->bedId, s1); strcpy(b->department, s2); strcpy(b->type, s3);
            b->bedNumber = i1; b->isOccupied = i2; b->price = f1; strcpy(b->patientId, s4);

            b->next = bedHead; bedHead = b;
        }
        fclose(fp);
    }
}

// ==========================================
// 核心：通用流水账单保存机制 (双向同步：链表+物理TXT)
// 作用：无论挂号还是开药产生消费，调用这个函数就能安全保存并落盘
// ==========================================
void saveRecordToFile(const char* filename, int type, const char* typeName, const char* patientId, const char* staffId, const char* desc, double cost, int isPaid) {
    // 动态分配内存
    Record* r = (Record*)malloc(sizeof(Record));
    // 生成随机的唯一流水号，如 REC5432
    sprintf(r->recordId, "REC%d", rand() % 10000 + 1000);
    r->type = type;
    strcpy(r->typeName, typeName);
    strcpy(r->patientId, patientId);
    strcpy(r->staffId, staffId);
    strcpy(r->description, desc);
    r->cost = cost;
    r->isPaid = isPaid;

    // 1. 头插法记入内存链表，方便后续查询
    r->next = recordHead; recordHead = r;

    // 2. 写入文件持久化 ("a"模式为追加，不会覆盖历史数据)
    FILE* fp = fopen(filename, "a");
    if (fp) {
        fprintf(fp, "%s %d %s %s %s %.2f %d %s\n", r->recordId, type, typeName, patientId, staffId, cost, isPaid, desc);
        fclose(fp);
    }
}