#include <vector>
#include <utility>
#include <cstdio>
#include <algorithm>
#include <queue>
#include <stack>
#include <cmath>
#include <memory.h>

#define DF 0.85
#define TOLERANCE 0.0000001
typedef std::pair<int,int> Edge;

struct CSRGraph{
    int numNode, nodelistSize;
    std::vector<int> idx;
    std::vector<int> nodelist;

    CSRGraph(int s, const std::vector<Edge> &EdgeList){
        numNode = s, nodelistSize = EdgeList.size();
        idx.reserve(numNode+1);
        nodelist.reserve(nodelistSize);
        
        idx.push_back(0);
        for(int i=0;i<nodelistSize;++i){
            while(idx.size() <= EdgeList[i].first) idx.push_back(i);
            nodelist.push_back(EdgeList[i].second);
        }
        while(idx.size() <= numNode) idx.push_back(nodelistSize);
    }
    ~CSRGraph(){};

    std::vector<int> BFS(int start);
    std::vector<int> DFS(int start);
    std::vector<double> PageRank();
    void printCSRGraph();
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

std::vector<int> CSRGraph::DFS(int start){
    std::vector<int> ret;
    ret.reserve(numNode);

    std::stack<int> s;
    bool visit[numNode];
    memset(visit,false,numNode);

    s.push(start-1);
    visit[start-1] = true;
    ret.push_back(0);
    while(!s.empty()){
        int cur = s.top();
        bool hasNext = false;
        for(int i=idx[cur];i<idx[cur+1];++i){
            if(!visit[nodelist[i]]){
                ret.push_back(nodelist[i]);
                visit[nodelist[i]] = true;
                s.push(nodelist[i]);
                hasNext = true;
                break;
            }
        }
        if(!hasNext) s.pop();
    }
    return ret;
}

std::vector<double> CSRGraph::PageRank(){
    std::vector<double> rank;
    rank.reserve(numNode);
    for(int i=0;i<numNode;++i) rank.push_back(1.0/numNode);

    double tol;
    double tmp[numNode];
    do{
        tol = 0;
        for(int i=0;i<numNode;++i) tmp[i] = 0;
        for(int i=0;i<numNode;++i){
            tmp[i] += (1-DF)/numNode;
            for(int j=idx[i];j<idx[i+1];++j) tmp[nodelist[j]] += DF*rank[i]/(idx[i+1]-idx[i]);
        }
        for(int i=0;i<numNode;++i) {
            tol += std::abs(rank[i]-tmp[i]);
            rank[i] = tmp[i];
        }
    } while (tol > TOLERANCE);
    return rank;
}

void CSRGraph::printCSRGraph(){
        for(int i=0; i < numNode; ++i){
            printf("Node %d: ",i+1);
            for(int j=idx[i];j<idx[i+1];++j) printf("%d ",nodelist[j]+1);
            printf("\n");
        }
        printf("\n");
        
        std::vector<int> bfs = BFS(1);
        std::vector<int> dfs = DFS(1);
        std::vector<double> pagerank = PageRank(); 

        printf("BFS result (from 1): ");
        for(int i=0;i<bfs.size();++i) printf("%d ",bfs[i]+1);
        printf("\n");

        printf("DFS result (from 1): ");
        for(int i=0;i<bfs.size();++i) printf("%d ",dfs[i]+1);
        printf("\n");

        printf("PageRank result: ");
        for(int i=0;i<numNode;++i) printf("%d:%lf ",i+1,pagerank[i]);
        printf("\n");
    }

int main(){    
    freopen("input.txt","rt",stdin);

    int a,b,n;
    scanf("%d",&n);  //# of vertices (1~n)
    std::vector<Edge> edgelist;
    while(scanf("%d %d",&a,&b)!=-1) edgelist.push_back({a-1,b-1});
    sort(edgelist.begin(),edgelist.end());

    CSRGraph g(n,edgelist);
    g.printCSRGraph();
}