#include <cmath>
#include <utility>
#include <algorithm>
#include <functional>
#include <cstdlib>

#include "csrgraph.h"
#include "timer.h"

typedef int32_t Node;
typedef std::pair<Node, Node> Edge;

const float DF = 0.85;
const double TOLERANCE = 0.0001;

float* pPageRank(CSRGraph<Node> &g){
    float init_score = 1.0f / g.num_node();
    float base_score = (1.0f-DF) / g.num_node();

    Timer t;
    t.Start();
    float* rank = new float[g.num_node()];
    float* tmp = new float[g.num_node()];

    #pragma omp parallel for
    for(Node i=0;i<g.num_node();++i) rank[i] = init_score;
    t.Stop();
    // printf("init\t%lf\n",t.Seconds());

    for(int iter = 0; iter < 20; ++iter){ //default max iter : 20
        double tol = 0;
        t.Start();
        #pragma omp parallel for
        for(Node i=0;i<g.num_node();++i)
            tmp[i] = rank[i]/g.out_degree(i);
        t.Stop();
        // printf("base\t%lf\n",t.Seconds());

        t.Start();
        #pragma omp parallel for reduction(+:tol) schedule(dynamic, 64)
        for(Node i=0;i<g.num_node();++i){
            float total_in = 0;
            // for(Node *it = g.in_idx(i), *end = g.in_idx(i+1); it != end; ++it){
            //     total_in += tmp[*it];
            // }
            for(Node v: g.in_neigh(i))
                total_in += tmp[v];
            float old_rank = rank[i];
            rank[i] = base_score + DF*total_in;
            tol += std::fabs(rank[i]-old_rank);
        }
        t.Stop();
        // printf("calc\t%lf\n",t.Seconds());
        printf(" %2d    %lf\n", iter, tol);
        if(tol < TOLERANCE) break;
    }
    delete[] tmp;
    return rank;
}

int main(int argc, char **argv){
    printf("[Pagerank]\n");

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
    float *pagerank;
    for(int i = 0; i<iteration; ++i){
        t.Start();
        pagerank = pPageRank(g);
        t.Stop();
        printf("Trial Time: \t%lf\n",t.Seconds());
        avg_time += t.Seconds();
        max_time = std::max(max_time, t.Seconds());
        min_time = std::min(min_time, t.Seconds());
    }

    // print result (option)
    // if(print_result){
    //     printf("result (top 10): ");
    //     std::pair<float, int> pr[g.num_node()];
    //     for(int i=0;i<g.num_node();++i) pr[i] = {pagerank[i],i};
    //     std::sort(pr, pr+g.num_node(), std::greater<std::pair<double,int>>());
    //     for(int i=0;i<10;++i) printf("%d(%lf) ",pr[i].second, pr[i].first);
    //     printf("\n");
    // }

    // benchmark
    printf("Min time: %lf\n", min_time);
    printf("Max time: %lf\n", max_time);
    printf("Benchmark: %lf\n",avg_time/iteration);
}