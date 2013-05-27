#ifndef HASH_H_

#define HASH_H_

namespace TimePass {

class Hash {
public:
//重载hash函数
static off_t HashFunc(const char* s) {
    off_t h = 0;
    while(*s) {
        h = 5 * h + *s;
        ++ s;
    }
    return h;
}

static off_t HashFunc(char c) {
    return c;
}

static off_t HashFunc(unsigned char c) {
    return c;
}

static off_t HashFunc(short x) {
    return x;
}

static off_t HashFunc(unsigned short x) {
    return x;
}

static off_t HashFunc(int x) {
    return x;
}

static off_t HashFunc(unsigned int x) {
    return x;
}

static off_t HashFunc(unsigned long x) {
    return x;
}

};

};

#endif
