// genealogy.cpp
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <queue>

using namespace std;

// --- Member Class ---
class Member {
public:
    int id;                     // 标识符
    string name;                // 姓名
    string birth_date;          // 出生日期 
    string marital_status;      // 婚否
    string address;             // 地址
    bool is_alive;              // 是否健在
    string death_date;          // 死亡日期 
    int parent_id;              // 父亲的ID (0表示无)
    vector<int> children_ids;   // 孩子的ID列表

    Member() : id(0), parent_id(0), is_alive(true) {}

    // 将Member对象转换为文件
    string to_string_line() const {
        string line = to_string(id) + "|" + name + "|" + birth_date + "|" + marital_status + "|" + address + "|" + 
                      (is_alive ? "1" : "0") + "|" + death_date + "|" + to_string(parent_id) + "|";
        for (auto i = 0; i < children_ids.size(); ++i) {
            line += to_string(children_ids[i]);
            if (i != children_ids.size() - 1)
                line += ";";
        }
        return line;
    }

    // 从文件创建Member对象
    static Member from_string_line(const string& line) {
        Member member;
        stringstream ss(line);
        string item;
        vector<string> tokens;

        while (getline(ss, item, '|')) {
            tokens.push_back(item);
        }

        if (tokens.size() < 8) {
            cerr << "数据格式错误: " << line << endl;
            return member;
        }

        member.id = stoi(tokens[0]);
        member.name = tokens[1];
        member.birth_date = tokens[2];
        member.marital_status = tokens[3];
        member.address = tokens[4];
        member.is_alive = (tokens[5] == "1") ? true : false;
        member.death_date = tokens[6];
        member.parent_id = stoi(tokens[7]);

        if (tokens.size() >= 9 && !tokens[8].empty()) {
            string children_str = tokens[8];
            stringstream cs(children_str);
            string child_id_str;
            while (getline(cs, child_id_str, ';')) {
                member.children_ids.push_back(stoi(child_id_str));
            }
        }

        return member;
    }

    // 打印成员信息
    void print_info(const unordered_map<int, Member>& members_map) const {
        cout << "ID: " << id << endl;
        cout << "姓名: " << name << endl;
        cout << "出生日期: " << birth_date << endl;
        cout << "婚否: " << marital_status << endl;
        cout << "地址: " << address << endl;
        cout << "健在否: " << (is_alive ? "是" : "否") << endl;
        if (!is_alive) {
            cout << "死亡日期: " << death_date << endl;
        }
        // 父亲信息
        if (parent_id != 0) {
            auto it = members_map.find(parent_id);
            if (it != members_map.end()) {
                cout << "父亲信息：" << endl;
                cout << "  姓名: " << it->second.name << endl;
                cout << "  出生日期: " << it->second.birth_date << endl;
            }
        }
        // 孩子信息
        if (!children_ids.empty()) {
            cout << "孩子信息：" << endl;
            for (int child_id : children_ids) {
                auto it = members_map.find(child_id);
                if (it != members_map.end()) {
                    cout << "  姓名: " << it->second.name << ", 出生日期: " << it->second.birth_date << endl;
                }
            }
        }
    }
};

//家谱类
class GenealogyManager {
private:
    unordered_map<int, Member> members_map; // key: id, value: Member
    string file_path;
    int next_id;

public:
    GenealogyManager(const string& path = "genealogy.txt") : file_path(path), next_id(1) {
        load_data();
    }

    // 加载数据从文件
    void load_data() {
        ifstream infile(file_path);
        if (!infile.is_open()) {
            // 文件不存在，初始化为空
            cout << "数据文件不存在。" << endl;
            return;
        }

        string line;
        while (getline(infile, line)) {
            if (line.empty()) continue;
            Member member = Member::from_string_line(line);
            members_map[member.id] = member;
            if (member.id >= next_id) {
                next_id = member.id + 1;
            }
        }

        infile.close();
    }

    // 保存数据到文件
    void save_data() const {
        ofstream outfile(file_path, ios::trunc);
        if (!outfile.is_open()) {
            cerr << "无法打开文件进行写入: " << file_path << endl;
            return;
        }

        for (const auto& pair : members_map) {
            outfile << pair.second.to_string_line() << "\n";
        }

        outfile.close();
    }

    // 添加成员
    void add_member(Member& member) {
        member.id = next_id++;
        members_map[member.id] = member;

        // 更新父亲的孩子列表
        if (member.parent_id != 0) {
            auto it = members_map.find(member.parent_id);
            if (it != members_map.end()) {
                it->second.children_ids.push_back(member.id);
            } else {
                cout << "警告: 父亲ID " << member.parent_id << " 未找到。" << endl;
            }
        }

        save_data();
        cout << "成员 '" << member.name << "' 已添加，ID为 " << member.id << "。\n";
    }

    // 删除成员及其后代
    void delete_member(int member_id) {
        auto it = members_map.find(member_id);
        if (it == members_map.end()) {
            cout << "成员ID " << member_id << " 未找到。\n";
            return;
        }

        // 递归删除所有后代
        for (int child_id : it->second.children_ids) {
            delete_member(child_id);
        }

        // 从父亲的孩子列表中移除
        if (it->second.parent_id != 0) {
            auto parent_it = members_map.find(it->second.parent_id);
            if (parent_it != members_map.end()) {
                parent_it->second.children_ids.erase(
                    remove(parent_it->second.children_ids.begin(), parent_it->second.children_ids.end(), member_id),
                    parent_it->second.children_ids.end()
                );
            }
        }

        // 删除成员
        members_map.erase(it);
        cout << "成员ID " << member_id << " 及其后代已删除。\n";

        save_data();
    }

    // 修改成员信息
    void modify_member(int member_id, const Member& updated_member) {
        auto it = members_map.find(member_id);
        if (it == members_map.end()) {
            cout << "成员ID " << member_id << " 未找到。\n";
            return;
        }

        // 如果父亲ID发生变化，需要更新原父亲和新父亲的孩子列表
        if (updated_member.parent_id != it->second.parent_id) {
            // 从原父亲的孩子列表中移除
            if (it->second.parent_id != 0) {
                auto original_parent_it = members_map.find(it->second.parent_id);
                if (original_parent_it != members_map.end()) {
                    original_parent_it->second.children_ids.erase(
                        remove(original_parent_it->second.children_ids.begin(), original_parent_it->second.children_ids.end(), member_id),
                        original_parent_it->second.children_ids.end()
                    );
                }
            }

            // 添加到新父亲的孩子列表
            if (updated_member.parent_id != 0) {
                auto new_parent_it = members_map.find(updated_member.parent_id);
                if (new_parent_it != members_map.end()) {
                    new_parent_it->second.children_ids.push_back(member_id);
                } else {
                    cout << "警告: 新父亲ID " << updated_member.parent_id << " 未找到。\n";
                }
            }
        }

        // 更新成员信息
        it->second.name = updated_member.name;
        it->second.birth_date = updated_member.birth_date;
        it->second.marital_status = updated_member.marital_status;
        it->second.address = updated_member.address;
        it->second.is_alive = updated_member.is_alive;
        it->second.death_date = updated_member.death_date;
        it->second.parent_id = updated_member.parent_id;
        // children_ids 不在此处更新

        save_data();
        cout << "成员ID " << member_id << " 信息已更新。\n";
    }

    // 查询成员通过姓名（可能有多个同名成员）
    vector<Member> get_members_by_name(const string& name) const {
        vector<Member> result;
        for (const auto& pair : members_map) {
            if (pair.second.name == name) {
                result.push_back(pair.second);
            }
        }
        return result;
    }

    // 查询成员通过出生日期
    vector<Member> get_members_by_birth_date(const string& birth_date) const {
        vector<Member> result;
        for (const auto& pair : members_map) {
            if (pair.second.birth_date == birth_date) {
                result.push_back(pair.second);
            }
        }
        return result;
    }

    // 获取第n代成员
    vector<Member>   get_nth_generation(int n) const {
        vector<Member> generation;
        if (n < 1) return generation;

        // 获取根代（无父亲）
        vector<Member> current_gen;
        for (const auto& pair : members_map) {
            if (pair.second.parent_id == 0) {
                current_gen.push_back(pair.second);
            }
        }

        int current_level = 1;
        while (current_level < n && !current_gen.empty()) {
            vector<Member> next_gen;
            for (const Member& member : current_gen) {
                for (int child_id : member.children_ids) {
                    auto it = members_map.find(child_id);
                    if (it != members_map.end()) {
                        next_gen.push_back(it->second);
                    }
                }
            }
            current_gen = next_gen;
            current_level++;
        }

        if (current_level == n) {
            generation = current_gen;
        }

        return generation;
    }

    // 获取成员的所有祖先
    vector<Member> get_ancestors(int member_id) const {
        vector<Member> ancestors;
        auto it = members_map.find(member_id);
        while (it != members_map.end() && it->second.parent_id != 0) {
            auto parent_it = members_map.find(it->second.parent_id);
            if (parent_it != members_map.end()) {
                ancestors.push_back(parent_it->second);
                it = parent_it;
            } else {
                break;
            }
        }
        return ancestors;
    }

    // 确定两人关系
    string determine_relationship(int id1, int id2) const {
        if (id1 == id2) return "同一个人。";

        // 获取所有祖先
        vector<Member> ancestors1 = get_ancestors(id1);
        vector<Member> ancestors2 = get_ancestors(id2);

        // 寻找共同祖先
        int common_ancestor_id = 0;
        for (const Member& anc1 : ancestors1) {
            for (const Member& anc2 : ancestors2) {
                if (anc1.id == anc2.id) {
                    common_ancestor_id = anc1.id;
                    break;
                }
            }
            if (common_ancestor_id != 0) break;
        }

        if (common_ancestor_id == 0) {
            return "无共同祖先，关系不明确。";
        } else {
            // 简单描述
            auto it = members_map.find(common_ancestor_id);
            if (it != members_map.end()) {
                return "两人有共同的祖先：" + it->second.name + "。具体关系需要进一步分析。";
            } else {
                return "两人有共同的祖先。具体关系需要进一步分析。";
            }
        }
    }

    // 添加孩子
    void add_child(int parent_id, const Member& child) {
        auto it = members_map.find(parent_id);
        if (it == members_map.end()) {
            cout << "父亲ID " << parent_id << " 未找到。\n";
            return;
        }

        Member new_child = child;
        new_child.id = next_id++;
        new_child.parent_id = parent_id;
        members_map[new_child.id] = new_child;
        it->second.children_ids.push_back(new_child.id);

        save_data();
        cout << "孩子 '" << new_child.name << "' 已添加，ID为 " << new_child.id << "。\n";
    }

    // 打印成员信息
    void print_member_info(int member_id) const {
        auto it = members_map.find(member_id);
        if (it == members_map.end()) {
            cout << "成员ID " << member_id << " 未找到。\n";
            return;
        }
        it->second.print_info(members_map);
    }

    // 生成示例数据
    void generate_sample_data() {
        // 注意：避免重复运行此函数以防止重复添加成员
        cout << "正在生成示例数据...\n";

        // 创建祖先
        Member grandfather;
        grandfather.name = "祖父1";
        grandfather.birth_date = "1940-01-01";
        grandfather.marital_status = "已婚";
        grandfather.address = "地址A";
        grandfather.is_alive = false;
        grandfather.death_date = "2000-05-05";
        grandfather.parent_id = 0;
        add_member(grandfather); // ID 1

        Member grandmother;
        grandmother.name = "祖父2";
        grandmother.birth_date = "1942-02-02";
        grandmother.marital_status = "已婚";
        grandmother.address = "地址A";
        grandmother.is_alive = false;
        grandmother.death_date = "2005-06-06";
        grandmother.parent_id = 0;
        add_member(grandmother); // ID 2

        // 创建父母
        Member father;
        father.name = "父亲1";
        father.birth_date = "1965-03-03";
        father.marital_status = "已婚";
        father.address = "地址B";
        father.is_alive = true;
        father.parent_id = grandfather.id;
        add_member(father); // ID 3

        Member mother;
        mother.name = "父亲2";
        mother.birth_date = "1967-04-04";
        mother.marital_status = "已婚";
        mother.address = "地址B";
        mother.is_alive = true;
        mother.parent_id = grandmother.id;
        add_member(mother); // ID 4

        // 创建子女
        for (int i = 1; i <= 10; ++i) {
            Member son;
            son.name = "儿子" + to_string(i);
            son.birth_date = "1990-05-27";
            son.marital_status = "未婚";
            son.address = "地址C";
            son.is_alive = true;
            son.parent_id = father.id;
            add_member(son); // IDs 5-14
        }

        for (int i = 1; i <= 10; ++i) {
            Member daughter;
            daughter.name = "女儿" + to_string(i);
            daughter.birth_date = "1992-06-28" ;
            daughter.marital_status = "未婚";
            daughter.address = "地址C";
            daughter.is_alive = true;
            daughter.parent_id = mother.id;
            add_member(daughter); // IDs 15-24
        }

        // 创建孙辈
        for (int i = 1; i <= 20; ++i) {
            // 每个儿子有2个孩子
            for (int j = 1; j <= 2; ++j) {
                Member grandchild;
                grandchild.name = "孙子" + to_string(i) + to_string(j);
                grandchild.birth_date = "2015-07-28";
                grandchild.marital_status = "未婚";
                grandchild.address = "地址D";
                grandchild.is_alive = true;
                // 假设儿子i的ID是4 + i (根据上面的添加顺序)
                grandchild.parent_id = 4 + i;
                add_member(grandchild); // IDs 25-44
            }
        }

        cout << "示例数据已生成，共 " << members_map.size() << " 个成员。\n";
    }

    // 获取下一个唯一ID（用于确保唯一性）
    int get_next_id() const {
        return next_id;
    }

    void print_family_tree(int root_id = 0, int depth = 0) const {
        // 如果是打印整个家谱树（root_id = 0），则找出所有根节点（没有父亲的成员）
        if (root_id == 0) {
            cout << "家族成员树形图：\n";
            for (const auto& pair : members_map) {
                if (pair.second.parent_id == 0) {
                    print_family_tree(pair.first, 0);
                }
            }
            return;
        }

        // 打印当前成员
        auto it = members_map.find(root_id);
        if (it == members_map.end()) return;

        // 打印缩进
        for (int i = 0; i < depth; i++) {
            cout << "    ";
        }
        
        // 打印成员信息
        cout << "├─ " << it->second.name 
             << " (ID:" << it->second.id 
             << ", 生日:" << it->second.birth_date 
             << ", " << (it->second.is_alive ? "在世" : "已故") 
             << ")\n";

        // 递归打印所有子女
        for (int child_id : it->second.children_ids) {
            print_family_tree(child_id, depth + 1);
        }
    }
};

// 读取整数输入，带错误检查
int read_int(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        string input;
        getline(cin, input);
        try {
            value = stoi(input);
            break;
        } catch (...) {
            cout << "无效输入，请输入一个整数。\n";
        }
    }
    return value;
}

// 选择成员（处理同名情况）
int select_member(const vector<Member>& members) {
    if (members.empty()) {
        return -1;
    } else if (members.size() == 1) {
        return members[0].id;
    } else {
        cout << "找到多个同名成员，请选择：\n";
        for (size_t i = 0; i < members.size(); ++i) {
            cout << i + 1 << ". ID: " << members[i].id << ", 出生日期: " << members[i].birth_date << ", 地址: " << members[i].address << "\n";
        }
        int choice = 0;
        while (true) {
            choice = read_int("选择编号: ");
            if (choice >= 1 && choice <= members.size()) {
                return members[choice - 1].id;
            } else {
                cout << "无效选择，请重新输入。\n";
            }
        }
    }
}

// --- UI Functions ---

// 添加成员
void add_member_ui(GenealogyManager& manager) {
    cout << "\n--- 添加成员 ---\n";
    Member member;
    cout << "姓名: ";
    cin >> member.name;
    cout << "出生日期 (YYYY-MM-DD): ";
    cin >> member.birth_date;
    cout << "婚否: ";
    cin >> member.marital_status;
    cout << "地址: ";
    cin >> member.address;
    cout << "是否健在 (1是/0否): ";
    string alive_input;
    cin >> alive_input;
    member.is_alive = (alive_input == "1") ? true : false;
    if (!member.is_alive) {
        cout << "死亡日期 (YYYY-MM-DD): ";
        cin >> member.death_date;
    } else {
        member.death_date = "";
    }
    cout << "父亲ID (0表示无): ";
    string parent_input;
    cin >> parent_input;
    try {
        member.parent_id = stoi(parent_input);
    } catch (...) {
        member.parent_id = 0;
    }

    manager.add_member(member);
}

// 删除成员
void delete_member_ui(GenealogyManager& manager) {
    cout << "\n--- 删除成员 ---\n";
    cout << "请输入要删除的成员姓名: ";
    string name;
    cin >> name;
    vector<Member> members = manager.get_members_by_name(name);
    if (members.empty()) {
        cout << "未找到该成员。\n";
        return;
    }
    int member_id = select_member(members);
    if (member_id != -1) {
        cout << "确定要删除成员ID " << member_id << " 及其后代吗？ (y/n): ";
        string confirm;
        cin >> confirm;
        if (confirm == "y" || confirm == "Y") {
            manager.delete_member(member_id);
        } else {
            cout << "取消删除。\n";
        }
    }
}

// 修改成员信息
void modify_member_ui(GenealogyManager& manager) {
    cout << "\n--- 修改成员信息 ---\n";
    cout << "请输入要修改的成员姓名: ";
    string name;
    cin >> name;
    vector<Member> members = manager.get_members_by_name(name);
    if (members.empty()) {
        cout << "未找到该成员。\n";
        return;
    }
    int member_id = select_member(members);
    if (member_id == -1) return;

    // 获取当前成员信息
    Member current_member;
    auto it = manager.get_members_by_name(name).begin();
    for (const Member& m : members) {
        if (m.id == member_id) {
            current_member = m;
            break;
        }
    }

    // 输入新的信息，按回车跳过不修改
    cout << "按回车跳过不修改。\n";
    cout << "姓名 [" << current_member.name << "]: ";
    string input;
    cin >> input;
    if (!input.empty()) current_member.name = input;

    cout << "出生日期 [" << current_member.birth_date << "]: ";
    cin >> input;
    if (!input.empty()) current_member.birth_date = input;

    cout << "婚否 [" << current_member.marital_status << "]: ";
    cin >> input;
    if (!input.empty()) current_member.marital_status = input;

    cout << "地址 [" << current_member.address << "]: ";
    cin >> input;
    if (!input.empty()) current_member.address = input;

    cout << "是否健在 (1是/0否) [" << (current_member.is_alive ? "1" : "0") << "]: ";
    cin >> input;
    if (!input.empty()) {
        current_member.is_alive = (input == "1") ? true : false;
    }

    if (!current_member.is_alive) {
        cout << "死亡日期 [" << (current_member.death_date.empty() ? "无" : current_member.death_date) << "]: ";
        cin >> input;
        if (!input.empty()) current_member.death_date = input;
    } else {
        current_member.death_date = "";
    }

    cout << "父亲ID [" << current_member.parent_id << "]: ";
    cin >> input;
    if (!input.empty()) {
        try {
            current_member.parent_id = stoi(input);
        } catch (...) {
            current_member.parent_id = 0;
        }
    }

    manager.modify_member(member_id, current_member);
}

// 按姓名查询成员信息
void query_by_name_ui(const GenealogyManager& manager) {
    cout << "\n--- 按姓名查询成员信息 ---\n";
    cout << "请输入要查询的成员姓名: ";
    string name;
    cin >> name;
    vector<Member> members = manager.get_members_by_name(name);
    if (members.empty()) {
        cout << "未找到该成员。\n";
        return;
    }
    for (const Member& member : members) {
        cout << "--------------------------\n";
        const_cast<GenealogyManager&>(manager).print_member_info(member.id);
    }
    cout << "--------------------------\n";
}

// 按出生日期查询成员名单
void query_by_birth_date_ui(const GenealogyManager& manager) {
    cout << "\n--- 按出生日期查询成员名单 ---\n";
    cout << "请输入出生日期 (YYYY-MM-DD): ";
    string birth_date;
    cin >> birth_date;
    vector<Member> members = manager.get_members_by_birth_date(birth_date);
    if (members.empty()) {
        cout << "未找到符合出生日期的成员。\n";
        return;
    }
    cout << "找到以下成员:\n";
    for (const Member& member : members) {
        cout << "ID: " << member.id << ", 姓名: " << member.name << ", 地址: " << member.address << "\n";
    }
}

// 确定两人关系
void determine_relationship_ui(const GenealogyManager& manager) {
    cout << "\n--- 确定两人关系 ---\n";
    cout << "请输入第一个成员姓名: ";
    string name1;
    cin >> name1;
    cout << "请输入第二个成员姓名: ";
    string name2;
    cin >> name2;

    vector<Member> members1 = manager.get_members_by_name(name1);
    vector<Member> members2 = manager.get_members_by_name(name2);

    if (members1.empty() || members2.empty()) {
        cout << "其中一位成员未找到。\n";
        return;
    }

    int id1 = select_member(members1);
    int id2 = select_member(members2);

    if (id1 == -1 || id2 == -1) return;

    string relation = manager.determine_relationship(id1, id2);
    cout << "关系: " << relation << "\n";
}

// 添加孩子
void add_child_ui(GenealogyManager& manager) {
    cout << "\n--- 添加孩子 ---\n";
    cout << "请输入父亲姓名: ";
    string parent_name;
    cin >> parent_name;
    vector<Member> parents = manager.get_members_by_name(parent_name);
    if (parents.empty()) {
        cout << "未找到该父亲。\n";
        return;
    }
    int parent_id = select_member(parents);
    if (parent_id == -1) return;

    // 输入孩子信息
    Member child;
    cout << "孩子姓名: ";
    cin >> child.name;
    cout << "孩子出生日期 (YYYY-MM-DD): ";
    cin >> child.birth_date;
    cout << "孩子婚否: ";
    cin >> child.marital_status;
    cout << "孩子地址: ";
    cin >> child.address;
    cout << "孩子是否健在 (1是/0否): ";
    string alive_input;
    cin >> alive_input;
    child.is_alive = (alive_input == "1") ? true : false;
    if (!child.is_alive) {
        cout << "孩子死亡日期 (YYYY-MM-DD): ";
        cin >> child.death_date;
    } else {
        child.death_date = "";
    }

    manager.add_child(parent_id, child);
}

// 显示第n代所有人
void display_nth_generation_ui(const GenealogyManager& manager) {
    cout << "\n--- 显示第n代所有人 ---\n";
    cout<<("请输入要显示的代数 n: ");
    int n;
    cin >> n;
    if (n < 1) {
        cout << "代数必须大于等于1。\n";
        return;
    }
    vector<Member> generation = manager.get_nth_generation(n);
    if (generation.empty()) {
        cout << "第 " << n << " 代无成员。\n";
    } else {
        cout << "第 " << n << " 代成员:\n";
        for (const Member& member : generation) {
            cout << "ID: " << member.id << ", 姓名: " << member.name << ", 出生日期: " << member.birth_date << ", 地址: " << member.address << "\n";
        }
    }
}

// 生成示例数据
void generate_sample_data_ui(GenealogyManager& manager) {
    cout << "\n--- 生成示例数据 ---\n";
    manager.generate_sample_data();
}

// 打印家族树
void print_family_tree_ui(const GenealogyManager& manager) {
    cout << "\n--- 打印家族树 ---\n";
    manager.print_family_tree();
}

// 主菜单
void display_menu() {
    cout << "\n=== 家谱管理系统 ===\n";
    cout << "1. 添加成员\n";
    cout << "2. 删除成员\n";
    cout << "3. 修改成员信息\n";
    cout << "4. 按姓名查询成员信息\n";
    cout << "5. 按出生日期查询成员名单\n";
    cout << "6. 确定两人关系\n";
    cout << "7. 添加孩子\n";
    cout << "8. 显示第n代所有人\n";
    cout << "9. 生成示例数据\n";
    cout << "a. 打印家族树\n";
    cout << "0. 退出\n";
    cout << "请选择功能 (0-10): ";
}


int main() {
    GenealogyManager manager;
    while (true) {
        display_menu();
        string choice;
        cin >> choice;
        if (choice.empty()) continue;
        switch (choice[0]) {
            case '1':
                add_member_ui(manager);
                break;
            case '2':
                delete_member_ui(manager);
                break;
            case '3':
                modify_member_ui(manager);
                break;
            case '4':
                query_by_name_ui(manager);
                break;
            case '5':
                query_by_birth_date_ui(manager);
                break;
            case '6':
                determine_relationship_ui(manager);
                break;
            case '7':
                add_child_ui(manager);
                break;
            case '8':
                display_nth_generation_ui(manager);
                break;
            case '9':
                generate_sample_data_ui(manager);
                break;
            case 'a':
                print_family_tree_ui(manager);
                break;
            case '0':
                cout << "退出系统。\n";
                return 0;
            default:
                cout << "无效选择，请重新输入。\n";
        }
    }
}
