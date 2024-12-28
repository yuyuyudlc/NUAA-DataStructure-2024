#include <iostream>
#include <vector>
#include <fstream>
#include <queue>
#include <climits>
#include <algorithm>
#define MAX 9999999.0
using namespace std;

struct Edge {
    int u, v;
    float weight;
    
    // 用于排序
    bool operator<(Edge& other) const {
        return weight < other.weight;
    }
};

class Graph {
private:
    int V; // 顶点数
    vector<Edge> edges; // 边集合
    
    // 并查集查找函数
    int find(vector<int>& parent, int i) {
        if (parent[i] != i) {
            parent[i] = find(parent, parent[i]);
        }
        return parent[i];
    }
    
    // 并查集合并函数
    void unionSets(vector<int>& parent, int x, int y) {
        parent[find(parent, x)] = find(parent, y);
    }

public:
    // 从文件读取图数据
    Graph(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "无法打开文件" << filename << endl;
            exit(1);
        }

        // 读取顶点数
        file >> V;

        // 读取边的信息
        int u, v;
        float w;
        while (file >> u >> v >> w) {
            edges.push_back({u, v, w});
        }
        file.close();
    }

    // Prim算法实现
    void primMST() {
        vector<bool> visited(V, false);
        vector<float> key(V, MAX);
        vector<int> parent(V, -1);
        
        // 从顶点0开始
        key[0] = 0;
        
        float totalWeight = 0;
        cout << "\nPrim算法生成的最小生成树：" << endl;
        
        for (int count = 0; count < V; count++) {
            // 找到未访问顶点中key值最小的顶点
            float minKey = MAX;
            int u = -1;
            
            for (int v = 0; v < V; v++) {
                if (!visited[v] && key[v] < minKey) {
                    minKey = key[v];
                    u = v;
                }
            }
            
            if (u == -1) break;
            
            visited[u] = true;
            
            // 如果不是起始顶点，输出这条边
            if (parent[u] != -1) {
                cout << parent[u] << " - " << u << " : " << key[u] << endl;
                totalWeight += key[u];
            }
            
            // 更新相邻顶点的key值
            for (const Edge& edge : edges) {
                if ((edge.u == u || edge.v == u) && !visited[edge.u == u ? edge.v : edge.u]) {
                    int v = edge.u == u ? edge.v : edge.u;
                    if (edge.weight < key[v]) {
                        parent[v] = u;
                        key[v] = edge.weight;
                    }
                }
            }
        }
        
        cout << "最小生成树的总权值：" << totalWeight << endl;
    }

    // Kruskal算法实现
    void kruskalMST() {
        vector<Edge> result;
        vector<int> parent(V);
        
        // 初始化并查集
        for (int i = 0; i < V; i++) {
            parent[i] = i;
        }
        
        // 对边按权重排序
        sort(edges.begin(), edges.end());
        
        float totalWeight = 0;
        cout << "\nKruskal算法生成的最小生成树：" << endl;
        
        for (const Edge& edge : edges) {
            int setU = find(parent, edge.u);
            int setV = find(parent, edge.v);
            
            // 如果加入这条边不会形成环
            if (setU != setV) {
                cout << edge.u << " - " << edge.v << " : " << edge.weight << endl;
                totalWeight += edge.weight;
                unionSets(parent, setU, setV);
            }
        }
        
        cout << "最小生成树的总权值：" << totalWeight << endl;
    }
};

int main() {
    string filename = "graph.txt";
    Graph g(filename);
    
    cout << "使用Prim算法：";
    g.primMST();
    
    cout << "\n使用Kruskal算法：";
    g.kruskalMST();
    
    return 0;
}
