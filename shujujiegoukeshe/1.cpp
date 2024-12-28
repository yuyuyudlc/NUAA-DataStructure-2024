#include <iostream>
#include <windows.h>
#include <psapi.h>
#include <string>
#include <chrono>
#include <thread>
using namespace std;

struct ProcessInfo 
{
    int processID;              
    // 进程ID
    string processName;         
    // 进程名称
    long long memoryUsage;      
    // 进程内存使用量（以字节为单位）
    int duration;               
    // 进程的持续时间（秒）
};

struct ActiveProcessNode 
{
    ProcessInfo data;          
    // 存储进程信息的结构体
    ActiveProcessNode* next;   
    // 指向下一个活动进程节点的指针
};

struct EndedProcessNode {
    ProcessInfo data;         
     // 存储进程信息的结构体
    EndedProcessNode* prev;    
    // 指向前一个已结束进程节点的指针
    EndedProcessNode* next;    
    // 指向下一个已结束进程节点的指针
};

ActiveProcessNode* activeHead = nullptr;  
 // 活动进程链表的头指针
EndedProcessNode* endedHead = nullptr;    
// 已结束进程链表的头指针

// 向活动进程链表中插入新进程
void InsertActiveProcess(ProcessInfo process) {
    ActiveProcessNode* newNode = new ActiveProcessNode{ process, nullptr }; 
    // 创建新的活动进程节点
    if (!activeHead || activeHead->data.memoryUsage < process.memoryUsage) {
        newNode->next = activeHead; 
        // 如果链表为空或当前进程内存使用大于头节点，则插入到链表头
        activeHead = newNode;
    }
    else {
        ActiveProcessNode* current = activeHead;
        while (current->next && current->next->data.memoryUsage >= process.memoryUsage) {
            current = current->next; 
            // 按照内存使用量排序，找到合适的位置插入
        }
        newNode->next = current->next;  
        // 插入新节点
        current->next = newNode;
    }
}

// 向已结束进程链表中插入新进程
void InsertEndedProcess(ProcessInfo process) {
    EndedProcessNode* newNode = new EndedProcessNode{ process, nullptr, nullptr }; 
    // 创建新的已结束进程节点
    if (!endedHead || endedHead->data.duration > process.duration) {
        newNode->next = endedHead;   
        // 如果链表为空或当前进程持续时间小于头节点，则插入到链表头
        if (endedHead) endedHead->prev = newNode;
        endedHead = newNode;
    }
    else {
        EndedProcessNode* current = endedHead;
        while (current->next && current->next->data.duration <= process.duration) {
            current = current->next;   
            // 按照持续时间排序，找到合适的位置插入
        }
        newNode->next = current->next; 
        // 插入新节点
        if (current->next) current->next->prev = newNode;
        current->next = newNode;
        newNode->prev = current;
    }
}

// 更新活动进程的持续时间，每次调用时持续时间加 1
void UpdateProcessDuration() 
{
    ActiveProcessNode* current = activeHead;
    while (current) 
    {
        current->data.duration++;  // 每秒钟持续时间加 1
        current = current->next;
    }
}

// 检查已结束进程是否重新启动
void CheckForRestartedProcesses() 
{
    EndedProcessNode* current = endedHead;
    while (current) 
    {
        // 检查该进程是否已经重新出现在活动链表中
        ActiveProcessNode* checkNode = activeHead;
        bool found = false;
        while (checkNode) 
        {
            if (checkNode->data.processID == current->data.processID) 
            {
                found = true;  // 如果找到了重新启动的进程
                break;
            }
            checkNode = checkNode->next;
        }

        // 如果进程已重新启动，从已结束链表中移除它
        if (found) 
        {
            // 删除该节点
            if (current->prev) 
            {
                current->prev->next = current->next;
            }
            else 
            {
                endedHead = current->next;
            }
            if (current->next) 
            {
                current->next->prev = current->prev;
            }
            EndedProcessNode* temp = current;
            current = current->next;
            delete temp;  // 删除已结束进程节点
        }
        else 
        {
            current = current->next;
        }
    }
}

// 更新活动进程链表
void UpdateProcesses() {
    int processes[1024], needed, count;
    EnumProcesses((DWORD*)processes, sizeof(processes), (DWORD*)&needed); // 获取当前系统中的所有进程ID
    count = needed / sizeof(int);  // 计算实际的进程数

    // 更新活动进程的持续时间
    UpdateProcessDuration();

    // 遍历系统中的每个进程
    for (int i = 0; i < count; i++)
    {
        if (processes[i] != 0) 
        {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
            if (hProcess) 
            {
                PROCESS_MEMORY_COUNTERS pmc;
                if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) 
                {
                    string processName = "<unknown>";  
                    // 使用标准字符串类型来存储进程名称
                    HMODULE hMod;
                    DWORD cbNeeded;
                    if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) 
                    {
                        char name[MAX_PATH];
                        GetModuleBaseNameA(hProcess, hMod, name, sizeof(name)); 
                        // 获取进程名称
                        processName = name;  
                        // 转换为 C++ string
                    }
                    ProcessInfo processInfo = 
                    { 
                        processes[i], processName, pmc.WorkingSetSize, 0 
                    };
                     // 创建进程信息

                    // 判断该进程是否在活动链表中，如果没有就插入
                    bool found = false;
                    ActiveProcessNode* checkNode = activeHead;
                    while (checkNode) 
                    {
                        if (checkNode->data.processID == processes[i]) 
                        {
                            found = true;  
                            // 如果进程已经在链表中
                            break;
                        }
                        checkNode = checkNode->next;
                    }
                    if (!found) 
                    {
                        InsertActiveProcess(processInfo); 
                        // 如果没有找到该进程，插入到活动链表中
                    }
                }
                CloseHandle(hProcess);
            }
        }
    }

    // 遍历活动进程链表，将已经结束的进程移到已结束链表
    ActiveProcessNode* prev = nullptr;
    ActiveProcessNode* current = activeHead;
    while (current) 
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, current->data.processID);
        if (!hProcess) 
        {
            // 进程结束，将其移到已结束链表
            ProcessInfo endedProcess = current->data;
            InsertEndedProcess(endedProcess); 
            // 插入到已结束链表

            // 删除当前节点
            if (prev) 
            {
                prev->next = current->next;
            }
            else 
            {
                activeHead = current->next;
            }
            ActiveProcessNode* temp = current;
            current = current->next;
            delete temp;  // 删除当前活动进程节点
        }
        else 
        {
            prev = current;
            current = current->next;
            CloseHandle(hProcess);
        }
    }

    // 检查已结束进程是否重新启动
    CheckForRestartedProcesses();
}

// 打印活动进程信息
void PrintActiveProcesses() 
{
    cout << "Active Processes:" << endl;
    ActiveProcessNode* current = activeHead;
    while (current) 
    {
        cout << "Process ID: " << current->data.processID << ", Name: " << current->data.processName
            << ", Memory Usage: " << current->data.memoryUsage << " KB, Duration: " << current->data.duration << "s" << endl;
        current = current->next;
    }
}

// 打印已结束进程信息
void PrintEndedProcesses() {
    cout << "Ended Processes:" << endl;
    EndedProcessNode* current = endedHead;
    while (current)  
    {
        cout << "Process ID: " << current->data.processID << ", Name: " << current->data.processName
            << ", Duration: " << current->data.duration << "s" << endl;
        current = current->next;
    }
}

int main() {
    while (true) {
        UpdateProcesses();           // 更新进程信息
        PrintActiveProcesses();      // 打印当前活动进程
        PrintEndedProcesses();       // 打印已结束进程
        this_thread::sleep_for(chrono::seconds(1));  // 每秒钟刷新一次
    }
    return 0;
}
