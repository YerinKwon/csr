#ifndef BITMAP_H_
#define BITMAP_H_

#include <algorithm>
#include <cstdint>

class Bitmap{
public:
    Bitmap(int s){ 
        size = (s + bpw - 1)/bpw;
        bm = new uint64_t[size];}
    ~Bitmap(){ delete[] bm;}

    void printBm(){
        for(int i=0;i<size;++i) printf("%ld ",bm[i]);
        printf("\n");
    }
    
    void reset(){ std::fill(bm, bm+size, 0);}
    uint64_t arrIdx(int n){ return n / bpw;}
    uint64_t bitIdx(int n){ return n & (bpw - 1);}
    bool isSet(int n){ return bm[arrIdx(n)] & (1l <<bitIdx(n));} //1l?
    void setBit(int n){ bm[arrIdx(n)] |= ((uint64_t) 1l << bitIdx(n));}
    void setBitAtomic(int n){ 
        int curVal, newVal;
        do{
            curVal = bm[arrIdx(n)];
            newVal = curVal | ((uint64_t) 1l <<bitIdx(n));
        }
        while(!__sync_bool_compare_and_swap(&bm[arrIdx(n)], curVal, newVal));
    }
    void swap(Bitmap &inputbm){std::swap(bm, inputbm.bm);}

private:
    const uint64_t bpw = 64;
    uint64_t * bm;
    uint64_t size;
};

#endif
