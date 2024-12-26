#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#define order 4  // B树的阶数
using namespace std;
// 在 Node 结构体后，其他函数声明之前添加：
struct Node;  // 前向声明
void remove(Node*& root, int key);
void remove(Node*& root, Node* node, int key);
void rebalance(Node*& root, Node* node);

struct Node {
    vector<int> data;
    vector<Node*> next;
    Node* parent;
    bool leaf = true;
    
    Node() {
        parent = nullptr;
    }
};

// 判断是否为质数
bool isPrime(int n) {
    if (n < 2) return false;
    for (int i = 2; i <= sqrt(n); i++) {
        if (n % i == 0) return false;
    }
    return true;
}

// 查找函数
Node* Find(Node* root, int x) {
    if (!root) return nullptr;
    
    int i = 0;
    while (i < root->data.size() && root->data[i] < x) {
        i++;
    }
    
    if (i < root->data.size() && root->data[i] == x) {
        return root;
    }
    
    if (root->leaf) {
        return nullptr;
    }
    
    return Find(root->next[i], x);
}

// 分裂节点
void split(Node*& root, Node* node) {
    int mid = node->data.size() / 2;
    int mid_value = node->data[mid];
    
    Node* right = new Node();
    right->leaf = node->leaf;
    
    // 移动数据到右节点
    for (int i = mid + 1; i < node->data.size(); i++) {
        right->data.push_back(node->data[i]);
    }
    
    // 移动子节点指针
    if (!node->leaf) {
        for (int i = mid + 1; i <= node->data.size(); i++) {
            right->next.push_back(node->next[i]);
            node->next[i]->parent = right;
        }
    }
    
    // 调整原节点
    node->data.resize(mid);
    if (!node->leaf) {
        node->next.resize(mid + 1);
    }
    
    if (node->parent == nullptr) {
        // 创建新根
        Node* new_root = new Node();
        new_root->leaf = false;
        new_root->data.push_back(mid_value);
        new_root->next.push_back(node);
        new_root->next.push_back(right);
        node->parent = new_root;
        right->parent = new_root;
        root = new_root;
    } else {
        // 插入到父节点
        Node* parent = node->parent;
        int pos = 0;
        while (pos < parent->data.size() && parent->data[pos] < mid_value) {
            pos++;
        }
        parent->data.insert(parent->data.begin() + pos, mid_value);
        parent->next.insert(parent->next.begin() + pos + 1, right);
        right->parent = parent;
        
        if (parent->data.size() > order - 1) {
            split(root, parent);
        }
    }
}

// 插入函数
void insert(Node*& root, int x) {
    if (!root) {
        root = new Node();
        root->data.push_back(x);
        return;
    }
    
    Node* node = root;
    while (!node->leaf) {
        int i = 0;
        while (i < node->data.size() && node->data[i] < x) {
            i++;
        }
        node = node->next[i];
    }
    
    // 插入到叶子节点
    int i = 0;
    while (i < node->data.size() && node->data[i] < x) {
        i++;
    }
    
    // 避免重复插入
    if (i < node->data.size() && node->data[i] == x) {
        return;
    }
    
    node->data.insert(node->data.begin() + i, x);
    
    if (node->data.size() > order - 1) {
        split(root, node);
    }
}

// 查找最小值
int findMin(Node* root) {
    if (!root) return -1;
    while (!root->leaf) {
        root = root->next[0];
    }
    return root->data[0];
}

// 查找最大值
int findMax(Node* root) {
    if (!root) return -1;
    while (!root->leaf) {
        root = root->next[root->next.size() - 1];
    }
    return root->data[root->data.size() - 1];
}

// 合并节点
void merge(Node*& root, Node* parent, int index) {
    Node* left = parent->next[index];
    Node* right = parent->next[index + 1];
    
    // 将父节点的关键字下移
    left->data.push_back(parent->data[index]);
    
    // 将右兄弟的关键字和子节点合并到左兄弟
    for (int i = 0; i < right->data.size(); i++) {
        left->data.push_back(right->data[i]);
    }
    
    if (!right->leaf) {
        for (int i = 0; i < right->next.size(); i++) {
            left->next.push_back(right->next[i]);
            right->next[i]->parent = left;
        }
    }
    
    // 从父节点删除关键字和指针
    parent->data.erase(parent->data.begin() + index);
    parent->next.erase(parent->next.begin() + index + 1);
    
    delete right;
    
    // 如果父节点是根节点且为空，更新根
    if (parent == root && parent->data.empty()) {
        root = left;
        root->parent = nullptr;
        delete parent;
    }
    // 如果父节点关键字太少，需要重新平衡
    else if (parent != root && parent->data.size() < (order - 1) / 2) {
        rebalance(root, parent);
    }
}

// 从兄弟节点借一个关键字
void borrowFromSibling(Node* node, int index, bool fromLeft) {
    Node* parent = node->parent;
    
    if (fromLeft) {
        Node* leftSibling = parent->next[index - 1];
        
        // 将父节点的关键字下移到当前节点
        node->data.insert(node->data.begin(), parent->data[index - 1]);
        
        // 将左兄弟的最大关键字上移到父节点
        parent->data[index - 1] = leftSibling->data.back();
        leftSibling->data.pop_back();
        
        if (!node->leaf) {
            // 移动对应的子节点
            node->next.insert(node->next.begin(), leftSibling->next.back());
            leftSibling->next.back()->parent = node;
            leftSibling->next.pop_back();
        }
    } else {
        Node* rightSibling = parent->next[index + 1];
        
        // 将父节点的关键字下移到当前节点
        node->data.push_back(parent->data[index]);
        
        // 将右兄弟的最小关键字上移到父节点
        parent->data[index] = rightSibling->data.front();
        rightSibling->data.erase(rightSibling->data.begin());
        
        if (!node->leaf) {
            // 移动对应的子节点
            node->next.push_back(rightSibling->next.front());
            rightSibling->next.front()->parent = node;
            rightSibling->next.erase(rightSibling->next.begin());
        }
    }
}

// 重新平衡节点
void rebalance(Node*& root, Node* node) {
    if (node == root) return;
    
    Node* parent = node->parent;
    int index = 0;
    while (index < parent->next.size() && parent->next[index] != node) {
        index++;
    }
    
    // 尝试从左兄弟借
    if (index > 0) {
        Node* leftSibling = parent->next[index - 1];
        if (leftSibling->data.size() > (order - 1) / 2) {
            borrowFromSibling(node, index, true);
            return;
        }
    }
    
    // 尝试从右兄弟借
    if (index < parent->next.size() - 1) {
        Node* rightSibling = parent->next[index + 1];
        if (rightSibling->data.size() > (order - 1) / 2) {
            borrowFromSibling(node, index, false);
            return;
        }
    }
    
    // 如果无法借，则需要合并
    if (index > 0) {
        merge(root, parent, index - 1);
    } else {
        merge(root, parent, index);
    }
}

// 从叶子节点删除关键字
void deleteFromLeaf(Node*& root, Node* node, int key) {
    int index = 0;
    while (index < node->data.size() && node->data[index] < key) {
        index++;
    }
    
    if (index < node->data.size() && node->data[index] == key) {
        node->data.erase(node->data.begin() + index);
        
        if (node == root) {
            if (node->data.empty()) {
                delete root;
                root = nullptr;
            }
        }
        else if (node->data.size() < (order - 1) / 2) {
            rebalance(root, node);
        }
    }
}

// 从内部节点删除关键字
void deleteFromInternal(Node*& root, Node* node, int key) {
    int index = 0;
    while (index < node->data.size() && node->data[index] < key) {
        index++;
    }
    
    if (index < node->data.size() && node->data[index] == key) {
        // 找到前驱或后继来替换
        if (node->next[index]->data.size() >= (order + 1) / 2) {
            // 使用前驱
            int pred = findMax(node->next[index]);
            node->data[index] = pred;
            remove(root, node->next[index], pred);
        } else if (node->next[index + 1]->data.size() >= (order + 1) / 2) {
            // 使用后继
            int succ = findMin(node->next[index + 1]);
            node->data[index] = succ;
            remove(root, node->next[index + 1], succ);
        } else {
            // 合并节点
            merge(root, node, index);
            remove(root, key);
        }
    } else if (!node->leaf) {
        remove(root, node->next[index], key);
    }
}

// 删除函数
void remove(Node*& root, Node* node, int key) {
    if (!node) return;
    
    int index = 0;
    while (index < node->data.size() && node->data[index] < key) {
        index++;
    }
    
    if (node->leaf) {
        deleteFromLeaf(root, node, key);
    } else {
        if (index < node->data.size() && node->data[index] == key) {
            deleteFromInternal(root, node, key);
        } else {
            remove(root, node->next[index], key);
        }
    }
}

// 删除入口函数
void remove(Node*& root, int key) {
    if (!root) return;
    remove(root, root, key);
}

// 将查询结果写入文件
void writeResult(const string& filename, int num, bool found) {
    ofstream out(filename, ios::app);
    out << num << (found ? " yes" : " no") << endl;
    out.close();
}

int main() {
    Node* root = nullptr;
    
    // 构建初始B树（插入1-10000的所有质数）
    for (int i = 1; i <= 10000; i++) {
        if (isPrime(i)) {
            insert(root, i);
        }
    }
    
    // 任务1：查询200-300的每个数
    ofstream out1("b-tree1.txt");
    out1.close();  // 清空文件
    for (int i = 200; i <= 300; i++) {
        bool found = (Find(root, i) != nullptr);
        writeResult("b-tree1.txt", i, found);
    }
    
    // 任务2：删除500-2000中的质数并查询600-700的质数
    for (int i = 500; i <= 2000; i++) {
        if (isPrime(i)) {
            remove(root, i);
        }
    }
    
    ofstream out2("b-tree2.txt");
    out2.close();
    for (int i = 600; i <= 700; i++) {
        if (isPrime(i)) {
            bool found = (Find(root, i) != nullptr);
            writeResult("b-tree2.txt", i, found);
        }
    }
    
    // 任务3：插入1-1000的偶数并查询100-200的偶数
    for (int i = 2; i <= 1000; i += 2) {
        insert(root, i);
    }
    
    ofstream out3("b-tree3.txt");
    out3.close();
    for (int i = 100; i <= 200; i += 2) {
        bool found = (Find(root, i) != nullptr);
        writeResult("b-tree3.txt", i, found);
    }
    
    return 0;
}