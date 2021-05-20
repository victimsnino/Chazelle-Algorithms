#include "Kruskal.h"

#include <algorithm>
#include <iostream>
#include <list>

/* Functions returns weight of the MST*/
namespace Kruskal
{


Graph::Graph(size_t V, size_t E)
{
    this->V = V;
    this->E = E;
}

void Graph::addEdge(size_t u, size_t v, size_t w, size_t index)
{
    edges.push_back({w, u, v, index});
}

std::list<size_t> Graph::kruskalMST()
{ 
    // Sort edges in increasing order on basis of cost
    std::sort(edges.begin(), edges.end(), [](const Edge& left, const Edge& right)
    {
        return left.w < right.w;
    });
  
    // Create disjoint sets
    DisjointSets ds(V);

    std::list<size_t> result{};
  
    // Iterate through all sorted edges
    std::vector<Edge>::iterator it;
    for (it=edges.begin(); it!=edges.end(); it++)
    {
        size_t u = it->u;
        size_t v = it->v;
  
        size_t set_u = ds.find(u);
        size_t set_v = ds.find(v);
  
        // Check if the selected edge is creating
        // a cycle or not (Cycle is created if u
        // and v belong to same set)
        if (set_u != set_v)
        {
            // Current edge will be in the MST
            // so print it
  
            // Update MST weight
            result.push_back(it->index);
  
            // Merge two sets
            ds.merge(set_u, set_v);
        }
    }
  
    return result;
}

DisjointSets::DisjointSets(size_t n)
{
    // Allocate memory
    this->n = n;
    parent  = new size_t[n + 1];
    rnk     = new size_t[n + 1];

    // Initially, all vertices are in
    // different sets and have rank 0.
    for (size_t i = 0; i <= n; i++)
    {
        rnk[i] = 0;

        //every element is parent of itself
        parent[i] = i;
    }
}

size_t DisjointSets::find(size_t u)
{
    /* Make the parent of the nodes in the path
       from u--> parent[u] point to parent[u] */
    if (u != parent[u])
        parent[u] = find(parent[u]);
    return parent[u];
}

void DisjointSets::merge(size_t x, size_t y)
{
    x = find(x), y = find(y);

    /* Make tree with smaller height
       a subtree of the other tree  */
    if (rnk[x] > rnk[y])
        parent[y] = x;
    else // If rnk[x] <= rnk[y]
        parent[x] = y;

    if (rnk[x] == rnk[y])
        rnk[y]++;
}

}