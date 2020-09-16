# csr

## About   
simple csr graph implementation and dfs,bfs,pagerank on it.   
CSR.cpp : implementation.    
input.txt : sample graph with 10 vertices and 30 edges.   

## How to run   
    g++ -std=c++14 CSR.cpp -o CSR
    ./CSR
    
## Result   

### Output Example: 10 vertices 30 edges   
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
PageRank result: 0:0.075637 1:0.198765 2:0.148021 3:0.089630 4:0.075579 5:0.066394 6:0.062939 7:0.083997 8:0.144267 9:0.054772 

-Benchmark-
BFS: 0.000011
DFS: 0.000010
PageRank: 0.000067