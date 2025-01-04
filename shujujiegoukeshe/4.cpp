#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
using namespace std;

// 定义AVL树的节点结构
struct Node {
    int data;
    int height;  // 节点的高度
    Node* left;
    Node* right;
    Node(int val) : data(val), height(0), left(nullptr), right(nullptr) {}
};

// 获取节点高度
int height(Node* node) {
    return node == nullptr ? -1 : node->height;
}

// 更新节点的高度
void updateHeight(Node* &node) {
    if (node == nullptr) return;
    int leftHeight = (node->left != nullptr) ? node->left->height : -1;
    int rightHeight = (node->right != nullptr) ? node->right->height : -1;
    node->height = max(leftHeight, rightHeight) + 1;
}

// 获取平衡因子
int balanceFactor(Node* node) {
    return height(node->left) - height(node->right);
}

// 左旋操作
void leftRotate(Node*& root) {
    Node* child = root->right;
    if (child == nullptr) return;
    Node* grandchild = child->left;
    child->left = root;
    root->right = grandchild;
    updateHeight(root);
    updateHeight(child);
    root = child;  
}

// 右旋操作
void rightRotate(Node*& root) {
    Node* child = root->left;
    Node* grandchild = child->right;
    child->right = root;
    root->left = grandchild;
    updateHeight(root);
    updateHeight(child);
    root = child;  
}

// 插入操作
void insert(Node*& root, int value) {
    if (root == nullptr) {
        root = new Node(value);
        return;
    }
    if (value < root->data) {
        insert(root->left, value);
    } else if (value > root->data) {
        insert(root->right, value);
    }

    updateHeight(root);

    int balance = balanceFactor(root);

    // 左左情况
    if (balance > 1 && value < root->left->data) {
        rightRotate(root);
    }
    // 右右情况
    if (balance < -1 && value > root->right->data) {
        leftRotate(root);
    }
    // 左右情况
    if (balance > 1 && value > root->left->data) {
        leftRotate(root->left);
        rightRotate(root);
    }
    // 右左情况
    if (balance < -1 && value < root->right->data) {
        rightRotate(root->right);
        leftRotate(root);
    }
}

// 删除节点
Node* deleteNode(Node* root, int value) {
    if (root == nullptr) return root;

    if (value < root->data) {
        root->left = deleteNode(root->left, value);
    } else if (value > root->data) {
        root->right = deleteNode(root->right, value);
    } else {
        if (root->left == nullptr) {
            Node* temp = root->right;
            delete root;
            return temp;
        } else if (root->right == nullptr) {
            Node* temp = root->left;
            delete root;
            return temp;
        } else {
            Node* temp = root->right;
            while (temp && temp->left != nullptr) {
                temp = temp->left;
            }
            root->data = temp->data;
            root->right = deleteNode(root->right, temp->data);
        }
    }

    updateHeight(root);

    int balance = balanceFactor(root);

    // 左左情况
    if (balance > 1 && balanceFactor(root->left) >= 0) {
        rightRotate(root);
    }
    // 右右情况
    if (balance < -1 && balanceFactor(root->right) <= 0) {
        leftRotate(root);
    }
    // 左右情况
    if (balance > 1 && balanceFactor(root->left) < 0) {
        leftRotate(root->left);
        rightRotate(root);
    }
    // 右左情况
    if (balance < -1 && balanceFactor(root->right) > 0) {
        rightRotate(root->right);
        leftRotate(root);
    }

    return root;
}

// 查询节点
bool search(Node* root, int value) 
{
    if (root == nullptr) return false;
    if (value == root->data) return true;
    if (value < root->data) return search(root->left, value);
    return search(root->right, value);
}

// 生成质数

// 写入查询结果到文件
vector<int> Eratosthenes(int m) {
    vector<bool> isPrime(m + 1, true);
    isPrime[0] = isPrime[1] = false; 
    for (int i = 2; i * i <= m; ++i) 
    {
        if (isPrime[i]) {
            for (int j = i * i; j <= m; j += i) {
                isPrime[j] = false;
            }
        }
    }

    vector<int> primes;
    for (int i = 2; i <= m; ++i) 
    {
        if (isPrime[i]) {
            primes.push_back(i);
        }
    }

    return primes;
}

// 从 n 到 m 生成质数并返回
vector<int> generatePrimes(int n, int m) 
{
    vector<int> primes = Eratosthenes(m);
    // 筛选出 n, m 范围内的质数
    vector<int> result;
    for (int prime : primes) 
    {
        if (prime >= n) 
        {
            result.push_back(prime);
        }
    }
    return result;
}
void writeQueryResults(const vector<int>& queries, Node* root, const string& filename) 
{
    ofstream outfile(filename);
    for (int query : queries) 
    {
        outfile << query << " " << (search(root, query) ? "yes" : "no") << endl;
    }
    outfile.close();
}

int main() {
    // 生成质数
    vector<int> primes = generatePrimes(0,10000);

    // 创建平衡二叉排序树并插入1-10000之间的质数
    Node* root = nullptr;
    for (int prime : primes) 
    {
        insert(root, prime);
    }

    // (1) 查询200-300之间的质数
    vector<int> query1 =generatePrimes(200,300);
    writeQueryResults(query1, root, "tree1.txt");

    // (2) 删除500-2000之间的质数并查询600-700之间的质数
    vector<int> deletePrimes;
    for (int prime : primes) 
    {
        if (prime >= 500 && prime <= 2000) 
        {
            deletePrimes.push_back(prime);
        }
    }
    for (int prime : deletePrimes) 
    {
        root = deleteNode(root, prime);
    }

    vector<int> query2 = generatePrimes(600,700);
    writeQueryResults(query2, root, "tree2.txt");

    // (3) 插入1-1000之间的偶数并查询100-200之间的偶数
    for (int i = 2; i <= 1000; i += 2) 
    {
        insert(root, i);
    }

    vector<int> query3;
    for (int i = 100; i <= 200; i += 2) 
    {
        query3.push_back(i);
    }
    writeQueryResults(query3, root, "tree3.txt");

    return 0;
}
