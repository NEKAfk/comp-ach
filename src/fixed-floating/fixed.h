#ifndef FIXED_H
#define FIXED_H

#include<iostream>
#include<vector>
#include<algorithm>

class Fixed {
    int A;
    int B;
    uint64_t n;
public:
    Fixed(uint64_t, int, int);
    void print();
    void add(const Fixed&);
    void sub(const Fixed&);
    void mul(const Fixed&);
    void div(const Fixed&);
};
#endif