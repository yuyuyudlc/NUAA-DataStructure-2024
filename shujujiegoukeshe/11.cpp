#include <iostream>
#include <string>
#include <unordered_map>
using namespace std;

// 自定义栈
template <typename T>
class MyStack {
private:
    T* arr;  // 存储栈元素的数组
    int capacity;  // 栈的最大容量
    int size;  // 当前栈的大小

public:
    MyStack(int cap = 100) {  // 构造函数，设置初始容量
        capacity = cap;
        arr = new T[capacity];
        size = 0;
    }

    ~MyStack() {  // 析构函数，释放内存
        delete[] arr;
    }

    void push(const T& value) {
        if (size == capacity) {
            // 栈满时，扩大容量
            capacity *= 2;
            T* newArr = new T[capacity];
            for (int i = 0; i < size; i++) {
                newArr[i] = arr[i];
            }
            delete[] arr;
            arr = newArr;
        }
        arr[size++] = value;
    }

    void pop() {
        if (size > 0) {
            --size;
        }
    }

    T top() const {
        if (size > 0) {
            return arr[size - 1];
        }
        throw runtime_error("Stack is empty");
    }

    bool empty() const {
        return size == 0;
    }

    int getSize() const {
        return size;
    }
};
// 自定义动态数组
template <typename T>
class MyVector {
private:
    T* arr;  // 存储数组元素的数组
    int capacity;  // 数组的最大容量
    int size;  // 当前数组的大小

public:
    MyVector(int cap = 100) {
        capacity = cap;
        arr = new T[capacity];
        size = 0;
    }

    ~MyVector() {
        delete[] arr;
    }

    void push_back(const T& value) {
        if (size == capacity) {
            // 扩容
            capacity *= 2;
            T* newArr = new T[capacity];
            for (int i = 0; i < size; i++) {
                newArr[i] = arr[i];
            }
            delete[] arr;
            arr = newArr;
        }
        arr[size++] = value;
    }

    T operator[](int index) const {
        if (index >= 0 && index < size) {
            return arr[index];
        }
        throw runtime_error("Index out of bounds");
    }

    int getSize() const {
        return size;
    }
};
// 优先级表
unordered_map<char, int> h{ {'+', 1}, {'-', 1}, {'*', 2}, {'/', 2} };

// 自定义栈类型
MyStack<int> num;
MyStack<char> op;

void eval() {  // 求值
    int a = num.top();  // 第二个操作数
    num.pop();

    int b = num.top();  // 第一个操作数
    num.pop();

    char p = op.top();  // 运算符
    op.pop();

    int r = 0;  // 结果 

    // 计算结果
    if (p == '+') r = b + a;
    if (p == '-') r = b - a;
    if (p == '*') r = b * a;
    if (p == '/') r = b / a;

    num.push(r);  // 结果入栈
}

int main() {
    string s;  // 读入表达式
    cin >> s;

    for (int i = 0; i < s.size(); i++) {
        if(s[i]=='#'||s[i]==' '){
            continue;
        }
        //string不考虑结束与休止,因此跳过
        if (isdigit(s[i])) {  // 数字入栈
            int x = 0, j = i;  // 计算数字
            while (j < s.size() && isdigit(s[j])) {
                x = x * 10 + s[j] - '0';
                j++;
            }
            num.push(x);  // 数字入栈
            i = j - 1;
        }
        else if (s[i] == '(') {  // 左括号入栈
            op.push(s[i]);
        }
        else if (s[i] == ')') {  // 右括号
            while (op.top() != '(')  // 一直计算到左括号
                eval();
            op.pop();  // 左括号出栈
        }
        else {  // 运算符
            while (op.getSize() && h[op.top()] >= h[s[i]])  // 优先级低，先计算
                eval();
            op.push(s[i]);  // 操作符入栈
        }
    }
    while (op.getSize()) eval();  // 剩余的进行计算
    cout << num.top() << endl;  // 输出结果
    return 0;
}
