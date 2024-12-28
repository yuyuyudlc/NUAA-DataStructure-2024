#include <iostream>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <tuple>
#include <algorithm>

using namespace std;


struct Connection {
    int stationId;      // 目标站点ID
    int lineId;         // 所属线路ID
};

// 定义站点结构体
struct Station {
    int id;                             // 站点ID
    unordered_set<int> lines;           // 所属线路ID集合
    vector<Connection> connections;     // 相邻站点的连接信息
};

struct TransferState {
    int stationId;
    int currentLineId;
    int transfers;
    vector<int> path; // 路径记录
};
/**
 *  从文件中读取公交线路数据并构建线路图
 *
 *  filename 数据文件名
 *  stations 存储所有站点的映射（站点ID -> Station）
 *  true 读取成功
 *  false 读取失败
 */
bool readBusData(const string& filename, unordered_map<int, Station>& stations) {
    // 打开数据文件
    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "无法打开文件 " << filename << endl;
        return false;
    }

    string line;
    // 记录每条线路的最后一个站点ID
    unordered_map<int, int> lastStationPerLine;

    // 读取文件中的每一行
    while (getline(infile, line)) {
        // 跳过标题行
        if (line.find("公交线路ID") != string::npos) {
            continue;
        }

        // 使用字符串流解析每一行的数据
        stringstream ss(line);
        int lineId, stationOrder, stationId;
        if (!(ss >> lineId >> stationOrder >> stationId)) {
            cerr << "解析错误: " << line << endl;
            continue;  // 跳过解析失败的行
        }

        // 获取当前站点，如果不存在则创建
        if (stations.find(stationId) == stations.end()) {
            stations[stationId] = Station{stationId, {}, {}};
        }
        // 添加所属线路
        stations[stationId].lines.insert(lineId);

        // 如果不是起始站点，建立与前一个站点的连接
        if (stationOrder > 0) {
            if (lastStationPerLine.find(lineId) != lastStationPerLine.end()) {
                int lastStationId = lastStationPerLine[lineId];
                // 建立双向连接
                stations[lastStationId].connections.push_back(Connection{stationId, lineId});
                stations[stationId].connections.push_back(Connection{lastStationId, lineId});
            }
            // 更新最后一个站点
            lastStationPerLine[lineId] = stationId;
        } else {
            // 起始站点，更新最后一个站点
            lastStationPerLine[lineId] = stationId;
        }
    }

    infile.close();
    return true;
}


/**
 *  查找转车次数最少的路线
 *
 *  startId 起始站点ID
 *  endId 终点站点ID
 *  stations 站点图
 *  resultPath 结果路径
 *  true 找到路径
 *  false 未找到路径
 */
bool findMinTransfersPath(int startId, int endId, const unordered_map<int, Station>& stations, vector<int>& resultPath) {
    if (stations.find(startId) == stations.end() || stations.find(endId) == stations.end()) {
        cerr << "起始站点或终点站点不存在。" << endl;
        return false;
    }

    // 使用队列进行BFS
    queue<TransferState> q;

    // 访问标记：stationId -> lineId -> 最小转车次数
    unordered_map<int, unordered_map<int, int>> visited;

    // 初始化队列：从起始站点出发，可以选择任何一条线路
    for (const auto& line : stations.at(startId).lines) {
        TransferState state;
        state.stationId = startId;
        state.currentLineId = line;
        state.transfers = 0;
        state.path.push_back(startId);
        q.push(state);
        visited[startId][line] = 0;
    }

    while (!q.empty()) {
        TransferState current = q.front();
        q.pop();

        // 如果到达终点，记录路径
        if (current.stationId == endId) {
            resultPath = current.path;
            return true;
        }

        // 遍历相邻站点
        auto it = stations.find(current.stationId);
        if (it == stations.end()) continue;

        for (const auto& conn : it->second.connections) {
            int neighborId = conn.stationId;
            int lineId = conn.lineId;

            // 判断是否需要转车
            int newTransfers = current.transfers;
            if (lineId != current.currentLineId) {
                newTransfers += 1;
            }

            // 检查是否已经访问过，或者是否有更少的转车次数
            if (visited.find(neighborId) != visited.end() &&
                visited.at(neighborId).find(lineId) != visited.at(neighborId).end() &&
                visited.at(neighborId).at(lineId) <= newTransfers) {
                continue;
            }

            // 标记为已访问
            visited[neighborId][lineId] = newTransfers;

            // 记录路径
            TransferState nextState;
            nextState.stationId = neighborId;
            nextState.currentLineId = lineId;
            nextState.transfers = newTransfers;
            nextState.path = current.path;
            nextState.path.push_back(neighborId);
            q.push(nextState);
        }
    }

    // 如果没有找到路径
    return false;
}

/**
 *  查找经过站点最少的路线
 *
 *  startId 起始站点ID
 *  endId 终点站点ID
 *  stations 站点图
 *  resultPath 结果路径
 *  true 找到路径
 *  false 未找到路径
 */
bool findMinStopsPath(int startId, int endId, const unordered_map<int, Station>& stations, vector<int>& resultPath) {
    if (stations.find(startId) == stations.end() || stations.find(endId) == stations.end()) {
        cerr << "起始站点或终点站点不存在。" << endl;
        return false;
    }

    // 使用队列进行BFS
    queue<vector<int>> q;
    unordered_set<int> visited;
    visited.insert(startId);

    // 初始化队列
    q.push({startId});

    while (!q.empty()) {
        vector<int> path = q.front();
        q.pop();

        int currentStation = path.back();

        // 如果到达终点，记录路径
        if (currentStation == endId) {
            resultPath = path;
            return true;
        }

        // 遍历相邻站点
        auto it = stations.find(currentStation);
        if (it == stations.end()) continue;

        for (const auto& conn : it->second.connections) {
            int neighborId = conn.stationId;
            if (visited.find(neighborId) == visited.end()) {
                visited.insert(neighborId);
                vector<int> newPath = path;
                newPath.push_back(neighborId);
                q.push(newPath);
            }
        }
    }

    // 如果没有找到路径
    return false;
}

int main() {
    // 创建一个存储所有站点的映射
    unordered_map<int, Station> stations;

    // 调用函数读取数据文件
    string filename = "road_test.txt";
    if (!readBusData(filename, stations)) {
        return 1;  // 如果读取失败，退出程序
    }

    // 用户交互
    while (true) {
        cout << "\n请选择操作:\n";
        cout << "1. 输入任意两站点，给出转车次数最少的乘车路线。\n";
        cout << "2. 输入任意两站点，给出经过站点最少的乘车路线。\n";
        cout << "3. 退出。\n";
        cout << "请输入选项(1/2/3): ";
        int choice;
        cin >> choice;

        if (choice == 3) {
            cout << "退出程序。" << endl;
            break;
        }

        if (choice != 1 && choice != 2) {
            cout << "无效的选项，请重新选择。" << endl;
            continue;
        }

        int startId, endId;
        cout << "请输入起始站点ID: ";
        cin >> startId;
        cout << "请输入终点站点ID: ";
        cin >> endId;

        vector<int> path;
        if (choice == 1) {
            // 最少转车次数
            if (findMinTransfersPath(startId, endId, stations, path)) {
                cout << "转车次数最少的路线 (" << path.size() - 1 << " 次转车): ";
                for (size_t i = 0; i < path.size(); ++i) {
                    cout << path[i];
                    if (i != path.size() - 1) cout << " -> ";
                }
                cout << endl;
            } else {
                cout << "未找到从站点 " << startId << " 到站点 " << endId << " 的路线。" << endl;
            }
        } else if (choice == 2) {
            // 最少经过站点
            if (findMinStopsPath(startId, endId, stations, path)) {
                cout << "经过站点最少的路线 (" << path.size() - 1 << " 个站点): ";
                for (size_t i = 0; i < path.size(); ++i) {
                    cout << path[i];
                    if (i != path.size() - 1) cout << " -> ";
                }
                cout << endl;
            } else {
                cout << "未找到从站点 " << startId << " 到站点 " << endId << " 的路线。" << endl;
            }
        }
    }

    return 0;
}
