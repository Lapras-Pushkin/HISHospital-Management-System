#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "models.h"
#include "utils.h"
#include "inpatient_department.h"
#include "transaction.h"
#include "outpatient_department.h" 
#include "drug.h"

// ---------------------------------------------------------
// 辅助函数：通过患者ID回溯其首诊负责科室
// 设计思路：在办理入院时，仅凭患者ID无法得知其应归属哪个病区。
// 此函数通过遍历全局流水，找到该患者最近的一张入院通知单（Type 6），
// 并根据开单医师的工号反向查询其所属科室，从而实现科室的自动匹配。
// ---------------------------------------------------------
void getResponsibleDept(const char* patientId, char* deptBuffer) {
    Record* r;
    char staffId[20] = "";
    Staff* s;

    strcpy(deptBuffer, "未知科室");
    r = recordHead->next;

    while (r) {
        if (r->type == 6 && strcmp(r->patientId, patientId) == 0) {
            strcpy(staffId, r->staffId);
        }
        r = r->next;
    }
    if (strlen(staffId) > 0) {
        s = staffHead->next;
        while (s) {
            if (strcmp(s->id, staffId) == 0) {
                strcpy(deptBuffer, s->department);
                break;
            }
            s = s->next;
        }
    }
}

// ---------------------------------------------------------
// 初始化模块：构建全院初始物理床位数据
// 设计思路：系统首次运行或未读取到本地文件时，自动生成基础数据。
// 包含5大科室，每科室5个房间，每房间4张床，共计100张床位。
// ---------------------------------------------------------
void initBedsIfEmpty() {
    char types[5][50] = { "单人病房", "双人病房", "三人病房", "单人陪护病房", "单人陪护疗养病房" };
    double prices[5] = { 200.0, 150.0, 100.0, 300.0, 500.0 };
    char wards[2][50] = { "普通病房", "特殊病房" };
    Bed* tail;
    int roomNum = 1;
    int d, i, j;

    if (bedHead->next) return;

    tail = bedHead;

    for (d = 0; d < 5; d++) {
        for (i = 0; i < 5; i++) {
            for (j = 1; j <= 4; j++) {
                Bed* b = (Bed*)malloc(sizeof(Bed));
                sprintf(b->bedId, "%d-%d", roomNum, j);
                b->isOccupied = 0; strcpy(b->patientId, ""); strcpy(b->bedType, types[i]);
                b->price = prices[i]; strcpy(b->wardType, (i >= 3) ? wards[1] : wards[0]);
                b->isRoundsDone = 0;
                b->next = NULL; tail->next = b; tail = b;
            }
            roomNum++;
        }
    }
}

// ---------------------------------------------------------
// 辅助函数：根据床位编号映射归属科室
// ---------------------------------------------------------
const char* getRoomDepartment(const char* bedId) {
    int roomNum;
    int deptIdx;
    sscanf(bedId, "%d-", &roomNum);
    deptIdx = (roomNum - 1) / 5;
    if (deptIdx == 0) return "内科";
    if (deptIdx == 1) return "外科";
    if (deptIdx == 2) return "儿科";
    if (deptIdx == 3) return "妇产科";
    if (deptIdx == 4) return "急诊科";
    return "未知科室";
}

// ---------------------------------------------------------
// 资源调度模块：监测科室负载并执行动态扩容
// 设计思路：在办理入院前，系统会自动检查目标科室的床位空闲率。
// 如果空闲率低于 20%，触发应急机制，将科室内的“单人疗养病房”
// 动态拆分为两张“双人病房”床位，以增加收治容量。
// ---------------------------------------------------------
void checkAndAdjustBedTension(const char* targetDept) {
    int total = 0, empty = 0;
    Bed* b = bedHead->next;
    int converted = 0;

    while (b) {
        if (strcmp(getRoomDepartment(b->bedId), targetDept) == 0) {
            total++;
            if (b->isOccupied == 0) empty++;
        }
        b = b->next;
    }

    if (total > 0 && ((float)empty / total) < 0.2f) {
        printf("\n  [系统动态调度] %s 专属空床率不足 20%%！触发紧急扩容机制...\n", targetDept);
        printf("  >>> 正在将【%s】内的单人陪护疗养病房临时调整为双人病房 <<<\n", targetDept);

        b = bedHead->next;
        while (b) {
            if (b->isOccupied == 0 &&
                strcmp(b->bedType, "单人陪护疗养病房") == 0 &&
                strcmp(getRoomDepartment(b->bedId), targetDept) == 0) {

                Bed* extra;
                strcpy(b->bedType, "双人病房");
                b->price = 150.0;

                extra = (Bed*)malloc(sizeof(Bed));
                *extra = *b;
                sprintf(extra->bedId, "%sA", b->bedId);
                extra->next = bedHead->next; bedHead->next = extra;
                converted++;
            }
            b = b->next;
        }
        if (converted > 0) {
            printf("  [扩容成功] 已为 %s 增加了 %d 张可用双人床位！\n", targetDept, converted);
        }
        else {
            printf("  [扩容失败] %s 当前已无空余的单人疗养病房可供拆分。\n", targetDept);
        }
    }
}

// 获取当前存在的科室名称列表，用于菜单提示
void getDynamicDeptPrompt(char* promptBuffer) {
    char depts[20][50]; int dCount = 0;
    Staff* stf = staffHead->next;
    int exists, i;

    while (stf) {
        exists = 0;
        for (i = 0; i < dCount; i++) if (strcmp(depts[i], stf->department) == 0) { exists = 1; break; }
        if (!exists && strlen(stf->department) > 0) { strcpy(depts[dCount], stf->department); dCount++; }
        stf = stf->next;
    }

    strcpy(promptBuffer, "");
    for (i = 0; i < dCount; i++) {
        strcat(promptBuffer, depts[i]);
        if (i < dCount - 1) strcat(promptBuffer, "/");
    }
}

// ---------------------------------------------------------
// 1. 全院床位实时使用图谱查询功能
// ---------------------------------------------------------
void viewAllBeds() {
    initBedsIfEmpty();
    while (1) {
        char deptStr[200];
        char targetDept[50];
        Bed* b;
        int bedCount = 0;

        system("cls");
        printf("\n========================================================================================\n");
        printf("                           全院病房与床位实时使用雷达图谱                               \n");
        printf("========================================================================================\n");

        getDynamicDeptPrompt(deptStr);
        printf("当前活跃科室有: (%s)\n请输入要查看的科室 (输0返回): ", deptStr);
        safeGetString(targetDept, 50);
        if (strcmp(targetDept, "0") == 0) return;

        printf("\n>>> 【%s】 专属住院病区动态 <<<\n", targetDept);
        // 使用定长占位符控制输出格式，确保表格列对齐
        printf("%-10s %-12s %-18s %-8s %-10s %-15s %-15s\n", "房号-床位", "病区等级", "房型", "单价", "目前状态", "收治大夫", "入住患者");
        printf("----------------------------------------------------------------------------------------\n");

        b = bedHead->next;
        while (b) {
            if (strcmp(getRoomDepartment(b->bedId), targetDept) == 0) {
                char patName[100] = "-"; char docName[50] = "-";

                // 如果床位被占用，关联查询对应的患者姓名及负责医生姓名
                if (b->isOccupied) {
                    Patient* p = patientHead->next;
                    Record* r;

                    while (p) { if (strcmp(p->id, b->patientId) == 0) { strcpy(patName, p->name); break; } p = p->next; }

                    r = recordHead->next;
                    while (r) {
                        if (r->type == 6 && strcmp(r->patientId, b->patientId) == 0) {
                            Staff* s = staffHead->next;
                            while (s) { if (strcmp(s->id, r->staffId) == 0) { strcpy(docName, s->name); break; } s = s->next; }
                            break;
                        }
                        r = r->next;
                    }
                }

                printf("%-10s %-12s %-18s %-8.2f %-10s %-15s %s(%s)\n",
                    b->bedId, b->wardType, b->bedType, b->price,
                    b->isOccupied ? "[有客]" : "[闲置]", docName,
                    b->isOccupied ? patName : "-", b->isOccupied ? b->patientId : "-");
                bedCount++;
            }
            b = b->next;
        }
        if (bedCount == 0) printf("  (未找到该科室的专属病床数据)\n");
        printf("----------------------------------------------------------------------------------------\n");
        system("pause");
    }
}

// ---------------------------------------------------------
// 2. 入院办理与床位分配系统
// ---------------------------------------------------------
void admitPatient(const char* docId) {
    Record* r;
    int noticeCount = 0;
    char pId[20];
    Record* targetNotice = NULL;
    Patient* p;
    Patient* targetPat = NULL;
    char requiredDept[50];
    int hasLocalEmpty = 0;
    Bed* b;
    int isCrossDept = 0;
    int hasAnyEmpty = 0;
    char selectBed[20];
    Bed* finalBed = NULL;
    int days;
    int baseDeposit;
    int finalDeposit;
    int isPaid = 0;
    Record* r5;
    char adminTime[30];

    initBedsIfEmpty();

    printf("\n========== 门诊下发《待入院通知单》队列 ==========\n");
    r = recordHead->next;

    // 分类显示待处理队列，重症患者优先展示，便于调度
    printf("【重症优先通道】\n");
    while (r) {
        if (r->type == 6 && r->isPaid == 0 && strstr(r->description, "重症")) {
            Patient* pt = patientHead->next; char pName[100] = "未知";
            char deptName[50];
            while (pt) { if (strcmp(pt->id, r->patientId) == 0) { strcpy(pName, pt->name); break; } pt = pt->next; }
            getResponsibleDept(r->patientId, deptName);
            printf(" -> [紧急] 患者ID: %s | 姓名: %s | 负责科室: %s | 说明: %s\n", r->patientId, pName, deptName, r->description);
            noticeCount++;
        }
        r = r->next;
    }

    printf("\n【普通入院队列】\n");
    r = recordHead->next;
    while (r) {
        if (r->type == 6 && r->isPaid == 0 && strstr(r->description, "普通")) {
            Patient* pt = patientHead->next; char pName[100] = "未知";
            char deptName[50];
            while (pt) { if (strcmp(pt->id, r->patientId) == 0) { strcpy(pName, pt->name); break; } pt = pt->next; }
            getResponsibleDept(r->patientId, deptName);
            printf(" -> [常规] 患者ID: %s | 姓名: %s | 负责科室: %s | 说明: %s\n", r->patientId, pName, deptName, r->description);
            noticeCount++;
        }
        r = r->next;
    }

    if (noticeCount == 0) { printf("  [提示] 当前暂无门诊下发的未处理住院通知单。\n"); return; }

    printf("\n请输入需办理入院的患者ID (输0返回): "); safeGetString(pId, 20);
    if (strcmp(pId, "0") == 0) return;

    // 校验通知单有效性，防止越权或重复收治
    r = recordHead->next;
    while (r) { if (r->type == 6 && strcmp(r->patientId, pId) == 0 && r->isPaid == 0) { targetNotice = r; break; } r = r->next; }

    if (!targetNotice) {
        int alreadyIn = 0;
        b = bedHead->next;
        while (b) { if (b->isOccupied && strcmp(b->patientId, pId) == 0) { alreadyIn = 1; break; } b = b->next; }
        if (alreadyIn) printf("  [拦截警告] 业务异常：该患者当前已处于住院收治状态。\n");
        else printf("  [拦截警告] 业务阻断：缺乏门诊前端开具的合规通知单，无法受理收治。\n");
        return;
    }

    p = patientHead->next;
    while (p) { if (strcmp(p->id, pId) == 0) { targetPat = p; break; } p = p->next; }

    getResponsibleDept(pId, requiredDept);
    printf("\n>>> 锁定通知单：该患者由【%s】下发，系统正检索 %s 的专属空余床位...\n", requiredDept, requiredDept);

    checkAndAdjustBedTension(requiredDept);

    b = bedHead->next;
    while (b) {
        if (!b->isOccupied && strcmp(getRoomDepartment(b->bedId), requiredDept) == 0) { hasLocalEmpty = 1; break; }
        b = b->next;
    }

    // 跨科室资源协调机制：当目标科室即使扩容后仍然满载时，允许跨科室提取空床资源
    if (!hasLocalEmpty) {
        printf("\n  [警告] %s 专属床位资源枯竭（扩容后仍无空余）！\n", requiredDept);
        printf("  >>> 触发系统应急预案：开启跨科室床位全局调度 <<<\n");
        isCrossDept = 1;

        b = bedHead->next;
        while (b) { if (!b->isOccupied) { hasAnyEmpty = 1; break; } b = b->next; }

        if (!hasAnyEmpty) {
            printf("  [极其抱歉] 全院整体床位网络满载，暂无法提供物理空间进行收治转运。\n");
            return;
        }

        printf("\n【全院可用空闲病床列表 (跨域收治)】:\n");
        b = bedHead->next;
        while (b) {
            if (!b->isOccupied) {
                printf("  [%s] 所属: %-6s | %s - %s (基准日租 ￥%.2f)\n", b->bedId, getRoomDepartment(b->bedId), b->wardType, b->bedType, b->price);
            }
            b = b->next;
        }
    }
    else {
        printf("\n【%s】可用空闲病床列表:\n", requiredDept);
        b = bedHead->next;
        while (b) {
            if (!b->isOccupied && strcmp(getRoomDepartment(b->bedId), requiredDept) == 0) {
                printf("  [%s] %s - %s (基准日租 ￥%.2f)\n", b->bedId, b->wardType, b->bedType, b->price);
            }
            b = b->next;
        }
    }

    printf("\n请输入系统拟分配的床位号 (如 1-3) (输入0取消): "); safeGetString(selectBed, 20);
    if (strcmp(selectBed, "0") == 0) return;

    b = bedHead->next;
    while (b) {
        if (strcmp(b->bedId, selectBed) == 0 && !b->isOccupied) {
            if (!isCrossDept && strcmp(getRoomDepartment(b->bedId), requiredDept) != 0) {
                // 非跨科室模式下，阻断分配其他科室的床位
            }
            else {
                finalBed = b; break;
            }
        }
        b = b->next;
    }

    if (!finalBed) {
        if (!isCrossDept) printf("  [!] 无效选择，该床位不存在或越界提取了非本科室床位。\n");
        else printf("  [!] 资源已被占用或该床位不存在。\n");
        return;
    }

    printf("\n  [√] 资源 [%s] 分配锁定成功！\n", finalBed->bedId);
    printf("请输入拟定住院周期(天): ");

    // 使用输入包装函数拦截非法输入字符
    days = safeGetPositiveInt();

    // 动态押金计算模型：保障账户资金充足，最低押金限额 1000 元，按百位向上取整
    baseDeposit = 200 * days;
    if (baseDeposit < 1000) baseDeposit = 1000;
    finalDeposit = ((baseDeposit + 99) / 100) * 100;

    printf("  系统通过风控模型核算，需缴纳初始住院统筹押金: %d 元。\n", finalDeposit);

    // 资金划扣处理
    if (targetPat->balance >= finalDeposit) {
        Transaction* newTrans;
        int maxId = 0;
        Transaction* curr;

        targetPat->balance -= finalDeposit; isPaid = 1;
        printf("  [√] 账户资金划扣完毕，病区收治状态已激活。\n");

        newTrans = (Transaction*)malloc(sizeof(Transaction));
        curr = transactionList;
        while (curr) { if (curr->id > maxId) maxId = curr->id; curr = curr->next; }
        newTrans->id = maxId + 1; newTrans->type = 2; newTrans->amount = finalDeposit;
        getCurrentTimeStr(newTrans->time, 30); sprintf(newTrans->description, "住院押金收取(患者:%s)", targetPat->name);
        newTrans->next = NULL;

        if (!transactionList) transactionList = newTrans;
        else { curr = transactionList; while (curr->next) curr = curr->next; curr->next = newTrans; }
    }
    else {
        printf("  [提示] 账户实时结余不足，系统已挂起床位并生成待处理押金账单。\n");
    }

    // 写入占用状态及记录
    finalBed->isOccupied = 1; strcpy(finalBed->patientId, pId);
    finalBed->isRoundsDone = 0;
    targetNotice->isPaid = 1;

    r5 = (Record*)malloc(sizeof(Record));
    extern void generateRecordID(char* buffer);
    generateRecordID(r5->recordId);
    r5->type = 5; strcpy(r5->patientId, pId); strcpy(r5->staffId, docId);
    r5->cost = finalDeposit; r5->isPaid = isPaid;

    getCurrentTimeStr(adminTime, 30);
    sprintf(r5->description, "病房:%s_床位:%s_入院日期:%s_预期天数:%d_押金:%d_出院日期:无_总费用:0",
        finalBed->wardType, finalBed->bedId, adminTime, days, finalDeposit);
    getCurrentTimeStr(r5->createTime, 30);
    r5->next = recordHead->next; recordHead->next = r5;

    if (isPaid) {
        if (isCrossDept) printf("\n  [成功] 跨模块调度机制完成，已将病患安置于 [%s] 的 %s 节点。\n", getRoomDepartment(finalBed->bedId), finalBed->bedId);
        else printf("\n  [成功] 基础收治流程完结，已分配至 %s 的 %s 节点。\n", requiredDept, finalBed->bedId);
    }
    else {
        printf("\n  [待确认] 空间分配记录已生成，待财务端确认资金流入后释放节点使用权。\n");
    }
}

// ---------------------------------------------------------
// 3. 模拟定时计费任务
// ---------------------------------------------------------
// 该功能模拟每日早晨对在院患者进行床位费用的自动划扣，
// 同步检测患者余额是否不足，重置日常查房标记。
void dailyDeductionSimulation() {
    Bed* b;
    int count = 0;
    Patient* p;

    printf("\n========== 执行批量床位核销日结算 (模拟08:00) ==========\n");
    b = bedHead->next;
    while (b) {
        if (b->isOccupied) {
            p = patientHead->next;
            while (p) {
                if (strcmp(p->id, b->patientId) == 0) {
                    if (p->balance < 1000) printf("  [预警] 检测到在院实体 %s 资金池跌破1000元红线 (结余: %.2f)！\n", p->name, p->balance);
                    printf("  -> 实体 %s (挂载床位:%s) 日度消耗计提: %.2f 元。\n", p->name, b->bedId, b->price);
                    count++; break;
                }
                p = p->next;
            }
            b->isRoundsDone = 0;
        }
        b = b->next;
    }
    printf("--------------------------------------------------\n");
    printf("批处理作业完毕。累计扫描并计算 %d 个活跃在院单元，病床日内查房状态已复位。\n", count);
}

// ---------------------------------------------------------
// 4. 病区查房与医嘱下达模块
// ---------------------------------------------------------
void wardRounds(const char* docId) {
    while (1) {
        char deptStr[200];
        char targetDept[50];
        Bed* b;
        int pCount = 0;
        char pId[20];
        Bed* targetBed = NULL;

        system("cls");
        printf("\n========== 住院病区日常巡检与查房 ==========\n");

        getDynamicDeptPrompt(deptStr);
        printf("调取目标科室域 (%s, 0返回): ", deptStr);
        safeGetString(targetDept, 50);
        if (strcmp(targetDept, "0") == 0) return;

        printf("\n--- 【%s】 当前住院受管名单 ---\n", targetDept);
        printf("%-10s %-15s %-10s %-10s %-10s\n", "房-床号", "患者ID", "姓名", "评估级别", "查房状态");
        printf("----------------------------------------------------------\n");

        b = bedHead->next;
        while (b) {
            if (b->isOccupied && strcmp(getRoomDepartment(b->bedId), targetDept) == 0) {
                Patient* p = patientHead->next; char pName[100] = "未知";
                char severity[10] = "普通";
                Record* r;

                while (p) { if (strcmp(p->id, b->patientId) == 0) { strcpy(pName, p->name); break; } p = p->next; }

                r = recordHead->next;
                while (r) {
                    if (r->type == 6 && strcmp(r->patientId, b->patientId) == 0) {
                        if (strstr(r->description, "重症")) strcpy(severity, "重症");
                    }
                    r = r->next;
                }

                printf("%-10s %-15s %-10s %-10s %-10s\n",
                    b->bedId, b->patientId, pName, severity, b->isRoundsDone ? "[已查房]" : "[待查房]");
                pCount++;
            }
            b = b->next;
        }

        if (pCount == 0) { printf("  (目标病区当前无处于活跃周期的在院人员)\n"); system("pause"); continue; }

        printf("\n检索需建立查房会话的患者ID (0退回科室层级): "); safeGetString(pId, 20);
        if (strcmp(pId, "0") == 0) continue;

        b = bedHead->next;
        while (b) {
            if (b->isOccupied && strcmp(b->patientId, pId) == 0 && strcmp(getRoomDepartment(b->bedId), targetDept) == 0) {
                targetBed = b; break;
            }
            b = b->next;
        }
        if (!targetBed) { printf("  [!] 定位失败：未在 %s 的管辖范畴内检索到对应人员。\n", targetDept); system("pause"); continue; }

        while (1) {
            int choice;
            system("cls");
            printf("\n========== 针对患者实体 [%s] 的查房干预面板 ==========\n", pId);
            printf("  [1] 录入基础观察医嘱\n  [2] 调用内部药房系统配发治疗耗材\n  [0] 结束当前查房会话\n---------------------------------------\n下达操作指令: ");

            choice = safeGetInt();
            if (choice == 0) break;

            if (choice == 1) {
                char note[200];
                Record* rx;
                extern void generateRecordID(char* buffer);

                printf("输入实时医嘱指令内容(避免非法空格切断字符): ");
                safeGetString(note, 200);
                rx = (Record*)malloc(sizeof(Record));

                generateRecordID(rx->recordId);
                rx->type = 2; strcpy(rx->patientId, pId); strcpy(rx->staffId, docId);
                rx->cost = 0; rx->isPaid = 1;
                sprintf(rx->description, "住院查房医嘱:%s", note);
                getCurrentTimeStr(rx->createTime, 30);
                rx->next = recordHead->next; recordHead->next = rx;
                printf("  [√] 会话操作确认，医嘱项已持久化记录。\n");
                targetBed->isRoundsDone = 1;
                system("pause");
            }
            else if (choice == 2) {
                Record* cur;

                // 利用外部接口复用门诊业务代码，避免重复实现开药逻辑
                extern char currentCallingPatientId[20];
                strcpy(currentCallingPatientId, pId);
                extern void prescribeMedicine(const char* docId);
                prescribeMedicine(docId);

                // 账单状态延迟写入：
                // 针对住院用药，强制修改费用记录的状态标记为4（后台记账挂钩），
                // 在出院总清算环节进行押金统筹对冲，避免立刻呈现至前端要求缴费。
                cur = recordHead->next;
                while (cur) {
                    if (cur->type == 3 && strcmp(cur->patientId, pId) == 0 && cur->isPaid == 0) {
                        char tempDesc[300];
                        cur->isPaid = 4;
                        sprintf(tempDesc, "[住院记账]%s", cur->description);
                        strcpy(cur->description, tempDesc);
                    }
                    cur = cur->next;
                }

                strcpy(currentCallingPatientId, "");
                targetBed->isRoundsDone = 1;
                system("pause");
            }
        }
    }
}

// ---------------------------------------------------------
// 5. 离院结算与状态流转解绑
// ---------------------------------------------------------
void dischargePatient() {
    while (1) {
        char deptStr[200];
        char targetDept[50];
        Bed* b_list;
        int count = 0;
        char pId[20];
        Bed* b;
        Bed* targetBed = NULL;
        Record* r_check;
        time_t t;
        struct tm* tm_info;
        int currentHour;
        int actualDays;
        int billableDays;
        double totalBedFee;
        double totalDrugFee = 0;
        Record* r;
        double totalHospitalCost;
        double total_deposit = 0;

        system("cls");
        printf("\n========== 离院综合清算办理控制台 ==========\n");

        getDynamicDeptPrompt(deptStr);
        printf("锁定业务作用域科室 (%s, 0放弃办理): ", deptStr);
        safeGetString(targetDept, 50);
        if (strcmp(targetDept, "0") == 0) return;

        printf("\n--- 【%s】 等待出院审批名册 ---\n", targetDept);
        printf("%-10s %-12s %-15s %-10s\n", "房号-床位", "属性", "患者ID", "姓名");
        printf("-----------------------------------------------------\n");

        b_list = bedHead->next;
        while (b_list) {
            if (b_list->isOccupied && strcmp(getRoomDepartment(b_list->bedId), targetDept) == 0) {
                Patient* p = patientHead->next; char pName[100] = "未知";
                while (p) { if (strcmp(p->id, b_list->patientId) == 0) { strcpy(pName, p->name); break; } p = p->next; }
                printf("%-10s %-12s %-15s %-10s\n", b_list->bedId, b_list->wardType, b_list->patientId, pName);
                count++;
            }
            b_list = b_list->next;
        }
        if (count == 0) { printf("  (当前无可流转的出院对象)\n"); system("pause"); continue; }

        printf("\n确立清算实体的患者ID (0终止当前操作): "); safeGetString(pId, 20);
        if (strcmp(pId, "0") == 0) continue;

        b = bedHead->next;
        while (b) { if (b->isOccupied && strcmp(b->patientId, pId) == 0 && strcmp(getRoomDepartment(b->bedId), targetDept) == 0) { targetBed = b; break; } b = b->next; }
        if (!targetBed) { printf("  [!] 校验异常：所选区域中无此实体记录。\n"); system("pause"); continue; }

        // 状态锁存机制：拦截重复执行出院计算
        // 若检索到已在底层记录了带缴清的补充流水，阻止医生重复生成催费记录单据，防止逻辑死锁。
        r_check = recordHead->next;
        while (r_check) {
            if (strcmp(r_check->patientId, pId) == 0 && r_check->type == 5 && r_check->isPaid == 0 && strstr(r_check->description, "出院清算_补缴欠费差额")) {
                printf("\n  [系统资源互斥] 底层检测到该出院流水线正在挂起，存在待处理的资金缺口！\n");
                printf("  >>> 操作指引：需告知责任患者前往前端终端支付尾款。款项入账后引擎将利用异步回调自行释放空间资源，此处拒绝重复建单操作！\n");
                system("pause");
                return;
            }
            r_check = r_check->next;
        }

        t = time(NULL);
        tm_info = localtime(&t);
        currentHour = tm_info->tm_hour;
        printf("\n键入供财务审核的最终驻留计费天数: ");
        actualDays = safeGetPositiveInt();

        billableDays = actualDays;
        if (currentHour >= 0 && currentHour < 8) {
            printf("  [时间优惠策略触发] 08:00 阈值前申请办理，系统已自动扣减终端最后一天的驻留费用计提！\n"); billableDays = (actualDays > 1) ? actualDays - 1 : 0;
        }

        // 汇总实际发生额（床位费用 + 标记为挂账状态的住院配药费）
        totalBedFee = billableDays * targetBed->price;
        r = recordHead->next;
        while (r) { if (strcmp(r->patientId, pId) == 0 && r->type == 3 && r->isPaid == 4) totalDrugFee += r->cost; r = r->next; }

        totalHospitalCost = totalBedFee + totalDrugFee;
        r = recordHead->next;
        while (r) { if (strcmp(r->patientId, pId) == 0 && r->type == 5 && r->isPaid == 1) total_deposit += r->cost; r = r->next; }

        // 处理场景A：所缴押金足以覆盖所有费用支出，释放床位并退款
        if (total_deposit >= totalHospitalCost) {
            double refund;

            r = recordHead->next;
            while (r) { if (strcmp(r->patientId, pId) == 0 && r->type == 3 && r->isPaid == 4) r->isPaid = 1; r = r->next; }

            refund = total_deposit - totalHospitalCost;
            if (refund > 0) {
                Patient* pt = patientHead->next;
                Record* r8;
                extern void generateRecordID(char* buffer);
                Transaction* newTrans;
                int maxId = 0;
                Transaction* curr;

                while (pt) { if (strcmp(pt->id, pId) == 0) { pt->balance += refund; break; } pt = pt->next; }
                printf("\n  [资金清算] 提取前期统筹押金 %.2f 元，溢出资产 %.2f 元，已执行原路资金清退。\n", total_deposit, refund);

                r8 = (Record*)malloc(sizeof(Record));
                generateRecordID(r8->recordId); r8->type = 8; strcpy(r8->patientId, pId); strcpy(r8->staffId, "SYS");
                r8->cost = refund; r8->isPaid = 1; sprintf(r8->description, "出院清算_押金结余退回");
                getCurrentTimeStr(r8->createTime, 30); r8->next = recordHead->next; recordHead->next = r8;

                newTrans = (Transaction*)malloc(sizeof(Transaction));
                curr = transactionList;
                while (curr) { if (curr->id > maxId) maxId = curr->id; curr = curr->next; }
                newTrans->id = maxId + 1; newTrans->type = 2; newTrans->amount = -refund;
                getCurrentTimeStr(newTrans->time, 30); strcpy(newTrans->description, "出院清算_押金结余退回");
                newTrans->next = NULL;
                if (!transactionList) transactionList = newTrans; else { curr = transactionList; while (curr->next) curr = curr->next; curr->next = newTrans; }
            }

            r = recordHead->next;
            while (r) {
                if (strcmp(r->patientId, pId) == 0 && r->type == 5 && r->isPaid == 1) {
                    char summary[200];
                    r->isPaid = 2;
                    sprintf(summary, " [出院结算:床费%.2f 药费%.2f 总消费%.2f]", totalBedFee, totalDrugFee, totalHospitalCost);
                    strcat(r->description, summary);
                }
                r = r->next;
            }
            targetBed->isOccupied = 0; strcpy(targetBed->patientId, "");
            printf("\n  [解除挂载] 系统解绑指令送达，该物理床位资源已恢复闲置状态。\n");
            system("pause"); return;
        }
        // 处理场景B：发生押金穿透，将参数写入待支付清单记录，移交其他模块处理
        else {
            double arrears = totalHospitalCost - total_deposit;
            Record* r_arrears = (Record*)malloc(sizeof(Record));
            extern void generateRecordID(char* buffer);

            generateRecordID(r_arrears->recordId); r_arrears->type = 5; strcpy(r_arrears->patientId, pId); strcpy(r_arrears->staffId, "SYS");
            r_arrears->cost = arrears; r_arrears->isPaid = 0;

            // 通过字符串包含序列化参数的方式，打通底层接口参数传递
            sprintf(r_arrears->description, "出院清算_补缴欠费差额_床位:%s_床费:%.2f_药费:%.2f", targetBed->bedId, totalBedFee, totalDrugFee);
            getCurrentTimeStr(r_arrears->createTime, 30); r_arrears->next = recordHead->next; recordHead->next = r_arrears;

            printf("\n  [资金拦截] 收支不平衡预警：实发总额 %.2f，储备结余 %.2f。\n", totalHospitalCost, total_deposit);
            printf("  >>> 资产缺口认定: %.2f 元。数据指针已抛出至前端财务缴费中心。 <<<\n", arrears);
            printf("  后续动作移交异步处理：患者完成终端资产确认后，底层引擎将代理执行资源的清退与解绑，当前会话完毕。\n");
            system("pause"); return;
        }
    }
}

// ---------------------------------------------------------
// 住院部业务流转中心
// ---------------------------------------------------------
void inpatientMenu(const char* docId) {
    while (1) {
        int choice;
        system("cls");
        printf("\n==================================================\n");
        printf("                 住院管理调度中心                 \n");
        printf("==================================================\n");
        printf("  [1] 查看全院病房实时图谱\n");
        printf("  [2] 办理患者入院 (核算押金/床位分配)\n");
        printf("  [3] 执行每日早8点查床扣费\n");
        printf("  [4] 日常查房与下达医嘱记录\n");
        printf("  [5] 办理患者出院 (特惠与统账流转)\n");
        printf("  [0] 返回上级医生大厅\n");
        printf("--------------------------------------------------\n");
        printf("调拨系统功能执行器: ");

        choice = safeGetInt();
        switch (choice) {
        case 1: viewAllBeds(); break;
        case 2: admitPatient(docId); system("pause"); break;
        case 3: dailyDeductionSimulation(); system("pause"); break;
        case 4: wardRounds(docId); break;
        case 5: dischargePatient(); break;
        case 0: return;
        }
    }
}