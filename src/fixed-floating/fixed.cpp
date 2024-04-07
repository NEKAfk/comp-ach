#include"fixed.h"

Fixed::Fixed(uint64_t in, int L, int R) {
    n = in;
    A = L;
    B = R;
}

void Fixed::add(const Fixed& x) {
    n = (n + x.n) & ((1LL << (A + B)) - 1);
}

void Fixed::sub(const Fixed& x) {
    n = (n - x.n) & ((1LL << (A + B)) - 1);
}

void Fixed::mul(const Fixed& x) {
    int s = 0;
    Fixed xCopy = x;
    if (n & (1LL << (A + B - 1))) {
        s ^= 1;
        n = ((~n) + 1) & ((1LL << (A + B)) - 1);
    }
    if (xCopy.n & (1LL << (A + B - 1))) {
        s ^= 1;
        xCopy.n = ((~xCopy.n) + 1) & ((1LL << (A + B)) - 1);
    }
    n = n * xCopy.n;
    if (!s && n & ((1LL << B) - 1)) {
        n = ((n >> B) + 1);
    } else {
        n = (n >> B);
    }
    if (s) {
        n = ((~n) + 1);
    }
    n &= ((1LL << (A + B)) - 1);
}

void Fixed::div(const Fixed& x) {
    int s = 0;
    Fixed xCopy = x;
    if (n & (1LL << (A + B - 1))) {
        s ^= 1;
        n = ((~n) + 1)&((1LL << (A + B)) - 1);
    }
    if (xCopy.n & (1LL << (A + B - 1))) {
        s ^= 1;
        xCopy.n = ((~xCopy.n) + 1) & ((1LL << (A + B)) - 1);
    }
    uint64_t tmpN = n / xCopy.n;
    uint64_t ost = (n % xCopy.n) << 1;
    for (int i = 0; i < B; i++) {
        tmpN = (tmpN << 1) + ost / xCopy.n;
        ost = (ost % xCopy.n) << 1;
    }
    n = tmpN;
    if (!s && ost != 0) {
        n++;
    }
    if (s) {
        n = ((~n) + 1);
    }
    n &= (1LL << (A + B)) - 1;
}

void Fixed::print() {
    bool isPos = true;
    uint64_t tmpN = n;
    if ((tmpN >> (A + B - 1)) & 1) {
        tmpN = (~tmpN + 1) & ((1LL << (A+B)) - 1);
        isPos = false;
    }
    std::vector<int> digits;
    for (int i = 0; i < 4; i++) {
        digits.push_back(tmpN / (1 << B));
        tmpN = (tmpN % (1 << B)) * 10;
    }
    if (isPos && tmpN != 0) {
        int k = 1;
        for (int i = digits.size() - 1; i >= 1; i--) {
            int s = digits[i] + k;
            k = s / 10;
            digits[i] = s % 10;
        }
        digits[0] += k;
    }
    if (!isPos && std::count(digits.begin(), digits.end(), 0) != digits.size()) {
        std::cout << '-';
    }
    for (int i = 0; i < digits.size(); i++) {
        if (i == 1) {
            std::cout << '.';
        }
        std::cout << digits[i];
    }
    std::cout << std::endl;
}
