#include "arithCoding.h"

arithCoding::arithCoding(){
    std::cout << "Please assign an file name\n";
    clear();
    exit(0);
}
arithCoding::arithCoding(std::string filename){
    srcFileName = filename;
    #ifdef DEBUG
    std::cout << "START" << std::endl;
    #endif
    clear();
}
arithCoding::~arithCoding(){
    #ifdef DEBUG
    std::cout << "FINISH" << std::endl;
    #endif
}
void arithCoding::clear(){
    dataBuff.clear();
    beforeSize = afterSize = 0;
    for(int i = 0; i < 16; i++){
        freq[i] = 0;
        Prob[i] = 0;
        sumProb[i] = 0;
    }
}
void arithCoding::zip(std::string fileName){
    // 准备文件
    std::fstream inFile(srcFileName.c_str(), std::ios::in | std::ios::binary);
    std::fstream outFile(fileName.c_str(), std::ios::out | std::ios::binary);
    if(!inFile.is_open() || !outFile.is_open()){
        std::cerr << "[ERROR] fail to open files!\n";
        exit(0);
    }

    // 准备数组
    clear();
    getFreq(srcFileName);
    writeTable(outFile);
    Normal();
    // 变量初始化
    char inChar;
    char left,right;
    double startPos = 0.0, rangewidth = 1.0;
    int leng = 0;
    // 开始循环
    while(inFile.get(inChar)){
        left = (inChar >> 4) & 0x0f;
        right = (inChar) & 0x0f;
        #ifdef DEBUG
        std::cout << "left:" << (int)left << "\t";
        std::cout << "right:" << (int)right << "\n";
        #endif
        startPos = startPos + rangewidth * sumProb[left];
        rangewidth = rangewidth * Prob[left];
        #ifdef DEBUG
        std::cout << "startPos :" << startPos<< "\t rangeWidth is" <<  rangewidth << "\n";
        #endif
        if(startPos >= 1.0){
            carryBit();
            startPos -= 1.0;
        }
        for(leng = 0;rangewidth != 1.0;rangewidth*=2,leng+=1);
        while(leng --){
            startPos *= 2.0;
            if(startPos >= 1.0){
                dataBuff.push_back(1);
                startPos -= 1.0;
            }else
                dataBuff.push_back(0);
        }
        startPos = startPos + rangewidth * sumProb[right];
        rangewidth = rangewidth * Prob[right];
        #ifdef DEBUG
        std::cout << "startPos :" << startPos<< "\t rangeWidth is" <<  rangewidth << "\n";
        #endif
        if(startPos >= 1.0){
            carryBit();
            startPos -= 1.0;
        }
        for(leng = 0;rangewidth != 1.0;rangewidth*=2,leng+=1);
        while(leng --){
            startPos *= 2.0;
            if(startPos >= 1.0){
                dataBuff.push_back(1);
                startPos -= 1.0;
            }else
                dataBuff.push_back(0);
        }
    }
    if(startPos >= 0.5)
        carryBit();
    while(dataBuff.size() % 8 != 0)
        dataBuff.push_back(0);
    afterSize = 16 * 2 + dataBuff.size() / 8;
    #ifdef DEBUG
    std::cout << "size is " << afterSize << "/" << beforeSize << "\n"; 
    #endif
    #ifdef DEBUG
    std::cout << "BUFFER IS:\n";
    std::list<bool>::iterator itr = dataBuff.begin();
    while(itr++ != dataBuff.end())
        std::cout << *itr;
    std::cout << "\n";
    #endif
    writeData(outFile);
    
}
void arithCoding::unzip(std::string fileName){
    // 准备文件
    std::fstream inFile(srcFileName.c_str(), std::ios::in | std::ios::binary);
    std::fstream outFile(fileName.c_str(), std::ios::out | std::ios::binary);
    if(!inFile.is_open() || !outFile.is_open()){
        std::cerr << "[ERROR] fail to open files!\n";
        exit(0);
    }
    // 准备数组
    clear();
    readTable(inFile);
    readData(inFile);
    Normal();
    // 变量初始化
    int counter = 0;
    char outChar;
    char left,right;
    bool aBit;
    double rangeWidth = 0.5;
    double startPos = 0.0;
    int leng;
    double tmpProb;
    for(int i =0; i < 8; i++,rangeWidth/=2.0){
        aBit = dataBuff.front();
        dataBuff.pop_front();
        if(aBit)
            startPos += rangeWidth;
    }
    // 开始循环
    while(counter < beforeSize){
        #ifdef DEBUG
        std::cout << "startPos = " << startPos << "\n";
        #endif
        findChar(startPos, left);
        startPos = startPos - sumProb[left];
        startPos = startPos / Prob[left];
        
        tmpProb = Prob[left];
        leng = 0;
        while(tmpProb < 1.0){
            tmpProb *= 2.0;
            leng++;
        }
        rangeWidth = pow(2.0, leng-9);
        while(leng-- && !dataBuff.empty()){
            aBit = dataBuff.front();
            dataBuff.pop_front();
            if(aBit)
                startPos += rangeWidth;
            rangeWidth /= 2.0;
        }
        #ifdef DEBUG
        std::cout << "startPos = " << startPos << "\n";
        #endif
        findChar(startPos, right);
        startPos = startPos - sumProb[right];
        startPos = startPos / Prob[right];
        
        tmpProb = Prob[right];
        leng = 0;
        while(tmpProb < 1.0){
            tmpProb *= 2.0;
            leng++;
        }
        rangeWidth = pow(2.0, leng-9);
        while(leng-- && !dataBuff.empty()){
            aBit = dataBuff.front();
            dataBuff.pop_front();
            if(aBit)
                startPos += rangeWidth;
            rangeWidth /= 2.0;
        }
        #ifdef DEBUG
        std::cout << "left:" << (int)left << "\t";
        std::cout << "right:" << (int)right << "\n";
        #endif
        outChar = left*16+right;
        outFile.write(&outChar, sizeof(char));
        counter ++;
    }

}
int arithCoding::getSize(){
    zipProc();
    #ifdef DEBUG
    std::cout << "size is " << afterSize << "/" << beforeSize << "\n";
    #endif
    return afterSize;
}
void arithCoding::zipProc(){
    // 准备文件
    std::fstream inFile(srcFileName.c_str(), std::ios::in | std::ios::binary);
    if(!inFile.is_open())
        std::cerr << "[ERROR] fail to open files!\n";
    // 准备数组
    getFreq(srcFileName);
    Normal();
    // 变量初始化
    char inChar;
    char left,right;
    double startPos = 0.0, rangewidth = 1.0;
    int leng = 0;
    // 开始循环
    while(inFile.get(inChar)){
        left = (inChar >> 4) & 0x0f;
        right = (inChar) & 0x0f;
        #ifdef DEBUG
        std::cout << "left:" << (int)left << "\t";
        std::cout << "right:" << (int)right << "\n";
        #endif
        startPos = startPos + rangewidth * sumProb[left];
        rangewidth = rangewidth * Prob[left];
        #ifdef DEBUG
        std::cout << "startPos :" << startPos<< "\t rangeWidth is" <<  rangewidth << "\n";
        #endif
        if(startPos >= 1.0){
            carryBit();
            startPos -= 1.0;
        }
        for(leng = 0;rangewidth != 1.0;rangewidth*=2,leng+=1);
        while(leng --){
            startPos *= 2.0;
            if(startPos >= 1.0){
                dataBuff.push_back(1);
                startPos -= 1.0;
            }else
                dataBuff.push_back(0);
        }
        startPos = startPos + rangewidth * sumProb[right];
        rangewidth = rangewidth * Prob[right];
        #ifdef DEBUG
        std::cout << "startPos :" << startPos<< "\t rangeWidth is" <<  rangewidth << "\n";
        #endif
        if(startPos >= 1.0){
            carryBit();
            startPos -= 1.0;
        }
        for(leng = 0;rangewidth != 1.0;rangewidth*=2,leng+=1);
        while(leng --){
            startPos *= 2.0;
            if(startPos >= 1.0){
                dataBuff.push_back(1);
                startPos -= 1.0;
            }else
                dataBuff.push_back(0);
        }
    }
    if(startPos >= 0.5)
        carryBit();
    while(dataBuff.size() % 8 != 0)
        dataBuff.push_back(0);
    afterSize = 16 * 2 + dataBuff.size() / 8;
    #ifdef DEBUG
    std::cout << "size is " << afterSize << "/" << beforeSize << "\n"; 
    #endif
    #ifdef DEBUG
    std::cout << "BUFFER IS:\n";
    std::list<bool>::iterator itr = dataBuff.begin();
    while(itr++ != dataBuff.end())
        std::cout << *itr;
    std::cout << "\n";
    #endif
}
// 压缩及其辅助函数
void arithCoding::getFreq(std::string fileName){
    std::fstream inFile(fileName.c_str(), std::ios::in | std::ios::binary);
    char inChar;
    char left,right;
    while(inFile.get(inChar)){
        beforeSize += 1;
        left = (inChar>>4) & 0x0f;
        right = inChar & 0x0f;
        
        freq[left] += 1;
        freq[right] += 1;
    }
    #ifdef DBEUG
    std::cout << "the frequence is:\n";
    for(int i = 0;i < 16;i++){
        std::cout << freq[i] << "\t";
    }
    std::cout << "\n";
    #endif
}
void arithCoding::writeTable(std::fstream& outFile){
    char outChar;
    for(int i = 0; i < 16;i++){
        outChar = freq[i] / 256;
        outFile.write(&outChar, sizeof(char));
        outChar = freq[i] % 256;
        outFile.write(&outChar, sizeof(char));
    }  
}
void arithCoding::writeData(std::fstream& outFile){
    char outChar;
    bool aBit;
    #ifdef DEBUG
    std::cout << "wrote:\n";
    #endif
    while(!dataBuff.empty()){
        outChar = 0;
        for(int i = 0; i < 8;i++){
            aBit = dataBuff.front();
            if(aBit)
                outChar = (outChar<<1)+1;
            else
                outChar = outChar<<1;
            dataBuff.pop_front();
        }
        #ifdef DEBUG
        std::cout << ((short)outChar & 0x00ff) << " ";
        #endif
        outFile.write(&outChar, sizeof(char));
    }
    #ifdef DEBUG
    std::cout << "\n";
    #endif
}
// 解压缩及其辅助函数
void arithCoding::readTable(std::fstream& inFile){
    short num;
    char inChar = 0;
    int sum = 0;
    for(int i= 0;i < 16;i++){
        inFile.get(inChar);
        num = ((short)inChar&0x00ff);
        inFile.get(inChar);
        num = (num << 8) + ((short)inChar&0xff);
        // inFile.read((char*)&num, sizeof(short));
        freq[i] = num ;
    }
    for(int i = 0;i < 16;i++)
        sum += freq[i];
    beforeSize = sum/2;
    #ifdef DEBUG
    for(int i = 0;i < 16;i++){
        std::cout << freq[i] << "\t";
    }
    std::cout << "\n";
    #endif
}
void arithCoding::readData(std::fstream& inFile){
    dataBuff.clear();
    char inChar;
    bool inBit;
    #ifdef DEBUG
    std::cout << "read:\n";
    #endif
    while(inFile.get(inChar)){ 
        #ifdef DEBUG
        std::cout << ((short)inChar & 0x00ff) << " ";
        #endif
        for(int i = 0;i < 8;i++){
            inBit = (inChar << i)& 0x80;
            dataBuff.push_back(inBit);
        }
    }
    #ifdef DEBUG
    std::cout << "\n";
    #endif
    #ifdef DEBUG
    std::cout << "BUFFER IS:\n";
    std::list<bool>::iterator itr = dataBuff.begin();
    while(itr++ != dataBuff.end())
        std::cout << *itr;
    std::cout << "\n";
    #endif
}
void arithCoding::findChar(double& startPos, char& theChar){
    for(int i = 0; i < 15;i++){
        if(startPos < sumProb[i+1]){
            theChar = i;
            return ;
        }
    }
    theChar = 15;
}
// 辅助函数
void arithCoding::Normal(){
    double tmpProb[16];
    double sum = 0.0;
    double delta;
    int low,high,lowPos,highPos;
    int i = 0;
    // 计算初始频率
    for(i = 0; i <16; i++){
        tmpProb[i] = freq[i] / 2.0 / (double)beforeSize;
    }
    // 第一次正则化
    for(i = 0; i< 16; i++){
        for(int j = 1;;j++){
            if(tmpProb[i] > powf(2.0, (float)-j)){
                tmpProb[i] = powf(2.0, (float)-j);
                break;
            }
        }
    }
    // 循环正则化
    while(true){
        #ifdef DEBUG
        for(i = 0; i < 16;i++){
            std::cout << tmpProb[i] << " ";
        }
        std::cout << "\n";
        #endif
        sum = 0.0;
        for(int j = 0;j <16;j++)
            sum += tmpProb[j];
        #ifdef DEBUG
        std::cout << "sum is:" << sum << std::endl;
        #endif
        if(sum > 1.0){
            delta = sum - 1.0;
            high = 0;
            highPos = 0;
            for(int j = 0;j <16;j++){
                if(tmpProb[j] == 0)
                    continue;
                if(tmpProb[j] > high && tmpProb[j] >= 2*delta){
                    high = tmpProb[j];
                    highPos = j;
                }
            }
            tmpProb[highPos] /= 2.0;
        }else if(sum < 1.0){
            delta = 1.0 - sum;
            low = 1;
            lowPos = 0;
            for(int j = 0; j <16 ;j++){
                if(tmpProb[j] == 0)
                    continue;
                if(tmpProb[j] < low && tmpProb[j] <= delta){
                    low = tmpProb[j];
                    lowPos = j;
                }
            }
            tmpProb[lowPos] *= 2.0;
        }else
            break;
    }
    sumProb[0] = 0.0;
    for(i = 0; i <16; i++)
        Prob[i] = tmpProb[i];
    for(i = 1;i < 16;i++)
        sumProb[i] = Prob[i-1]+sumProb[i-1];
    #ifdef DEBUG
    for(i = 0; i < 16;i++){
        std::cout << Prob[i] << "\t";
    }
    std::cout << "\n";
    for(i = 0; i < 16;i++){
        std::cout << sumProb[i] << "\t";
    }
    std::cout << "\n";
    #endif
}
void arithCoding::carryBit(){
    std::list<bool> tmp;
    bool aBit;
    while(1 == (aBit = dataBuff.back())){
        dataBuff.pop_back();
        tmp.push_front(1);
    }
    dataBuff.pop_back();
    dataBuff.push_back(1);
    while(!tmp.empty()){
        tmp.pop_front();
        dataBuff.push_back(0);
    }
}