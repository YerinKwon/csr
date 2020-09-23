#ifndef SLIDING_Q_H_
#define SLIDING_Q_H_

#include <algorithm>

class SlidingQ{
public:
    SlidingQ(int s){
        sq = new int[s];
        idx_in = 0, idx_s = 0, idx_e = 0;
    }
    ~SlidingQ(){ delete[] sq;}

    void push_back(int val){ sq[idx_in++] = val;}
    void slide(){ idx_s = idx_e; idx_e = idx_in;}
    bool empty(){ return idx_s == idx_e;}
    int size(){ return idx_e-idx_s;}
    int start(){ return idx_s;}
    int end(){ return idx_e;}
    int get(int i){return sq[i];}

private:
    int idx_in, idx_s, idx_e;
    int *sq;
    friend class Qbuffer;
};

class Qbuffer{
public:    
    Qbuffer(SlidingQ &master, int size = 16384) : sq(master), lqSize(size){
        idx = 0;
        lq = new int[lqSize];
    }
    ~Qbuffer(){ delete[] lq;}

    void flush(){
        int from = __sync_fetch_and_add(&sq.idx_in,idx);
        std::copy(lq, lq+idx, sq.sq+from);
        idx = 0;
    }

    void push_back(int val){
        if(idx == lqSize) flush();
        lq[idx++] = val;
    }

private:
    int idx;
    int *lq;
    SlidingQ &sq;
    const int lqSize;
};


#endif