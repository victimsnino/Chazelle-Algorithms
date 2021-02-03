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
extension edge - border edge of min current cost

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

T-tree is based on stack
##### * Retraction (aka pop):
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
##### * Extension (aka push):
  ```python
  border_edge(u,v) = min_current_cost([heap.findmin() for heap in all_heaps]) # extension edge
  def find_fusion_edge():
    for link(a,b) in min_links:
      if working_cost(link) <= cost(u,v) and (a or b) in C[zi] (where i minimal index):
        return (a,b), i
  fusion_edge(a,b), i = find_fusion_edge()
  if fusion_edge:
    def fusion(fusion_edge(a,b), i):
      foreach edge with both endpoints in C[zi+1] U .. U C[zk]:
        contract(edge)
      # let imagine whole C[zi+1] U..U C[zk] contracted to b
      contract(a,b)
    fusion(fusion_edge, i)
  else:
    i = k-1 # ???
  update_relevant_min_links() #O(k^2)
  for j in range(i+1, k+1):
    delete H(j)
    for jj in range(j+1, k+1):
      delete H(j, jj)

  discard_corrupted_edges()
  clusters = create()
  for cluster in clusters:
    (r,s) = cluster.find(edge is min_current_cost) #edge
    cluster.discard_rest_edges((r,s))
    def get_heap():
      if ((r,s) from H(j) and (_, s) from H(j ,jj)) or (r,s) from H(j, jj):
        assert (_, s) from H(j, l) # h < i <= l
        return H(i)
      else: #i (r,s) from H(j)
        assert(_, s) from H(h,j): h < i
        return H(h, j)

    get_heap().insert((r,s))
    for h, j in (h < i < j):
      H(h, i).meld(H(h, j))
    
    for edge in old_border_edges(v):
      delete_from_heaps(edge)
      check_for_minlink(edge)
    for edge in new_border_edges(v):
      H(i,k).insert(cheapest_from_multiple_and_discard_rest(edge))
  ```
  Property:
  * a <= (in chain) u
  * (????) We extend chain by adding single-vertex Czk == v and (u,v) - chain link
  * zk - end of path: without fusion is k +1, with fusion i+1