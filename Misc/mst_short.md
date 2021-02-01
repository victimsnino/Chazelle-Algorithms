C of G contractible - intersection with MST(G) is connected

### T - tree:
* root - whole graph as a one node
* lefs - separate nodes
* height d
* Cz - subgraph
  * nz - number of verticies


### z1,...,zk = z - active path

**border edge** - edge with only ONE vertex from Cz1 U ... Czk
* can be corrupted
* bad - corrupted from Cz that contracted

### Cost:
* Original
* Current (in soft heap, probably raised)
* working - current if bad, else original

### Soft heaps:
* H(i) - one per Czi
* H(i,j) - one per collection of 0 <= i < j ??
* One edge only in one heap!
* Error rate 1/c

### Algorithm: msf(G, t)
1,2) Boruvka phase __c__ times
3) Building the Hierarchy T

### Step 1,2 Boruvka


### Step 3:
* nz **target** size of Cz = S(t-1, S(t, dzk-1))^3, 
* dzk - height of z.

cluster = subset of edges that share the same endpoint outside of chain

z = z1,...,zk - connected via chain of edges  (chain-links)
__discards__ edges  - removed from consideration

* Property #1: (Propery of chain-links - decreasing)
```python 
for any i < k chain_link[i, i+1] = edge has properties:
```
  * current_cost >= any border edge of Cz1 U .. U Czi
  * current_cost < working_cost any chain_link[<i, <i] (Probably we need to maintain min-link every-time for it)
    
* Property #2 (about where edges in heaps)
``` python
for j in range()???:
    for u, v in border_edges(Czj):
        assert(u in Czj)
        in_heap_j = (u,v) in H(j)
        in_any_heap_i_j = (u,v) in any ONE of H(i,j) for i in range(j)

        assert(in_heap_j or in_any_heap_i_j)
        
        if in_heap_j:
            exist edge(_, v) in H(i,j)
        else: #if in_any_heap_i_j:
            assert(v in Czi)
            for l in range(i+1, j):
                v not in Czl # (probably or guaranteed?)

```

T is based on stack
* Retraction (aka pop):
  ```python
  assert(k >= 2)
  # reaches max available size
  assert(nzk == S(t-1, S(t, dzk-1))**3) #if dzk == 1, then compare with S(t,1)**3 = 8
  contracted_vertex = C[zk]->contract()
  edges = C[zk-1] <- contracted_vertex
  assert(exists edge from edges is chain-link(C[zk-1], C[zk]))
  assert(all edges not contracted)
  active_path.pop()
  #####
  delete H(k) and H(k-1, k)
  discard_corrupted_edges()
  clusters = create()
  for cluster in clusters:
    (r,s) = cluster.find(edge is min_current_cost) #edge
    cluster.discard_rest_edges((r,s))
    def get_heap():
      if ((r,s) from H(k) and (_, s) from H(k-1, k)) or (r,s) from H(k-1, k):
        assert (_, s) from H(i, k-1)
        return H(k-1)
      else: #i (r,s) from H(k)
        assert(_, s) from H(i,k): i < k -1
        return H(i, k)

    get_heap().insert((r,s))
    for i in range(k-1):
      H(i, k-1).meld(H(i, k))
  ```
  Properties:
  * By inserting (r,s) to H(i,k) - it is at least second edge pointing to s there
  * H(i,k) melded to H(i, k-1) -> we can insert to H(k-1) instead of H(i,k), but better not
