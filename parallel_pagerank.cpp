#include <cmath>
#include <utility>
#include <algorithm>
#include <functional>

#include "csrgraph.h"
#include "timer.h"

#define DF 0.85
#define TOLERANCE 0.0001

double* pPageRank(CSRGraph &g){
    double* rank = new double[g.num_node()];

    #pragma omp parallel for
    for(int i=0;i<g.num_node();++i) rank[i] = 1.0/g.num_node();

    double tol;
    double tmp[g.num_node()];
    do{
        tol = 0;
        double noLink = 0;
        #pragma omp parallel for reduction(+:noLink)
        for(int i=0;i<g.num_node();++i) {
            int outdegree = g.outidx(i+1) - g.outidx(i);
            if(outdegree == 0) noLink += rank[i];
            else tmp[i] = rank[i]/outdegree;
        }
        #pragma omp parallel for reduction(+:tol)
        for(int i=0;i<g.num_node();++i){
            double total_in = 0;
            for(int j=g.inidx(i);j<g.inidx(i+1);++j) total_in += tmp[g.innodelist(j)];
            double old_rank = rank[i];
            rank[i] = (1.0-DF)/g.num_node() + DF*(total_in + noLink/g.num_node());
            tol += std::abs(rank[i]-old_rank);
        }
    } while (tol > TOLERANCE);
    return rank;
}

int main(int argc, char **argv){
    printf("[Pagerank]\n");

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
    double *pagerank;
    for(int i = 0; i<iteration; ++i){
        t.Start();
        pagerank = pPageRank(g);
        t.Stop();
        printf("%lf\n",t.Seconds());
        avg_time += t.Seconds();
    }

    // print result (option)
    if(print_result){
        printf("result (top 10): ");
        std::pair<double, int> pr[g.num_node()];
        for(int i=0;i<g.num_node();++i) pr[i] = {pagerank[i],i};
        std::sort(pr, pr+g.num_node(), std::greater<std::pair<double,int>>());
        for(int i=0;i<std::min(10,g.num_node());++i) printf("%d(%lf) ",pr[i].second, pr[i].first);
        printf("\n");
    }
    // double sum = 0;
    // for(int i=0;i<g.num_node();++i) sum += pagerank[i];
    // printf("%lf\n",sum);

    // benchmark
    printf("Benchmark: %lf\n",avg_time/iteration);
}