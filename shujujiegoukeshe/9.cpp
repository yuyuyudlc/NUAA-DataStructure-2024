#include<iostream>
#include<vector>
#include<queue>
#include<ctime>
#include<fstream>
#include <cstdlib>
#include <string>
#include <chrono>  // 需要添加头文件以使用时间测量
using namespace std;

void insert(vector<int> &p) {
    for (int i = 1; i < p.size(); i++) {
        int temp = p[i];
        int j = i - 1;
        while (j >= 0 && p[j] > temp) {
            p[j + 1] = p[j];
            j--;
        }
        p[j + 1] = temp;
    }
}

void shellSort(vector<int> &p) {
    int n = p.size();
    for (int gap = n / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < n; i++) {
            int temp = p[i];
            int j = i;
            while (j >= gap && p[j - gap] > temp) {
                p[j] = p[j - gap];
                j -= gap;
            }
            p[j] = temp;
        }
    }
}

void bubblesort(vector<int> &p) {
    for (int i = 0; i < p.size(); i++) {
        for (int j = 0; j < p.size() - i - 1; j++) {
            if (p[j] > p[j + 1]) {
                swap(p[j], p[j + 1]);
            }
        }
    }
}

void quicksort(vector<int> &p, int l, int r) {
    if (l >= r) return;
    int i = l, j = r;
    int temp = p[l];
    while (i < j) {
        while (i < j && p[j] >= temp) j--;
        p[i] = p[j];
        while (i < j && p[i] <= temp) i++;
        p[j] = p[i];
    }
    p[i] = temp;
    quicksort(p, l, i - 1);
    quicksort(p, i + 1, r);
}

void selectsort(vector<int> &p) {
    for (int i = 0; i < p.size(); i++) {
        int minIndex = i;
        for (int j = i + 1; j < p.size(); j++) {
            if (p[j] < p[minIndex]) {
                minIndex = j;
            }
        }
        swap(p[i], p[minIndex]);
    }
}

void heapSort(vector<int>& arr) {
    priority_queue<int, vector<int>, greater<int>> pq;  // 使用最小堆
    for (int num : arr) {
        pq.push(num);
    }
    int idx = 0;
    while (!pq.empty()) {
        arr[idx++] = pq.top();
        pq.pop();
    }
}

void merge(vector<int> &p, vector<int> &t, int l, int r) {
    if (l >= r) {
        return;
    }
    int mid = (l + r) / 2;
    merge(p, t, l, mid);
    merge(p, t, mid + 1, r);
    int k = 0;
    int i = l;
    int j = mid + 1;
    while (i <= mid && j <= r) {
        if (p[i] <= p[j]) {
            t[k++] = p[i++];
        } else {
            t[k++] = p[j++];
        }
    }
    while (i <= mid) {
        t[k++] = p[i++];
    }
    while (j <= r) {
        t[k++] = p[j++];
    }
    k = 0;
    for (int i = l; i <= r; i++) {
        p[i] = t[k++];
    }
}

int cal(int x) {
    int i = 0;
    while (x > 0) {
        x /= 10;
        i++;
    }
    return i;  // 返回位数
}

void radix(vector<int> &p) {
    int max1 = 0;
    for (int i = 0; i < p.size(); i++) {
        max1 = max(max1, p[i]);
    }
    max1 = cal(max1);
    vector<vector<int>> q(10);
    int r = 1;
    for (int i = 0; i < max1; i++) {
        for (int j = 0; j < p.size(); j++) {
            int k = p[j] / r % 10;
            q[k].push_back(p[j]);
        }
        int k = 0;
        for (int j = 0; j < 10; j++) {
            for (int l = 0; l < q[j].size(); l++) {
                p[k++] = q[j][l];
            }
        }
        for (int j = 0; j < 10; j++) {
            q[j].clear();
        }
        r *= 10;
    }
}

// 从文件读取数据
void loadDataFromFile(const string &file, vector<int> &p) {
    ifstream inFile(file);
    int num;
    while (inFile >> num) {
        p.push_back(num);
    }
}

// 测试排序算法
void testSortingAlgorithms(const string &file) {
    vector<int> data;
    loadDataFromFile(file, data);  // 从文件加载数据

    // 测试所有排序算法
    vector<int> dataCopy;

    // 插入排序
    dataCopy = data;
    auto start = chrono::high_resolution_clock::now();
    insert(dataCopy);
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
    cout << "Insertion Sort took " << duration.count() << " ms" << endl;

    // 希尔排序
    dataCopy = data;
    start = chrono::high_resolution_clock::now();
    shellSort(dataCopy);
    stop = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
    cout << "Shell Sort took " << duration.count() << " ms" << endl;

    // 冒泡排序
    dataCopy = data;
    start = chrono::high_resolution_clock::now();
    bubblesort(dataCopy);
    stop = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
    cout << "Bubble Sort took " << duration.count() << " ms" << endl;

    // 快速排序
    dataCopy = data;
    start = chrono::high_resolution_clock::now();
    quicksort(dataCopy, 0, dataCopy.size() - 1);
    stop = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
    cout << "Quick Sort took " << duration.count() << " ms" << endl;

    // 选择排序
    dataCopy = data;
    start = chrono::high_resolution_clock::now();
    selectsort(dataCopy);
    stop = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
    cout << "Selection Sort took " << duration.count() << " ms" << endl;

    // 堆排序
    dataCopy = data;
    start = chrono::high_resolution_clock::now();
    heapSort(dataCopy);
    stop = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
    cout << "Heap Sort took " << duration.count() << " ms" << endl;

    // 归并排序
    dataCopy = data;
    vector<int> temp(dataCopy.size());
    start = chrono::high_resolution_clock::now();
    merge(dataCopy, temp, 0, dataCopy.size() - 1);
    stop = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
    cout << "Merge Sort took " << duration.count() << " ms" << endl;

    // 基数排序
    dataCopy = data;
    start = chrono::high_resolution_clock::now();
    radix(dataCopy);
    stop = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
    cout << "Radix Sort took " << duration.count() << " ms" << endl;
}
void generateR(vector<int> &p,string file){
    srand(time(0));
    p.clear();
    for(int i=0;i<100000;i++){
        p.push_back(rand()%1000000);
    }
    ofstream outFile(file);
    for (const int &num : p) {
        outFile << num << endl;  // 每个数字占一行
    }
}
int main() {
    vector<int> p;
    for (char i = '0'; i <= '9'; i++) {
        string file = "data" + string(1, i) + ".txt";
        cout << "Testing " << file << endl;
        generateR(p,file);
        testSortingAlgorithms(file);  // 对每个文件进行排序测试
    }
    return 0;
}
