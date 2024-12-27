#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <queue>
#include <vector>
#include <bitset>
#include <sstream>
using namespace std;

// 定义Huffman树节点
struct HuffmanNode {
    char ch;               // 存储字符
    int frequency;         // 字符频率
    HuffmanNode* left;     // 左子节点
    HuffmanNode* right;    // 右子节点

    HuffmanNode(char character, int freq) : ch(character), frequency(freq), left(nullptr), right(nullptr) {}

    // 用于优先队列的比较函数，按频率从小到大排序
    struct Compare {
        bool operator()(HuffmanNode* left, HuffmanNode* right) {
            return left->frequency > right->frequency;
        }
    };
};

// 递归地为每个字符生成Huffman编码
void generate(HuffmanNode* root, const string& code, unordered_map<char, string>& huffmanCodes) {
    if (root == nullptr) {
        return;
    }
    if (root->left == nullptr && root->right == nullptr) {
        huffmanCodes[root->ch] = code;
    }
    generate(root->left, code + "0", huffmanCodes);
    generate(root->right, code + "1", huffmanCodes);
}

// 读取文件内容并统计字符频率
unordered_map<char, int> calculate(const string& filename) {
    ifstream file(filename);
    unordered_map<char, int> freqMap;
    char ch;

    while (file.get(ch)) {
        freqMap[ch]++;
    }

    return freqMap;
}

// 构建Huffman树
HuffmanNode* build(const unordered_map<char, int>& freqMap) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, HuffmanNode::Compare> minHeap;

    // 将每个字符及其频率插入到最小堆中
    for (const auto& entry : freqMap) {
        minHeap.push(new HuffmanNode(entry.first, entry.second));
    }

    // 构建Huffman树
    while (minHeap.size() > 1) {
        HuffmanNode* left = minHeap.top();
        minHeap.pop();
        HuffmanNode* right = minHeap.top();
        minHeap.pop();

        HuffmanNode* newNode = new HuffmanNode('\0', left->frequency + right->frequency);
        newNode->left = left;
        newNode->right = right;

        minHeap.push(newNode);
    }

    return minHeap.top(); // 返回根节点
}

// 将Huffman编码表写入文件
void writeHuffmanCodes(const unordered_map<char, string>& huffmanCodes, const string& filename) {
    ofstream file(filename);
    for (const auto& entry : huffmanCodes) {
        file << entry.first << ": " << entry.second << endl;
    }
    file.close();
}

// 将文章编码为二进制字符串
string encodeText(const string& text, const unordered_map<char, string>& huffmanCodes) {
    string encodedText = "";
    for (char ch : text) {
        encodedText += huffmanCodes.at(ch);
    }
    return encodedText;
}

// 将二进制字符串写入文件
void writeb(const string& binaryData, const string& filename) {
    ofstream file(filename, ios::binary);

    // 写入二进制数据
    size_t dataSize = binaryData.size();
    for (size_t i = 0; i < dataSize; i += 8) {
        bitset<8> byte(binaryData.substr(i, 8));
        char byteChar = static_cast<char>(byte.to_ulong());
        file.write(&byteChar, sizeof(byteChar));
    }

    file.close();
}

// 解码函数，通过Huffman树解码二进制文件
string decodeText(const string& binaryData, HuffmanNode* root) {
    string decodedText = "";
    HuffmanNode* currentNode = root;
    for (char bit : binaryData) {
        if (bit == '0') {
            currentNode = currentNode->left;
        } else {
            currentNode = currentNode->right;
        }

        // 如果到达叶子节点，则记录字符并返回根节点
        if (currentNode->left == nullptr && currentNode->right == nullptr) {
            decodedText += currentNode->ch;
            currentNode = root;
        }
    }
    return decodedText;
}

// 从二进制文件读取数据
string readB(const string& filename) {
    ifstream file(filename, ios::binary);
    string binaryData = "";
    char byte;

    while (file.read(&byte, sizeof(byte))) {
        bitset<8> bits(byte);
        binaryData += bits.to_string();
    }

    return binaryData;
}

int main() {
    // 1. 读取文件并统计字符频率
    string filename = "source.txt";  // 输入文件名
    unordered_map<char, int> freqMap = calculate(filename);

    // 2. 构建Huffman树
    HuffmanNode* root = build(freqMap);

    // 3. 生成Huffman编码
    unordered_map<char, string> huffmanCodes;
    generate(root, "", huffmanCodes);

    // 4. 将Huffman编码表写入文件
    writeHuffmanCodes(huffmanCodes, "Huffman.txt");

    // 5. 读取文件内容并进行Huffman编码
    ifstream file(filename);
    string text((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    string encodedText = encodeText(text, huffmanCodes);

    // 6. 将编码结果（以二进制形式）写入code.dat文件
    writeb(encodedText, "code.dat");

    // 7. 读取二进制数据并解码
    string binaryData = readB("code.dat");
    string decodedText = decodeText(binaryData, root);

    // 8. 将解码后的文本保存到recode.txt
    ofstream recodeFile("recode.txt");
    recodeFile << decodedText;
    recodeFile.close();

    cout << "Huffman encoding and decoding process completed!" << endl;
    return 0;
}
