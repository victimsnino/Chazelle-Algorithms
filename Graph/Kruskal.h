// C++ program for Kruskal's algorithm to find Minimum
// Spanning Tree of a given connected, undirected and
// weighted graph

#include <list>
#include <utility>
#include <vector>

// Creating shortcut for an integer pair
namespace Kruskal
{
struct Edge
{
    size_t w;
    size_t u;
    size_t v;
    size_t index;
};
  
// Structure to represent a graph
struct Graph
{
    size_t               V, E;
    std::vector<Edge> edges;
  
    // Constructor
    Graph(size_t V, size_t E);

    // Utility function to add an edge
    void addEdge(size_t u, size_t v, size_t w, size_t index);

    // Function to find MST using Kruskal's
    // MST algorithm
    std::list<size_t>kruskalMST();
};
  
// To represent Disjoint Sets
struct DisjointSets
{
    size_t *parent, *rnk;
    size_t n;
  
    // Constructor.
    DisjointSets(size_t n);

    // Find the parent of a node 'u'
    // Path Compression
    size_t find(size_t u);

    // Union by rank
    void merge(size_t x, size_t y);
};
}