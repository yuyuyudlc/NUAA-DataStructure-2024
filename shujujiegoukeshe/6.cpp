#include<iostream>
#include<algorithm>
#include<cstring>
using namespace std;

#define N 100
int h[N], e[N], ne[N], w[N], idx;
int n, m;
bool visited[N];  // 记录节点是否被访问过
int minTime = INT_MAX;  // 用来记录最小的最大施工时间
void add(int a, int b, int c) {
    e[idx] = b;
    w[idx] = c;
    ne[idx] = h[a];
    h[a] = idx++;
}

// DFS 深度优先搜索
void dfs(int node, int currentMaxTime) {
    if (node == n) {
        minTime = min(minTime, currentMaxTime);
        return;
    }

    // 遍历当前节点的所有邻接节点
    for (int i = h[node]; i != -1; i = ne[i]) {
        int neighbor = e[i];
        int time = w[i];

        if (!visited[neighbor]) {
            visited[neighbor] = true;

            // 更新当前路径中的最大施工时间
            int newMaxTime = max(currentMaxTime, time);

            // 继续搜索
            dfs(neighbor, newMaxTime);

            visited[neighbor] = false;  // 回溯，撤销访问
        }
    }
}

int main() {
    memset(h, -1, sizeof(h));  // 初始化邻接表为空
    memset(visited, false, sizeof(visited));  // 初始化访问标记为false

    // 输入交通枢纽和隧道数目
    cin >> n >> m;

    // 输入所有的隧道信息
    for (int i = 0; i < m; i++) {
        int a, b, c;
        cin >> a >> b >> c;
        add(a, b, c);
        add(b, a, c);  
    }


    visited[1] = true;
    dfs(1, 0);  // 从1号开始搜索，最大施工时间初始为0

    // 输出最小的最大施工时间
    cout << minTime << endl;

    return 0;
}
