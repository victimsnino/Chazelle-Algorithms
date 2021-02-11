import numpy as np
import random
import networkx as nx
import matplotlib.pyplot as plt
k = 4

graph = nx.Graph(nx.binomial_tree(k))


edges = list(reversed(list(nx.to_edgelist(graph))))
new_edge_number = len(graph.nodes)
for i,j,_ in edges:
    graph.add_edge(j, new_edge_number)
    new_edge_number += 1

nx.draw_networkx(graph)
plt.show()

edges = list(reversed(list(nx.to_edgelist(graph))))
weights = [i+1 for i in range(len(edges))]

for i,j,_ in edges:
    graph[i][j]['weight'] = weights.pop(0)

matrix = np.array(nx.to_numpy_array(graph))
for i in range(matrix.shape[0]):
    string_to_print ='std::array{ '
    for v in matrix[i,:]:
        string_to_print += f'{str(int(v)):>2}, '
    string_to_print += '},'
    print(string_to_print)