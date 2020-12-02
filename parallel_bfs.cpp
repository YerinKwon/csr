#include <cstdlib>

#include "csrgraph.h"
#include "sliding_q.h"
#include "bitmap.h"
#include "timer.h"

typedef int32_t Node;
typedef std::pair<Node, Node> Edge;

int64_t topDown(CSRGraph<Node> &g,Node* parent, SlidingQ<Node> &sq){
    int64_t outcnt = 0;
    #pragma omp parallel
    {
        Qbuffer<Node> lq(sq);
        #pragma omp for reduction(+: outcnt)
        for(auto i = sq.begin(); i != sq.end(); ++i){
            Node u = *i;
            for(auto j = g.out_idx(u), jend = g.out_idx(u+1); j != jend; ++j){
                Node v = *j;
                Node cur_val = parent[v];
                if(cur_val < 0){
                    if(__sync_bool_compare_and_swap(&parent[v], cur_val, u)){
                        lq.push_back(v);
                        outcnt += -cur_val;
                    }
                }
            }
        }
        lq.flush();
    }
    return outcnt;
}

int64_t bottomUp(CSRGraph<Node> &g,Node* parent, Bitmap &prev, Bitmap &cur){
    int64_t awakecnt = 0;
    cur.reset();
    #pragma omp parallel for reduction(+: awakecnt) schedule(dynamic, 1024)
    for(Node u=0; u<g.num_node(); ++u){
        if(parent[u] < 0){
            for(auto j = g.in_idx(u),jend = g.in_idx(u+1); j != jend; ++j){
                Node v = *j;
                if(prev.isSet(v)){
                    parent[u] = v;
                    awakecnt++;
                    cur.setBit(u);
                    break;
                }
            }
        }
    }
    return awakecnt;
}

void QToBitmap(SlidingQ<Node> &sq, Bitmap &bm){
    #pragma omp parallel for
    for(auto i=sq.begin(); i<sq.end(); ++i) {
        Node n = *i;
        bm.setBitAtomic(n);
    }
}

void BitmapToQ(CSRGraph<Node> &g,Bitmap &bm, SlidingQ<Node> &sq){
    #pragma omp parallel
    {
        Qbuffer<Node> lq(sq);
        #pragma omp for
        for(Node i=0;i<g.num_node();++i)
            if(bm.isSet(i)) lq.push_back(i);
        lq.flush();
    }
    sq.slide();
}

int* pBFS(CSRGraph<Node> &g ,Node start, int alpha = 15, int beta = 18){
    printf("Source:\t\t%d\n",start);
    Timer t;
    t.Start();
    Node* parent = new Node[g.num_node()];
    #pragma omp parallel for
    for(Node i=0;i<g.num_node();++i){
        parent[i] = g.out_degree(i) != 0 ? -g.out_degree(i) : -1;
    }
    t.Stop();
    printf("i\t\t%lf\n",t.Seconds());

    parent[start] = start;
    SlidingQ<Node> sq(g.num_node());
    sq.push_back(start);
    sq.slide();
    
    Bitmap prev(g.num_node()), cur(g.num_node());
    prev.reset(), cur.reset();

    int remainingEdge = g.nodelist_size();
    int outcnt = g.out_degree(start);

    while(!sq.empty()){
        if(outcnt > remainingEdge / alpha){
            // BU
            t.Start();
            QToBitmap(sq,prev);
            t.Stop();
            printf("e\t\t%lf\n",t.Seconds());
            int prevawakecnt, awakecnt = sq.size();
            sq.slide();
            do{
                t.Start();
                prevawakecnt = awakecnt;
                awakecnt = bottomUp(g, parent, prev, cur);
                prev.swap(cur);
                t.Stop();
                printf("bu\t%d\t%lf\n",awakecnt,t.Seconds());
            }while( awakecnt >= prevawakecnt || awakecnt > g.num_node()/beta );
            t.Start();
            BitmapToQ(g, prev,sq);
            t.Stop();
            printf("c\t\t%lf\n",t.Seconds());
            outcnt = 1;
        }else{
            // TD
            t.Start();
            remainingEdge -= outcnt;
            outcnt = topDown(g, parent, sq);
            sq.slide();
            t.Stop();
            printf("td\t%d\t%lf\n",sq.size(),t.Seconds());
        }
    }

    #pragma omp parallel for
    for(int i=0;i<g.num_node();++i)
        if(parent[i] < -1) parent[i] = -1;

    return parent;
}

int main(int argc, char **argv){    
    printf("[BFS]\n");

    // command line parsing
    freopen(argv[1],"rt",stdin);
    int iteration = atoi(argv[2]);

    // graph generation
    int a,b;
    std::vector<Edge> edgelist;
    while(scanf("%d %d",&a,&b)!=-1) edgelist.push_back({a,b});
    CSRGraph<Node> g(std::move(edgelist));
    
    // run pagerank
    Timer t;
    double avg_time = 0, max_time=0, min_time=99999;
    int *bfs;
    for(int i = 0; i<iteration; ++i){
        t.Start();
        bfs = pBFS(g,0);
        t.Stop();
        printf("Trial Time:\t%lf\n",t.Seconds());
        delete bfs;
        avg_time += t.Seconds();
        max_time = std::max(max_time, t.Seconds());
        min_time = std::min(min_time, t.Seconds());
    }

    // benchmark
    printf("Min time: %lf\n", min_time);
    printf("Max time: %lf\n", max_time);
    printf("Benchmark: %lf\n",avg_time/iteration);
}