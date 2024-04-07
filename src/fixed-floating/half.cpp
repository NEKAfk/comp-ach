#include"half.h"

Half::Half(uint64_t n) {
    s = (n >> 15) & 1;
    exp = (31 & (n >> 10)) - 15;
    mant = n & ((1 << 10) - 1);
    if (exp == -15 && mant == 0) {
        exp = -14;
        return;
    } else if (exp == 16 && mant == 0) {
        mant = 1 << 10;
        return;
    } else if (exp == 16 && mant != 0) {
        mant = 1 + (1 << 10);
        return;
    }
    if (exp != -15) {
        mant += 1 << 10;
    } else {
        exp = -14;
    }
}

void Half::mul(const Half& x) {
    if (((mant == 0 && x.exp== 16 && x.mant == 1<<10)
                || (x.mant == 0 && exp ==16 && mant == 1 << 10))
                || (mant == 1 + (1 << 10) && exp == 16)
                || (x.mant == 1 + (1 << 10) && x.exp == 16)) {
        exp = 16;
        mant = 1 + (1 << 10);
        return;
    } else if (mant == 1 << 10 && exp == 16 || x.mant == 1 << 10 && x.exp == 16) {
        mant = 1 << 10;
        exp = 16;
        s ^= x.s;
        return;
    } else if (mant == 0 || x.mant == 0) {
        mant = 0;
        exp = -14;
        s ^= x.s;
        return;
    }
    mant *= x.mant;
    exp += x.exp - 10;
    s ^= x.s;
    rearrange();
}

void Half::div(const Half& x) {
    if ((mant == 0 && x.mant == 0)
            || (mant == 1 << 10 && exp == 16 && x.mant == 1 << 10 && x.exp == 16)
            || (mant == 1 + (1 << 10) && exp == 16)
            || (x.mant == 1 + (1 << 10) && x.exp == 16)) {
        mant = 1 + (1 << 10);
        exp = 16;
        return;
    } else if (mant == 0 || x.mant == 1 << 10 && x.exp == 16) {
        mant = 0;
        exp = -14;
        s ^= x.s;
        return;
    } else if (x.mant == 0 || mant == 1 << 10 && exp == 16) {
        mant = 1 << 10;
        exp = 16;
        s ^= x.s;
        return;
    }

    uint64_t mask = 0;
    int start = 10;
    int fill = 11;
    uint64_t tmpMant = 0;
    while (mask / x.mant == 0) {
        mask <<= 1;
        if (start >= 0) {
            mask += ((mant >> start) & 1);
            start--;
        } else {
            exp--;
        }
    }
    while (fill != -1) {
        tmpMant += (mask / x.mant) << fill;
        fill--;
        mask %= x.mant;
        mask <<= 1;
        if (start >= 0) {
            mask += ((mant >> start) & 1);
            start--;
        } else if (fill != 0) {
                exp--;
        }
    }
    if (mask != 0 && s == x.s) {
        tmpMant |= 1;
    }
    mant = tmpMant;
    exp -= x.exp - 10;
    s ^= x.s;
    rearrange();
}

void Half::add(const Half& x) {
    if ((mant == 1 + (1 << 10) && exp == 16)
            || (x.mant == 1 + (1 << 10) && x.exp == 16)
            || (mant == 1 << 10 && x.mant == 1 << 10
                    && exp == 16 && x.exp == 16 && s != x.s)) {
        mant = 1 + (1<<10);
        exp = 16;
        return;
    } else if (mant == 1 << 10 && exp == 16) {
        return;
    } else if (x.mant == 1 << 10 && x.exp == 16) {
        mant = 1 << 10;
        exp = 16;
        s = x.s;
        return;
    } else if (mant == x.mant && exp == x.exp && s^x.s) {
        mant = 0;
        exp = -14;
        s = 0;
        return;
    } else if (mant == 0) {
        s = x.s;
        mant = x.mant;
        exp = x.exp;
        return;
    } else if (x.mant == 0) {
        return;
    }

    Half xCopy(x);
    for (int i = 30; i >=0; i--) {
        if ((xCopy.mant >> i) & 1) {
            xCopy.mant <<= (30 - i);
            xCopy.exp -= (30 - i);
            break;
        }
    }
    for (int i = 30; i >=0; i--) {
        if ((mant >> i) & 1) {
            mant <<= (30 - i);
            exp -= (30 - i);
            break;
        }
    }
    
    if (exp < xCopy.exp || (exp == xCopy.exp && mant < xCopy.mant)) {
        std::swap(s, xCopy.s);
        std::swap(exp, xCopy.exp);
        std::swap(mant, xCopy.mant);
    }
    if (s == xCopy.s) {
        mant += xCopy.mant >> std::min(exp - xCopy.exp, 60);
        if (!s && xCopy.mant & ((1LL << std::min(exp - xCopy.exp, 60)) - 1)) {
            mant |= 1;
        }
    } else {
        mant -= (xCopy.mant >> std::min(exp - xCopy.exp, 60));
        if (xCopy.mant & ((1 << std::min(exp - xCopy.exp, 60)) - 1)) {
            mant -= 1;
            if (!s) {
                mant |= 1;
            }
        }
    }
    rearrange();
}

void Half::sub(const Half& x) {
    Half xCopy(x);
    xCopy.s = 1 - xCopy.s;
    add(xCopy);
}

void Half::rearrange() {
    bool round = false;
    for (int i = 60; i >= 0; i--) {
        if ((mant >> i) & 1) {
            if (i > 10) {
                round = !s && (mant & ((1 << (i - 10)) - 1));
                mant >>= (i - 10);
                exp += (i - 10);
            } else {
                mant <<= (10 - i);
                exp -= (10 - i);
            }
            break;
        }
    }
    if (mant==0 || s && exp < -14 - 10) {
        mant = 0;
        exp = -14;
    } else if (exp >= 16) {
        if (s) {
            exp = 15;
            mant = (1LL << 11) - 1;
            return;
        }
        mant = 1 << 10;
        exp = 16;
    } else if (exp <= -15) {
        if (round || (!s && (mant & ((1LL << std::min(-14 - exp, 60)) - 1)))) {
            mant >>= std::min(-14 - exp, 60);
            mant += 1;
            exp = -14;
            rearrange();
        } else {
            mant >>= -14 - exp;
            exp = -14;
        }
    } else {
        if (round) {
            mant += 1;
            rearrange();
        }
    }
}

void Half::print() {
    if (mant == 1 + (1 << 10) && exp == 16) {
        std::cout << "nan\n";
        return;
    }
    if (s) {
        std::cout << '-';
    } 
    if (mant == 1 << 10 && exp == 16) {
        std::cout << "inf\n";
    } else if (mant == 0) {
        std::cout << "0x0.000p+0\n";
    } else {
        uint64_t tmpMant = mant;
        int tmpExp = exp - 10;
        for (int i = 10; i >= 0; i--) {
            if ((mant >> i) & 1) {
                tmpMant = mant << (12 - i);
                tmpExp += i;
                break;
            }
        }
        std::cout << "0x1.";
        for (int i = 8; i >= 0; i-=4) {
            std::cout << std::hex << ((tmpMant>>i) & 15);
        }
        if (tmpExp >= 0) {
            std::cout << "p+" << std::dec << tmpExp << std::endl;
        } else {
            std::cout << "p" << std::dec << tmpExp << std::endl;
        }
    }
}