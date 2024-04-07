#include"float.h"

Float::Float(uint64_t n) {
    s = (n >> 31) & 1;
    exp = (255 & (n >> 23)) - 127;
    mant = n & ((1 << 23) - 1);
    if (exp == -127 && mant == 0) {
        exp = -126;
        return;
    } else if (exp == 128 && mant == 0) {
        mant = 1 << 23;
        return;
    } else if (exp == 128 && mant != 0) {
        mant = 1 + (1 << 23);
        return;
    }
    if (exp != -127) {
        mant += 1 << 23;
    } else {
        exp = -126;
    }
}

void Float::mul(const Float& x) {
    if (((mant == 0 && x.exp== 128 && x.mant == 1<<23)
                || (x.mant == 0 && exp == 128 && mant == 1 << 23))
                || (mant == 1 + (1 << 23) && exp == 128)
                || (x.mant == 1 + (1 << 23) && x.exp == 128)) {
        exp = 128;
        mant = 1 + (1 << 23);
        return;
    } else if (mant == 1 << 23 && exp == 128 || x.mant == 1 << 23 && x.exp == 128) {
        mant = 1 << 23;
        exp = 128;
        s ^= x.s;
        return;
    } else if (mant == 0 || x.mant == 0) {
        mant = 0;
        exp = -126;
        s ^= x.s;
        return;
    }
    mant *= x.mant;
    exp += x.exp - 23;
    s ^= x.s;
    rearrange();
}

void Float::div(const Float& x) {
    if ((mant == 0 && x.mant == 0)
            || (mant == 1 << 23 && exp == 128 && x.mant == 1 << 23 && x.exp == 128)
            || (mant == 1 + (1 << 23) && exp == 128)
            || (x.mant == 1 + (1 << 23) && x.exp == 128)) {
        mant = 1 + (1 << 23);
        exp = 128;
        return;
    } else if (mant == 0 || x.mant == 1 << 23 && x.exp == 128) {
        mant = 0;
        exp = -126;
        s ^= x.s;
        return;
    } else if (x.mant == 0 || mant == 1 << 23 && exp == 128) {
        mant = 1 << 23;
        exp = 128;
        s ^= x.s;
        return;
    }

    uint64_t mask = 0;
    int start = 23;
    int fill = 24;
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
    exp -= x.exp - 23;
    s ^= x.s;
    rearrange();
}

void Float::add(const Float& x) {
    if ((mant == 1 + (1 << 23) && exp == 128)
            || (x.mant == 1 + (1 << 23) && x.exp == 128)
            || (mant == 1 << 23 && x.mant == 1 << 23
                    && exp == 128 && x.exp == 128 && s != x.s)) {
        mant = 1 + (1<<23);
        exp = 128;
        return;
    } else if (mant == 1 << 23 && exp == 128) {
        return;
    } else if (x.mant == 1 << 23 && x.exp == 128) {
        mant = 1 << 23;
        exp = 128;
        s = x.s;
        return;
    } else if (mant == x.mant && exp == x.exp && s != x.s) {
        mant = 0;
        exp = -126;
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

    Float xCopy(x);
    for (int i = 50; i >=0; i--) {
        if ((xCopy.mant >> i) & 1) {
            xCopy.mant <<= (50 - i);
            xCopy.exp -= (50 - i);
            break;
        }
    }
    for (int i = 50; i >=0; i--) {
        if ((mant >> i) & 1) {
            mant <<= (50 - i);
            exp -= (50 - i);
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
        if (xCopy.mant & ((1LL << std::min(exp - xCopy.exp, 60)) - 1)) {
            mant -= 1;
            if (!s) {
                mant |= 1;
            }
        }
    }
    rearrange();
}

void Float::sub(const Float& x) {
    Float xCopy(x);
    xCopy.s = 1 - xCopy.s;
    add(xCopy);
}

void Float::rearrange() {
    bool round = false;
    for (int i = 60; i >= 0; i--) {
        if ((mant >> i) & 1) {
            if (i > 23) {
                round = !s && (mant & ((1 << (i - 23)) - 1));
                mant >>= (i - 23);
                exp += (i - 23);
            } else {
                mant <<= (23 - i);
                exp -= (23 - i);
            }
            break;
        }
    }
    if (mant==0 || s && exp < -126 - 23) {
        mant = 0;
        exp = -126;
    } else if (exp >= 128) {
        if (s) {
            exp = 127;
            mant = (1LL << 24) - 1;
            return;
        }
        mant = 1 << 23;
        exp = 128;
    } else if (exp <= -127) {
        if (round || (!s && (mant & ((1LL << std::min(-126 - exp, 60)) - 1)))) {
            mant >>= std::min(-126 - exp, 60);
            mant += 1;
            exp = -126;
            rearrange();
        } else {
            mant >>= -126 - exp;
            exp = -126;
        }
    } else {
        if (round) {
            mant += 1;
            rearrange();
        }
    }
}

void Float::print() {
    if (mant == 1 + (1 << 23) && exp == 128) {
        std::cout << "nan\n";
        return;
    }
    if (s) {
        std::cout << '-';
    } 
    if (mant == 1 << 23 && exp == 128) {
        std::cout << "inf\n";
    } else if (mant == 0) {
        std::cout << "0x0.000000p+0\n";
    } else {
        uint64_t tmpMant = mant;
        int tmpExp = exp - 23;
        for (int i = 23; i >= 0; i--) {
            if ((mant >> i) & 1) {
                tmpMant = mant << (24 - i);
                tmpExp += i;
                break;
            }
        }
        std::cout << "0x1.";
        for (int i = 20; i >= 0; i-=4) {
            std::cout << std::hex << ((tmpMant>>i) & 15);
        }
        if (tmpExp >= 0) {
            std::cout << "p+" << std::dec << tmpExp << std::endl;
        } else {
            std::cout << "p" << std::dec << tmpExp << std::endl;
        }
    }
}