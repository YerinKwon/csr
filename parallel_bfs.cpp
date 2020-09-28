#include "csrgraph.h"
#include "sliding_q.h"
#include "bitmap.h"
#include "timer.h"

int topDown(CSRGraph &g,int* parent, SlidingQ &sq){
    int outcnt = 0;
    #pragma omp parallel
    {
        Qbuffer lq(sq);
        #pragma omp for reduction(+: outcnt)
        for(int i = sq.start(); i<sq.end(); ++i){
            int u = sq.get(i);
            for(int v:g.out_neigh(u)){
                int cur_val = parent[v];
                if(cur_val < 0){
                    if(__sync_bool_compare_and_swap(&parent[v], cur_val, u)){
                        lq.push_back(v);
                        outcnt += -cur_val;
                    }
                }
            }
            // for(int j = g.outidx(u);j<g.outidx(u+1);++j){ //HERE
            //     int v = g.outnodelist(j);
            //     int cur_val = parent[v];
            //     if(cur_val < 0){
            //         if(__sync_bool_compare_and_swap(&parent[v], cur_val, u)){
            //             lq.push_back(v);
            //             outcnt += -cur_val;
            //         }
            //     }
            // }
        }
        lq.flush();
    }
    return outcnt;
}

int bottomUp(CSRGraph &g,int* parent, Bitmap &prev, Bitmap &cur){
    int awakecnt = 0;
    cur.reset();
    #pragma omp parallel for reduction(+: awakecnt)
    for(int u=0; u<g.num_node(); ++u){
        if(parent[u] < 0){
            for(int j = g.inidx(u); j < g.inidx(u+1); ++j){ //HERE
                int v = g.innodelist(j);
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

void QToBitmap(SlidingQ &sq, Bitmap &bm){
    #pragma omp parallel for
    for(int i=sq.start(); i<sq.end(); ++i) {
        int n = sq.get(i);
        bm.setBitAtomic(n);
    }
}

void BitmapToQ(CSRGraph &g,Bitmap &bm, SlidingQ &sq){
    #pragma omp parallel
    {
        Qbuffer lq(sq);
        #pragma omp for
        for(int i=0;i<g.num_node();++i)
            if(bm.isSet(i)) lq.push_back(i);
        lq.flush();
    }
    sq.slide();
}

int* pBFS(CSRGraph &g ,int start, int alpha = 15, int beta = 18){
    int* parent = new int[g.num_node()];
    #pragma omp parallel for
    for(int i=0;i<g.num_node();++i){
        int cnt = g.outidx(i+1)-g.outidx(i);
        parent[i] = cnt > 0 ? -cnt:-1;
    }

    parent[start] = start;
    SlidingQ sq(g.num_node());
    sq.push_back(start);
    sq.slide();
    
    Bitmap prev(g.num_node()), cur(g.num_node());
    prev.reset(), cur.reset();

    int remainingEdge = g.nodelist_size();
    int outcnt = g.outidx(start+1)-g.outidx(start);

    while(!sq.empty()){
        if(outcnt > remainingEdge / alpha){
            // BU
            QToBitmap(sq,prev);
            int prevawakecnt, awakecnt = sq.size();
            sq.slide();
            do{
                prevawakecnt = awakecnt;
                awakecnt = bottomUp(g, parent, prev, cur);
                prev.swap(cur);
            }while( awakecnt >= prevawakecnt || awakecnt > g.num_node()/beta );
            BitmapToQ(g, prev,sq);
            outcnt = 1;
        }else{
            // TD
            remainingEdge -= outcnt;
            outcnt = topDown(g, parent, sq);
            sq.slide();
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
    bool print_graph = *argv[2] - '0', print_result =  *argv[3] - '0';
    int iteration = *argv[4] - '0';

    // graph generation
    int a,b,n;
    scanf("%d",&n);  //# of vertices (0~n-1)
    std::vector<Edge> edgelist;
    while(scanf("%d %d",&a,&b)!=-1) edgelist.push_back({a,b});
    CSRGraph g(n,edgelist);
    
    // print graph (option)
    if(print_graph) g.printCSRGraph();
    
    // run pagerank
    Timer t;
    double avg_time = 0;
    int *bfs;
    for(int i = 0; i<iteration; ++i){
        t.Start();
        bfs = pBFS(g,0);
        t.Stop();
        printf("%lf\n",t.Seconds());
        delete bfs;
        avg_time += t.Seconds();
    }

    // print result (option)
    if(print_result){
        printf("result (from 0): ");
        for(int i=0;i<g.num_node();++i) printf("%d ",bfs[i]);
        printf("\n");
    }

    // benchmark
    printf("Benchmark: %lf\n",avg_time/iteration);
}