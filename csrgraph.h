#ifndef CSRGRAPH_H_
#define CSRGRAPH_H_

#include <vector>
#include <algorithm>
#include <utility>
#include <cstdio>

typedef std::pair<int,int> Edge;

class CSRGraph{
public:
    CSRGraph(int s, std::vector<Edge> &EdgeList){
        num_node_ = s, nodelist_size_ = EdgeList.size();
        idx.reserve(num_node_+1);
        nodelist.reserve(nodelist_size_);
        in_idx.reserve(num_node_+1);
        in_nodelist.reserve(nodelist_size_);
        
        std::sort(EdgeList.begin(),EdgeList.end());
        idx.push_back(0);
        for(int i=0;i<nodelist_size_;++i){
            while(idx.size() <= EdgeList[i].first) idx.push_back(i);
            nodelist.push_back(EdgeList[i].second);
        }
        while(idx.size() <= num_node_) idx.push_back(nodelist_size_);

        std::sort(EdgeList.begin(),EdgeList.end(),[](Edge a,Edge b)->bool{
            if(a.second == b.second) return a.first<b.first;
            return a.second<b.second;
        });
        in_idx.push_back(0);
        for(int i=0;i<nodelist_size_;++i){
            while(in_idx.size() <= EdgeList[i].second) in_idx.push_back(i);
            in_nodelist.push_back(EdgeList[i].first);
        }
        while(in_idx.size() <= num_node_) in_idx.push_back(nodelist_size_);
    }
    ~CSRGraph(){};

    int num_node(){ return num_node_;}
    int nodelist_size(){ return nodelist_size_;}
    int outidx(int n){ return idx[n];}
    int outnodelist(int i){ return nodelist[i];}
    int inidx(int n){ return in_idx[n];}
    int innodelist(int i){ return in_nodelist[i];}

    void printCSRGraph(){
        printf("Graph: \n");
        for(int i=0; i < num_node_; ++i){
            printf("Node %d: ",i);
            for(int j=idx[i];j<idx[i+1];++j) printf("%d ",nodelist[j]);
            printf("\n");
        }
        printf("\n");

        printf("Inversed Graph: \n");
        for(int i=0; i < num_node_; ++i){
            printf("Node %d: ",i);
            for(int j=in_idx[i];j<in_idx[i+1];++j) printf("%d ",in_nodelist[j]);
            printf("\n");
        }
        printf("\n");
    }

private:
    int num_node_, nodelist_size_;
    std::vector<int> idx;
    std::vector<int> nodelist;
    std::vector<int> in_idx;
    std::vector<int> in_nodelist;
};

#endif