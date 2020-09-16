# csr

## About   
Naive csr graph implementation and dfs,bfs,pagerank on it.   
CSR.cpp : implementation.    
timer.h : simple timer for benchmark. (from https://github.com/sbeamer/gapbs)   
input.txt : sample graph with 10 vertices and 30 edges.   

## How to run   
    g++ -std=c++14 CSR.cpp -o CSR   
    ./CSR [input file path] [show graph (0/1)] [show result (0/1)] [iteration (int)]   
show graph: print each node line by line with its neighbors.   
show result: print the result of each algorithm.   
iteration: number of iteration for benchmark. (Recommend to set 1 when show result is 1)   
    
## Result   

### Output Example: 10 vertices 30 edges   
    ./CSR input.txt 1 1 1   

Node 0: 1 2 9    
Node 1: 2 3 4 8    
Node 2: 1 7 8    
Node 3: 0 1 4 5 8 9    
Node 4: 2 5 7    
Node 5:    
Node 6: 3 8    
Node 7: 1    
Node 8: 0 1 2 6    
Node 9: 0 2 5 6    

BFS result (from 0): 0 1 2 9 3 4 8 7 5 6    
DFS result (from 0): 0 1 2 7 8 6 3 4 5 9    
PageRank result (top 10): 1(0.198765) 2(0.148021) 8(0.144267) 3(0.089630) 7(0.083997) 0(0.075637) 4(0.075579) 5(0.066394) 6(0.062939) 9(0.054772)    

-Benchmark-   
BFS: 0.000012   
DFS: 0.000010   
PageRank: 0.000047   

### Benchmark
Benchmark and comparison with gapbs.

*num node : 1024    
*10 iteration    
        					
avg degree of node | 2 | 10 | 100 | 200 | 500 | 1000   
---|---|---|---|---|---|---   
bfs (gapbs)	| 0.00029 | 0.00024 | 0.00019 | 0.00017 | 0.00009 | 0.0001   
pagerank (gapbs) | 0.00036 | 0.00041 | 0.00024 | 0.0003 | 0.00036 | 0.00042   
bfs (naive) | 0.00007 | 0.000119 | 0.000442 | 0.000802 | 0.00188 | 0.003661   
pagerank (naive) | 0.001302 | 0.001582 | 0.006739 | 0.01136 | 0.023237 | 0.027871   
dfs (naive) | 0.00009 | 0.000157 | 0.000662 | 0.001224 | 0.002888 | 0.005695   