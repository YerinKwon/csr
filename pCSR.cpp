#include <vector>
#include <utility>
#include <cstdio>
#include <algorithm>
#include <queue>
#include <stack>
#include <cmath>
#include <memory.h>

#include "timer.h"

#define DF 0.85
#define TOLERANCE 0.0001
typedef std::pair<int,int> Edge;

struct CSRGraph{
    int numNode, nodelistSize;
    std::vector<int> idx;
    std::vector<int> nodelist;
    std::vector<int> in_idx;
    std::vector<int> in_nodelist;

    CSRGraph(int s, std::vector<Edge> &EdgeList){
        numNode = s, nodelistSize = EdgeList.size();
        idx.reserve(numNode+1);
        nodelist.reserve(nodelistSize);
        in_idx.reserve(numNode+1);
        in_nodelist.reserve(nodelistSize);
        
        std::sort(EdgeList.begin(),EdgeList.end());
        idx.push_back(0);
        for(int i=0;i<nodelistSize;++i){
            while(idx.size() <= EdgeList[i].first) idx.push_back(i);
            nodelist.push_back(EdgeList[i].second);
        }
        while(idx.size() <= numNode) idx.push_back(nodelistSize);

        std::sort(EdgeList.begin(),EdgeList.end(),[](Edge a,Edge b)->bool{
            if(a.second == b.second) return a.first<b.first;
            return a.second<b.second;
        });
        in_idx.push_back(0);
        for(int i=0;i<nodelistSize;++i){
            while(in_idx.size() <= EdgeList[i].second) in_idx.push_back(i);
            in_nodelist.push_back(EdgeList[i].first);
        }
        while(in_idx.size() <= numNode) in_idx.push_back(nodelistSize);
    }
    ~CSRGraph(){};

    std::vector<int> BFS(int start);
    double* pPageRank();
    void printCSRGraph(bool showGraph, bool showResult, int iter);
};

std::vector<int> CSRGraph::BFS(int start){
    std::vector<int> ret;
    ret.reserve(numNode);

    std::queue<int> q;
    bool visit[numNode];
    memset(visit,false,numNode);

    q.push(start-1);
    visit[start-1] = true;
    while(!q.empty()){
        int cur = q.front();
        ret.push_back(cur);
        q.pop();
        for(int i=idx[cur];i<idx[cur+1];++i){
            if(!visit[nodelist[i]]){
                q.push(nodelist[i]);
                visit[nodelist[i]] = true;
            }
        }
    }
    return ret;
}

double* CSRGraph::pPageRank(){
    double* rank = new double[numNode];

    #pragma omp parallel for
    for(int i=0;i<numNode;++i) rank[i] = 1.0/numNode;

    double tol;
    double tmp[numNode];
    do{
        tol = 0;
        double noLink = 0;
        #pragma omp parallel for reduction(+:noLink)
        for(int i=0;i<numNode;++i) {
            if(idx[i+1] == idx[i]) tmp[i]=0;
            else tmp[i] = rank[i]/(idx[i+1]-idx[i]);
        }
        #pragma omp parallel for reduction(+:tol)
        for(int i=0;i<numNode;++i){
            double total_in = 0;
            for(int j=in_idx[i];j<in_idx[i+1];++j) total_in += tmp[in_nodelist[j]];
            double old_rank = rank[i];
            rank[i] = (1-DF)/numNode + DF*(total_in + noLink/numNode);
            tol += std::abs(rank[i]-old_rank);
        }
    } while (tol > TOLERANCE);
    return rank;
}

void CSRGraph::printCSRGraph(bool showGraph, bool showResult, int iter){
    if(showGraph){
        for(int i=0; i < numNode; ++i){
            printf("Node %d: ",i);
            for(int j=idx[i];j<idx[i+1];++j) printf("%d ",nodelist[j]);
            printf("\n");
        }
        printf("\n");

        for(int i=0; i < numNode; ++i){
            printf("Node %d: ",i);
            for(int j=in_idx[i];j<in_idx[i+1];++j) printf("%d ",in_nodelist[j]);
            printf("\n");
        }
        printf("\n");
    }

    Timer t;
    double bfsT=0, dfsT=0, prT=0;
    for(int it=0;it<iter;++it){
        t.Start();
        std::vector<int> bfs = BFS(1);
        t.Stop();
        bfsT += t.Seconds();

        t.Start();
        double* ppagerank = pPageRank();
        t.Stop();
        prT += t.Seconds();

        if(showResult){
            printf("BFS result (from 0): ");
            for(int i=0;i<bfs.size();++i) printf("%d ",bfs[i]);
            printf("\n");

            printf("PageRank result (top 10): ");

            std::pair<double, int> pr[numNode];
            for(int i=0;i<numNode;++i) pr[i] = {ppagerank[i],i};
            sort(pr, pr+numNode, std::greater<std::pair<double,int>>());
            for(int i=0;i<std::min(10,numNode);++i) printf("%d(%lf) ",pr[i].second, pr[i].first);
            printf("\n\n");
        }

        delete[] ppagerank;
    }

    printf("-Benchmark-\n");
    printf("BFS: %lf\nPageRank: %lf\n",bfsT/iter,prT/iter);
}

int main(int argc, char **argv){    
    /*
        input format
        ./CSR [input file path] [show graph (0/1)] [show result(0/1)] [# of iteration (1~9, recomment 1 when showing result)]
    */

    freopen(argv[1],"rt",stdin);

    int a,b,n;
    // scanf("%d",&n);  //# of vertices (0~n-1)
    n = 1024;
    std::vector<Edge> edgelist;
    while(scanf("%d %d",&a,&b)!=-1) edgelist.push_back({a,b});
    CSRGraph g(n,edgelist);
    g.printCSRGraph(*argv[2]-'0', *argv[3]-'0', *argv[4]-'0');
}