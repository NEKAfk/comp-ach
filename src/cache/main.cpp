#include<iostream>
#include<string>
#include<bitset>
#include<stdint.h>

#define M 64
#define N 60
#define K 32

#define ADDR_LEN 20
#define CACHE_TAG_LEN 9
#define CACHE_OFFSET_LEN 7 //log2(CACHE_LINE_SIZE)
#define CACHE_IDX_LEN (ADDR_LEN - CACHE_TAG_LEN - CACHE_OFFSET_LEN)
#define MEM_SIZE (1ULL << ADDR_LEN)

#define CACHE_WAY 4
#define CACHE_LINE_SIZE 128
#define CACHE_SETS_COUNT (1 << CACHE_IDX_LEN)
#define CACHE_LINE_COUNT (CACHE_SETS_COUNT * CACHE_WAY)
#define CACHE_SIZE (CACHE_LINE_SIZE * CACHE_LINE_COUNT)
#define FL_LEN 4
//#define FL_LEN 3 //pLRU
//#define FL_LEN (2 + 2) //2 + log2(CACHE_WAY) //LRU

#define ADDR1_BUS_LEN ADDR_LEN
#define ADDR2_BUS_LEN ADDR_LEN - CACHE_OFFSET_LEN
#define DATA1_BUS_LEN 16
#define DATA2_BUS_LEN 16
#define CTR1_BUS_LEN 3
#define CTR2_BUS_LEN 2

using namespace std;

// int8_t a[M][K];
// int16_t b[K][N];
// int c[M][N];
int clk = 0;
int all = 0;
int hit = 0;
bool isLRU = true;

void C1_RESPONSE() {
  clk+=1;
}

void C2_RESPONSE() {
  clk+=1;
}


class CacheLine {
public:
  bitset<FL_LEN> flags;
  CacheLine() {
    flags = 0;
    flags[0] = 1;
  }
  int tag;
  int8_t *data = new int8_t[CACHE_LINE_SIZE];
};

class Mem {
public:
int8_t *ram;
Mem() {
  ram = new int8_t[MEM_SIZE];
}
void writeline(int8_t *data, int addr) {
  int times = CACHE_LINE_SIZE * 8 / DATA2_BUS_LEN;
  clk+=times;
  for (int c = 0; c < times; c++) {
    for (int j = 0; j < DATA2_BUS_LEN / 8; j++)
    {
      ram[addr] = data[addr & ((1 << CACHE_OFFSET_LEN) - 1)];
      addr++;
    }
  }
}

void readline(int8_t *data, int addr) {
  int times = CACHE_LINE_SIZE * 8 / DATA2_BUS_LEN;
  clk+=times;
  for (int c = 0; c < times; c++) {
    for (int j = 0; j < DATA2_BUS_LEN / 8; j++)
    {
      data[addr & ((1 << CACHE_OFFSET_LEN) - 1)] = ram[addr];
      addr++;
    }
  }
}
};

class Cache {
public:
  CacheLine** cache;//[CACHE_SETS_COUNT][CACHE_WAY];
  Mem mem;
  Cache(Mem &mem) {
    this->mem = mem;
    cache = new CacheLine* [CACHE_SETS_COUNT];
    for (int i = 0; i < CACHE_SETS_COUNT; i++)
    {
      cache[i] = new CacheLine[CACHE_WAY];
    }
  }

  void zeroFlags(int index, int i) {
    int s = 0;
    for (int j = 0; j < CACHE_WAY; j++) {
      s += cache[index][j].flags[2] * (1 - cache[index][j].flags[0]);
    }
    if (s == CACHE_WAY) {
      for (int j = 0; j < CACHE_WAY; j++) {
        cache[index][j].flags[2] = 0;
      }
    }
    cache[index][i].flags[2] = 1;
  }

  void updateFlags(int index, int i) {
    int tmp = (cache[index][i].flags >> 2).to_ullong();
    for (int j = 0; j < CACHE_WAY; j++)
    {
      if ((cache[index][j].flags.to_ullong() >> 2) > tmp) {
        cache[index][j].flags = cache[index][j].flags.to_ullong() - 4;
      }
    }
    cache[index][i].flags = (cache[index][i].flags.to_ullong() & 3) + (((1 << (FL_LEN - 2)) - 1) << 2);
  }

  bool inCache(int addr) {
    int index = (addr >> CACHE_OFFSET_LEN) & ((1 << CACHE_IDX_LEN) - 1);
    int tag = (addr >> (CACHE_OFFSET_LEN + CACHE_IDX_LEN));
    for (int i = 0; i < CACHE_WAY; i++) {
      if (cache[index][i].tag == tag && !cache[index][i].flags[0]) {
        return true;
      }
    }
    return false;
  }

  int getFromCache(int addr, int bytes) {
    int times = max(bytes, DATA1_BUS_LEN) / DATA1_BUS_LEN;
    clk += times;
    int result = 0;
    int index = (addr >> CACHE_OFFSET_LEN) & ((1 << CACHE_IDX_LEN) - 1);
    int tag = (addr >> (CACHE_OFFSET_LEN + CACHE_IDX_LEN));
    int offset = addr & ((1 << CACHE_OFFSET_LEN) - 1);
    for (int i = 0; i < CACHE_WAY; i++) {
      if (cache[index][i].tag == tag) {
        if (isLRU) {
          updateFlags(index, i);
        } else {
          cache[index][i].flags[2] = 1;
          zeroFlags(index, i);
        }
        for (int c = 0; c < times; c++) {
          for (int j = 1; j <= DATA1_BUS_LEN / 8 && c*DATA1_BUS_LEN + 8*j <= bytes; j++)
          {
            result <<= 8;
            result += (cache[index][i].data[offset]);
            offset++;
          }
        }
        break;
      }
    }
    return result;
  }

  int FreeLine(int index) {
    int i = 0;
    for (;i < CACHE_WAY; i++) {
      if (cache[index][i].flags[0] == 1) {
        return i;
      }
    }
    i = 0;
    for (;i < CACHE_WAY; i++) {
      if ((cache[index][i].flags.to_ullong() >> 2) == 0) {
        break;
      }
    }
    if (cache[index][i].flags[1] == 1) {
      clk++;
      clk+=100;
      mem.writeline(cache[index][i].data, (cache[index][i].tag << (CACHE_IDX_LEN + CACHE_OFFSET_LEN)) + (index << CACHE_IDX_LEN));
      C2_RESPONSE();
    }
    cache[index][i].flags[0] = 1;
    return i;
  }

  void readFromMem(int addr) {
    int index = (addr >> CACHE_OFFSET_LEN) & ((1 << CACHE_IDX_LEN) - 1);
    int tag = (addr >> (CACHE_OFFSET_LEN + CACHE_IDX_LEN));
    int i = FreeLine(index);
    cache[index][i].tag = tag;
    cache[index][i].flags[0] = 0;
    cache[index][i].flags[1] = 0;
    if (isLRU) {
      updateFlags(index, i);
    } else {
      cache[index][i].flags[2] = 1;
      zeroFlags(index, i);
    }
    clk++;
    clk+=100;
    mem.readline(cache[index][i].data, (tag << (CACHE_IDX_LEN + CACHE_OFFSET_LEN)) + (index << CACHE_IDX_LEN));
    C2_RESPONSE();
  }

  void writeToMem(int addr) {
    int index = (addr >> CACHE_OFFSET_LEN) & ((1 << CACHE_IDX_LEN) - 1);
    int tag = (addr >> (CACHE_OFFSET_LEN + CACHE_IDX_LEN));
    for (int i = 0; i < CACHE_WAY; i++) {
      if (cache[index][i].tag == tag && cache[index][i].flags[0] == 0 && cache[index][i].flags[1] == 1) {
        cache[index][i].flags[1] = 0;
        cache[index][i].flags[0] = 1;
        clk++;
        clk+=100;
        mem.writeline(cache[index][i].data, (cache[index][i].tag << (CACHE_IDX_LEN + CACHE_OFFSET_LEN)) + (index << CACHE_IDX_LEN));
        C2_RESPONSE();
        break;
      }
    }
  }

  void writeToCache(int addr, int data, int bytes) {
    int times = bytes / DATA1_BUS_LEN;
    clk+=times;
    int index = (addr >> CACHE_OFFSET_LEN) & ((1 << CACHE_IDX_LEN) - 1);
    int tag = (addr >> (CACHE_OFFSET_LEN + CACHE_IDX_LEN));
    int offset = addr & ((1 << CACHE_OFFSET_LEN) - 1);
    for (int i = 0; i < CACHE_WAY; i++) {
      if (cache[index][i].tag == tag) {
        cache[index][i].flags[1] = 1;
        if (isLRU) {
          updateFlags(index, i);
        } else {
          cache[index][i].flags[2] = 1;
          zeroFlags(index, i);
        }
        for (int c = 1; c <= times; c++) {
          for (int j = DATA1_BUS_LEN / 8 - 1; j >= 0; j--)
          {
            cache[index][i].data[offset] = (data >> (DATA1_BUS_LEN*(times - c) + 8 * j)) & ((1 << 8) - 1);
            offset++;
          }
        }
        break;
      }
    }
  }

  void flush() {
    for (int i = 0; i < CACHE_SETS_COUNT; i++)
    {
      for (int j = 0; j < CACHE_WAY; j++)
      {
        if (cache[i][j].flags[0] == 1 && cache[i][j].flags[1] == 0) {
          continue;
        }
        cache[i][j].flags[0] = 1;
        clk++;
        clk+=100;
        mem.writeline(cache[i][j].data, (cache[i][j].tag << (CACHE_IDX_LEN + CACHE_OFFSET_LEN)) + (i << CACHE_IDX_LEN));
        C2_RESPONSE();
      }
    }
  }

  void Reset() {
    for (int i = 0; i < CACHE_SETS_COUNT; i++)
    {
      int cnt = 0;
      for (int j = 0; j < CACHE_WAY; j++)
      {
        cache[i][j].flags = 0;
        cache[i][j].flags[0] = 1;
        if (isLRU) {
          cache[i][j].flags = cache[i][j].flags.to_ullong() + (cnt << 2);
          cnt++;
        }
      }
    }
  }

  ~Cache() {
    for (int i = 0; i < CACHE_SETS_COUNT; i++) {
      delete[] cache[i];
    }
    delete[] cache;
  }
};

Mem mem;
Cache cache(mem);
int reg[8];

#pragma region SmallInstructions
int getReg(string Rx) {
  return reg[stoull(Rx.substr(1, 1))];
}
void MOV(string Rx, int val) {
  clk++;
  reg[stoull(Rx.substr(1, 1))] = val;
}
void ADD(string Rx, int val) {
  clk++;
  reg[stoull(Rx.substr(1, 1))] += val;
}
void ADD(string Rx, string Ry) {
  clk++;
  reg[stoull(Rx.substr(1, 1))] += reg[stoull(Ry.substr(1, 1))];
}
void MUL(string Rx, int val) {
  clk+=5;
  reg[stoull(Rx.substr(1, 1))] *= val;
}
void MUL(string Rx, string Ry) {
  clk+=5;
  reg[stoull(Rx.substr(1, 1))] *= reg[stoull(Ry.substr(1, 1))];
}

#pragma endregion

#pragma region BigInstructions

void C2_READ_LINE(int addr) {
  cache.readFromMem(addr);
}
void C2_WRITE_LINE(int addr) {
  cache.writeToMem(addr);
}

void C1_READ8(string Rx, int addr) {
  clk++;
  if (!cache.inCache(addr)) {
    clk+=4;
    C2_READ_LINE(addr);
  } else {
    clk+=6;
    hit++;
  }
  C1_RESPONSE();
  all++;
  reg[stoull(Rx.substr(1, 1))] = cache.getFromCache(addr, 8) & ((1ULL << 8) - 1);
}

void C1_READ16(string Rx, int addr) {
  clk++;
  if (!cache.inCache(addr)) {
    clk+=4;
    C2_READ_LINE(addr);
  } else {
    clk+=6;
    hit++;
  }
  C1_RESPONSE();
  all++;
  reg[stoull(Rx.substr(1, 1))] = cache.getFromCache(addr, 16);
}

void C1_READ32(string Rx, int addr) {
  clk++;
  if (!cache.inCache(addr)) {
    clk+=4;
    C2_READ_LINE(addr);
  } else {
    clk+=6;
    hit++;
  }
  C1_RESPONSE();
  all++;
  reg[stoull(Rx.substr(1, 1))] = cache.getFromCache(addr, 32);
}

void C1_WRITE32(int addr, string Rx) {
  clk++;
  if (!cache.inCache(addr)) {
    clk+=4;
    C2_READ_LINE(addr);
  } else {
    clk+=6;
    hit++;
  }
  C1_RESPONSE();
  all++;
  cache.writeToCache(addr, reg[stoull(Rx.substr(1, 1))], 32);
}

void C1_WRITE16(int addr, string Rx) {
  clk++;
  if (!cache.inCache(addr)) {
    clk+=4;
    C2_READ_LINE(addr);
  } else {
    clk+=6;
    hit++;
  }
  C1_RESPONSE();
  all++;
  cache.writeToCache(addr, reg[stoull(Rx.substr(1, 1))], 16);
}

void C1_WRITE8(int addr, string Rx) {
  clk++;
  if (!cache.inCache(addr)) {
    clk+=4;
    C2_READ_LINE(addr);
  } else {
    clk+=6;
    hit++;
  }
  C1_RESPONSE();
  all++;
  cache.writeToCache(addr, reg[stoull(Rx.substr(1, 1))], 8);
}

#pragma endregion

void mmul()
{
  MOV("R0", 1ULL << 18);//pa

  MOV("R1", (1ULL << 18) + M*K + 2*K*N);//pc

  MOV("R3", 0);//y
for1:
  if (getReg("R3") >= M) goto endfor1;
  MOV("R4", 0);//x
  for2:
    if (getReg("R4") >= N) goto endfor2;
    MOV("R2", (1ULL << 18) + M*K);//pb

    MOV("R5", 0);//s

    MOV("R6", 0);//k
    for3:
      if (getReg("R6") >= K) goto endfor3;

      C1_READ8("R7", getReg("R0") + getReg("R6"));
      C1_READ16("R8", getReg("R2") + 2*getReg("R4"));
      MUL("R7", "R8");
      ADD("R5", "R7");

      ADD("R2", 2*N);

      ADD("R6", 1);
      clk++;
    goto for3;
    endfor3:

    C1_WRITE32(getReg("R1") + 4*getReg("R4"), "R5");

    ADD("R4", 1);
    clk++;
  goto for2;
  endfor2:

  ADD("R0", K);

  ADD("R1", 4*N);

  ADD("R3", 1);
  clk++;
goto for1;
endfor1:
clk++;
return;
}

int main() {
  cache.Reset();
  mmul();
  isLRU = false;
  float ans1 = (float)hit / all * 100;
  int clk1 = clk;
  hit = 0;
  all = 0;
  clk = 0;
  cache.Reset();
  mmul();
  float ans2 = (float)hit / all * 100;
  int clk2 = clk;
  printf("LRU:\thit perc. %3.4f%%\ttime: %d\npLRU:\thit perc. %3.4f%%\ttime: %d\n", ans1, clk1, ans2, clk2);
}
