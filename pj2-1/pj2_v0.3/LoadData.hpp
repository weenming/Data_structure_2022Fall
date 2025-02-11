#include <io.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "Permission.hpp"
#include "Tree.hpp"
using namespace std;

/**
 * Load user tree and file tree from the structure of file directory
 * ref: https://www.cnblogs.com/ranjiewen/p/5960976.html
 */
void LoadTree(string cur_path, UserTree* tree, int parent_id) {
    // Recursively visit all directories in the current path
    _finddata_t FileInfo;

    long Handle = _findfirst((cur_path + "/*").c_str(), &FileInfo);

    int next = 0;
    // visit all sub directory
    while (Handle != -1L && (FileInfo.attrib & _A_SUBDIR) && next == 0) {
        // _A_SUBDIR: 0x10, seems the last few bits are used to determine the
        // type of fileinfo.attrib
        string empty = "";
        if (strcmp(FileInfo.name, ".") &&
            strcmp(FileInfo.name,
                   "..")) {  // exclude first two dir: cur rand parent
            string usr_name = FileInfo.name;
            int new_id = tree->AddItem(parent_id,
                                       usr_name);  // must treat as a string
            LoadTree(cur_path + "/" + FileInfo.name, tree, new_id);
        }
        next = _findnext(Handle, &FileInfo);
    }
    _findclose(Handle);
    return;
}

void LoadTree(string cur_path, FileTree* tree, int parent_id) {
    // Recursively visit all directories in the current path
    _finddata_t FileInfo;

    long Handle = _findfirst((cur_path + "/*").c_str(), &FileInfo);

    int next = 0;
    // visit all sub directory
    while (Handle != -1L && (FileInfo.attrib & _A_SUBDIR) && next == 0) {
        // _A_SUBDIR: 0x10, seems the last few bits are used to determine the
        // type of fileinfo.attrib
        if (strcmp(FileInfo.name, ".") &&
            strcmp(FileInfo.name,
                   "..")) {  // exclude first two dir: cur rand parent
            int new_id =
                tree->AddItem(parent_id, cur_path + "/" + FileInfo.name);
            LoadTree(cur_path + "/" + FileInfo.name, tree, new_id);
        }
        next = _findnext(Handle, &FileInfo);
    }
    _findclose(Handle);
    return;
}

void LoadCsv(Permission* pms, UserTree* usr_tree, FileTree* file_tree,
             string filename) {
    char separator = ',';
    string row, item;
    ifstream in(filename);
    // 要去掉标题行，但是文件里并没有标题行，还是会导致第一行的开头有些奇怪的字符。
    // 估计是文件开头的什么信息...
    // 坑爹的是这些奇怪字符在debugger里面不显示...
    // 所以我的解决方法就是手动加一行表头再读掉它
    getline(in, row);
    while (getline(in, row)) {
        vector<string> R;
        stringstream ss(row);
        while (getline(ss, item, separator)) R.push_back(item);
        // Add to Permission type
        // one line in the csv: usr_name, file_name, permission
        // R[3][0]: first char in the string, should be char for the number of
        // permission level. FILE and USER corresponds to a special line
        if (R[0] == "USER") {
            continue;
        }
        pms->AddPms(usr_tree->GetId(R[0]), file_tree->GetId(R[1]),
                    R.at(2).at(0) - '0');
    }
    in.close();
    return;
}
