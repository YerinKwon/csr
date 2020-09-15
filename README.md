# csr

## About   
simple csr graph implementation and dfs,bfs,pagerank on it.   
CSR.cpp : implementation.    
input.txt : sample graph with 10 vertices and 30 edges.   

## How to run   
    g++ -std=c++14 CSR.cpp -o CSR
    ./CSR
    
## Result   
Node 1: 2 3 10    
Node 2: 3 4 5 9    
Node 3: 2 8 9    
Node 4: 1 2 5 6 9 10    
Node 5: 3 6 8    
Node 6:    
Node 7: 4 9    
Node 8: 2    
Node 9: 1 2 3 7    
Node 10: 1 3 6 7    

BFS result (from 1): 1 2 3 10 4 5 9 8 6 7    
DFS result (from 1): 1 2 3 8 9 7 4 5 6 10    
PageRank result: 1:0.054959 2:0.144427 3:0.107555 4:0.065127 5:0.054917 6:0.048243 7:0.045733 8:0.061034 9:0.104828 10:0.039798    
