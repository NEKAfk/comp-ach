#ifndef HALF_H
#define HALF_H

#include<iostream>

class Half {
    int s;
    int exp;
    int64_t mant;
public:
    Half(uint64_t);
    void mul(const Half&);
    void div(const Half&);
    void add(const Half&);
    void sub(const Half&);
    void rearrange();
    void print();
};
#endif