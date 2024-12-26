#include <iostream>
using namespace std;

// 自定义 pair 结构体
template <typename T1, typename T2>
struct MyPair {
    T1 first;
    T2 second;

    MyPair() : first(), second() {}
    MyPair(T1 a, T2 b) : first(a), second(b) {}
};

// 自定义栈结构体
template <typename T>
struct MyStack {
    struct Node {
        T data;
        Node* next;
        Node(T val) : data(val), next(nullptr) {}
    };

    Node* topNode;  // 栈顶元素

    MyStack() : topNode(nullptr) {}

    // 判断栈是否为空
    bool empty() const {
        return topNode == nullptr;
    }

    // 入栈
    void push(const T& value) {
        Node* newNode = new Node(value);
        newNode->next = topNode;
        topNode = newNode;
    }

    // 出栈
    void pop() {
        if (!empty()) {
            Node* temp = topNode;
            topNode = topNode->next;
           // delete temp;
        }
    }

    // 获取栈顶元素
    T top() const {
        if (!empty()) {
            return topNode->data;
        }
        throw runtime_error("Stack is empty");
    }

    // 析构栈，释放内存
    ~MyStack() {
        while (!empty()) {
            pop();
        }
    }
};

// 定义常量和数组
int n = 8, ans = 0;
int d[50] = {0}, ud[50] = {0}, b[50] = {0};
MyStack<MyPair<int, int>> st;  // 使用自定义栈类型

void f() {
    int step = 1;
    int col = 1;
    while (true) {
        bool found = false;

        for (; col <= n; col++) {
            if (d[col] == 0 && ud[col + step - 1] == 0 && b[col - step + n - 1] == 0) {
                d[col] = 1;
                ud[col + step - 1] = 1;
                b[col - step + n - 1] = 1;
                st.push(MyPair<int, int>(step, col));
                step++;
                col = 1;
                found = true;
                break;
            }
        }

        if (!found) {
            if (st.empty()) break;
            MyPair<int, int> last = st.top();
            st.pop();
            step = last.first;
            col = last.second;
            d[col] = 0;
            ud[col + step - 1] = 0;
            b[col - step + n - 1] = 0;
            col++;
        } else if (step == n + 1) {
            if (ans < 100) {
                MyStack<MyPair<int, int>> temp = st;
                int s[20] = {0};
                while (!temp.empty()) {
                    s[temp.top().first] = temp.top().second;
                    temp.pop();
                }
                for (int i = 1; i <= n; i++) {
                    cout << s[i] << " ";
                }
                cout << endl;
            }
            ans++;
            MyPair<int, int> last = st.top();
            st.pop();
            step = last.first;
            col = last.second;
            d[col] = 0;
            ud[col + step - 1] = 0;
            b[col - step + n - 1] = 0;
            col++;
        }
    }
    cout << ans;
}

int main() {
    f();
    return 0;
}
