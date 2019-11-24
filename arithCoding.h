#ifndef ARITHCODE_H
#define ARITHCODE_H

#include "interface.h"
#include <list>
#include <iostream>
#include <fstream>
#include <math.h>
#include <cstdlib>
class arithCoding:Coding{
public:
    void zip(std::string fileName);
    void unzip(std::string fileName);
    int getSize();

    arithCoding();
    arithCoding(std::string filename);
    ~arithCoding();
private:
    std::string srcFileName;
    // 二进制数据
    std::list<bool> dataBuff;
    // 频数
    short freq[16];
    // 频率
    double Prob[16];
    // 累计频率
    double sumProb[16];
    // 文件规模
    int beforeSize;
    int afterSize;

    // 压缩及其辅助函数
    void zipProc();
    void getFreq(std::string fileName);
    void writeTable(std::fstream& outFile);
    void writeData(std::fstream& outFile);
    // 解压缩及其辅助函数
    void readTable(std::fstream& inFile);
    void readData(std::fstream& inFile);
    void findChar(double& startPos, char& theChar);
    // 辅助函数
    void clear();
    void Normal();
    void carryBit();
};

#endif