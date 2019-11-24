#ifndef INTERFACE_H
#define INTERFACE_H
#include <string>

class Coding{
public:
    virtual void zip(std::string fileName)=0;
    virtual void unzip(std::string fileName)=0;
    virtual int getSize()=0;
};
#endif //INTERFACE_H
