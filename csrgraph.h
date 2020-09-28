#ifndef CSRGRAPH_H_
#define CSRGRAPH_H_

#include <vector>
#include <algorithm>
#include <utility>
#include <cstdio>

typedef std::pair<int,int> Edge;

class CSRGraph{
    int num_node_, nodelist_size_;
    int** out_idx_;
    int* out_nodelist_;
    int** in_idx_;
    int* in_nodelist_;

    class Neighbor{
        int node_id;
        int **next_list;
        int s_offset;
    public:
        Neighbor(int id, int** n_list, int o) : node_id(id), next_list(n_list), s_offset(0){
            int max_offset = end()-begin();
            s_offset = s_offset<max_offset ? s_offset:max_offset;
        }
        int* begin(){ return next_list[node_id]+s_offset;}
        int* end(){ return next_list[node_id+1];}
    };


    int find_max_node(std::vector<Edge> &el){
        int max_node = 0;
        #pragma omp parallel for reduction(max: max_node)
        for(auto iter = el.begin(); iter<el.end(); ++iter){
            int mx = std::max(iter->first, iter->second);
            max_node = std::max(max_node, mx);
        }
        return max_node;
    }

    int* count_degree(std::vector<Edge> &el, bool inv){
        int* degree = new int[num_node_];
        #pragma omp parallel for
        for(int i=0;i<num_node_;++i) degree[i] = 0;

        #pragma omp parallel for
        for(auto iter = el.begin();iter<el.end();++iter){
            if(inv) __sync_fetch_and_add(&degree[iter->second],1);
            else __sync_fetch_and_add(&degree[iter->first],1);
        }
        return degree;
    }

    int* count_offset(int* degree){
        int block_size = 1<<20;
        int num_block = (num_node_ + (block_size-1))/block_size;

        int* offset = new int[num_node_+1];
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
            int cur = 0;
            for(int j=block_size*i; j<end;++j){
                offset[j] = cur;
                cur += degree[j];
            }
        }

        return offset;
    }

    void set_index(int* offset, int** idx, int* nodelist){
        #pragma omp parallel for
        for(int i=0;i<num_node_+1;++i)
            idx[i] = nodelist + offset[i];
    }

    void makeCSR(std::vector<Edge> &el, int** idx, int* nodelist, bool inv){
        int *degree = count_degree(el, inv);
        int *offset = count_offset(degree);
        set_index(offset, idx, nodelist);

        for(auto iter = el.begin(); iter<el.end();++iter){
            if(inv) nodelist[__sync_fetch_and_add(&offset[iter->second],1)] = iter->first;
            else nodelist[__sync_fetch_and_add(&offset[iter->first],1)] = iter->second;
        }

        delete[] degree;
        delete[] offset;
    }

public:
    CSRGraph(std::vector<Edge> &el){
        nodelist_size_ = el.size();
        num_node_ =  find_max_node(el)+1;

        out_idx_ =  new int*[num_node_+1];
        out_nodelist_ =  new int[nodelist_size_];
        in_idx_ =  new int*[num_node_+1];
        in_nodelist_ =  new int[nodelist_size_];

        makeCSR(el, out_idx_, out_nodelist_, false);
        makeCSR(el, in_idx_, in_nodelist_, true);
    }


    ~CSRGraph(){
        delete[] out_idx_;
        delete[] out_nodelist_;
        delete[] in_idx_;
        delete[] in_nodelist_;
    };

    int num_node(){ return num_node_;}
    int nodelist_size(){ return nodelist_size_;}
    // int outidx(int n){ return idx[n];}
    // int outnodelist(int i){ return nodelist[i];}
    // int inidx(int n){ return in_idx[n];}
    // int innodelist(int i){ return in_nodelist[i];}

    // Neighbor in_neigh(int n, int s_o = 0){ return Neighbor(n,in_idx,s_o);}
    // Neighbor out_neigh(int n, int s_o = 0){ return Neighbor(n,idx,s_o);}

    // void printCSRGraph(){
    //     printf("Graph: \n");
    //     for(int i=0; i < num_node_; ++i){
    //         printf("Node %d: ",i);
    //         for(int j=idx[i];j<idx[i+1];++j) printf("%d ",nodelist[j]);
    //         printf("\n");
    //     }
    //     printf("\n");

    //     printf("Inversed Graph: \n");
    //     for(int i=0; i < num_node_; ++i){
    //         printf("Node %d: ",i);
    //         for(int j=in_idx[i];j<in_idx[i+1];++j) printf("%d ",in_nodelist[j]);
    //         printf("\n");
    //     }
    //     printf("\n");
    // }
};

#endif