#ifndef CSRGRAPH_H_
#define CSRGRAPH_H_

#include <vector>
#include <algorithm>
#include <utility>
#include <cstdio>
#include <iostream>
#include <type_traits>

typedef int64_t Offset;

template <typename Node>
class CSRGraph{
    typedef std::pair<Node,Node> Edge;

    int64_t num_node_;
    int64_t nodelist_size_;
    Node** out_idx_;
    Node* out_nodelist_;
    Node** in_idx_;
    Node* in_nodelist_;

    typedef std::make_unsigned<std::ptrdiff_t>::type OffsetT;

    class Neighborhood {
        Node n_;
        Node** g_index_;
        OffsetT start_offset_;
    public:
        Neighborhood(Node n, Node** g_index, OffsetT start_offset) :
            n_(n), g_index_(g_index), start_offset_(0) {
            OffsetT max_offset = end() - begin();
            start_offset_ = std::min(start_offset, max_offset);
        }
        typedef Node* iterator;
        iterator begin() { return g_index_[n_] + start_offset_; }
        iterator end()   { return g_index_[n_+1]; }
    };

    Node find_max_node(std::vector<Edge> &el){
        Node max_node = 0;
        #pragma omp parallel for reduction(max: max_node)
        for(auto iter = el.begin(); iter<el.end(); ++iter){
            Node mx = std::max(iter->first, iter->second);
            max_node = std::max(max_node, mx);
        }
        return max_node;
    }

    Node* count_degree(std::vector<Edge> &el, bool inv){
        Node* degree = new Node[num_node_];
        #pragma omp parallel for
        for(int i=0;i<num_node_;++i) degree[i] = 0;

        #pragma omp parallel for
        for(auto iter = el.begin();iter<el.end();++iter){
            if(inv) __sync_fetch_and_add(&degree[iter->second],1);
            else __sync_fetch_and_add(&degree[iter->first],1);
        }
        return degree;
    }

    Offset* count_offset(Node* degree){
        int64_t block_size = 1<<20;
        int64_t num_block = (num_node_ + (block_size-1))/block_size;

        Offset* offset = new Offset[num_node_+1];
        #pragma omp parallel for
        for(int i=0;i<num_node_+1;++i) offset[i] = 0;

        for(int i=0;i<num_block;++i){
            int end = std::min((i+1)*block_size, num_node_);
            #pragma omp parallel for reduction(+:offset[end])
            for(int j=block_size*i; j<end;++j) offset[end] += degree[j];
        }

        for(int i=0;i<num_block;++i){
            int end = std::min((i+1)*block_size, num_node_);
            offset[end] += offset[block_size*i];
        }

        #pragma omp parallel for
        for(int i=0;i<num_block;++i){
            int end = std::min((i+1)*block_size, num_node_);
            Offset cur = 0;
            for(int j=block_size*i; j<end;++j){
                offset[j] = cur;
                cur += degree[j];
            }
        }

        return offset;
    }

    void set_index(Offset* offset, Node** idx, Node* nodelist){
        #pragma omp parallel for
        for(int i=0;i<num_node_+1;++i)
            idx[i] = nodelist + offset[i];
    }

    void makeCSR(std::vector<Edge> &el, Node** idx, Node* nodelist, bool inv){
        Node *degree = count_degree(el, inv);
        Offset *offset = count_offset(degree);
        set_index(offset, idx, nodelist);

        for(auto iter = el.begin(); iter<el.end();++iter){
            if(inv) nodelist[__sync_fetch_and_add(&offset[iter->second],1)] = iter->first;
            else nodelist[__sync_fetch_and_add(&offset[iter->first],1)] = iter->second;
        }

        delete[] degree;
        delete[] offset;
    }

public:
    CSRGraph(std::vector<Edge> &&el){
        nodelist_size_ = el.size();
        num_node_ =  find_max_node(el)+1;

        out_idx_ =  new Node*[num_node_+1];
        out_nodelist_ =  new Node[nodelist_size_];
        in_idx_ =  new Node*[num_node_+1];
        in_nodelist_ =  new Node[nodelist_size_];

        makeCSR(el, out_idx_, out_nodelist_, false);
        makeCSR(el, in_idx_, in_nodelist_, true);
    }


    ~CSRGraph(){
        delete[] out_idx_;
        delete[] out_nodelist_;
        delete[] in_idx_;
        delete[] in_nodelist_;
    };

    int64_t num_node() const{ return num_node_;}
    int64_t nodelist_size(){ return nodelist_size_;}
    int64_t out_degree(Node n) const { return out_idx_[n+1]-out_idx_[n];}
    int64_t in_degree(Node n) const { return in_idx_[n+1]-in_idx_[n];}
    Node* out_idx(Node n){ return out_idx_[n];}
    Node* in_idx(Node n){ return in_idx_[n];}
    Neighborhood out_neigh(Node n, OffsetT start_offset = 0) const {
        return Neighborhood(n, out_idx_, start_offset);
    }

    Neighborhood in_neigh(Node n, OffsetT start_offset = 0) const {
        return Neighborhood(n, in_idx_, start_offset);
    }
};

#endif