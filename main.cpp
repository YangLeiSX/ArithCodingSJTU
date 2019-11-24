#include "arithCoding.h"
#include <time.h>

int main(int argc, char** argv){
    std::string srcFile = "target.txt";
    std::string dstFile = "target.txt.cal";
    arithCoding code1(srcFile);
    arithCoding code2(dstFile);

    std::cout << code1.getSize() << "\n";
    code1.zip(dstFile);
    code2.unzip("unzip_"+srcFile);
    return 0;
}