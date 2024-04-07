#ifndef FLOAT_H
#define FLOAT_H

#include<iostream>

class Float {
    int s;
    int exp;
    int64_t mant;
public:
    Float(uint64_t);
    void mul(const Float&);
    void div(const Float&);
    void add(const Float&);
    void sub(const Float&);
    void rearrange();
    void print();
};
#endif