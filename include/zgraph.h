//
// Created by ran on 2024/1/25.
//

#ifndef INTERPRETER_COLLECTION_GRAPH_GRAPH_H_
#define INTERPRETER_COLLECTION_GRAPH_GRAPH_H_

#include "hashtable/hash_table.h"
#include "hashtable/hash_set.h"
#include "list/array_list.h"
#include "list/linked_list.h"
#ifdef __cplusplus
extern "C" {
#endif
#define VERTEX_EXISTS (-1)
#define FROM_VERTEX_NOT_EXISTS (-2)
#define TO_VERTEX_NOT_EXISTS (-3)
#define SELF_LOOP (-5)

#define EDGE_EXISTS (-1)
#define GRAPH_ERROR (-4)
#define GRAPH_SUCCESS 1

typedef void *GraphData;
typedef struct Vertex Vertex;
typedef struct Edge Edge;
typedef struct Graph Graph;

/**
 * for iteration graph
 */
typedef struct VertexEntry {
  int *id_list;
  int size;
} VertexEntry;

Graph *create_graph(int directed, int weighted);
/**
 * add a data in graph.
 *
 * @param g
 * @param data
 * @return      id (>0) that represent this data if successful.
 */
int add_graph_data(Graph *g, GraphData data);

/**
 * add a data in graph with designated id
 * @param g
 * @param id
 * @param data
 * @return       id that represent this data if successful.
 */
int add_graph_data_with_id(Graph *g, int id, GraphData data);

int add_edge(Graph *g, int from, int to, int weight);

Edge *get_edge(Graph *graph, int from, int to);

void set_weight(Graph *graph, int from, int to, int weight);

void free_graph(Graph *graph);

int vertex_count(Graph *graph);

int edge_count(Graph *graph);

int is_vertex_connected(Graph *graph, int id1, int id2);

int remove_vertex(Graph *graph, int id);

int remove_edge(Graph *graph, int from, int to);

Hashset *get_adj_set(Graph *graph, int id);

void free_vertex_entry(VertexEntry *);

int has_vertex(Graph *graph, int id);

int degree_of(Graph *graph, int id);

int in_degree_of(Graph *graph, int id);

int out_degree_of(Graph *graph, int id);

/**
 * reverse edges of the graph. if the graph is undirected, return it self.Otherwise create a new reversed graph
 *
 * @param graph
 * @return
 */
Graph *reverse_graph(Graph *graph);

int get_edge_to(Edge *edge);
int get_edge_from(Edge *edge);
int get_edge_weight(Edge *edge);
// ----------Algorithms------------------
VertexEntry *dfs_graph(Graph *graph);

/**
 * dfs non-recursive
 *
 * @param graph
 * @return
 */
VertexEntry *dfs_graph_nr(Graph *graph);

int component_count(Graph *graph);

/**
 * components of the graph. Hashtable of <int, array_list> where key is component id and value is vertex id list
 *
 * @param graph
 * @return
 */
Hashtable *graph_components(Graph *graph);

/**
 * single source path. Hashtable of <int,int> where key is the vertex and value is its parent vertex.
 *
 * @param graph
 * @param s
 * @return
 */
typedef enum {
  BFS, DFS
} GraphOrd;
/**
 * return a map of <id,pid>. if choose BFS, the map can be used for generate a shortest path for
 * unweighted graph.
 *
 * @param graph
 * @param s
 * @param ord
 * @return
 */
Hashtable *single_source_path(Graph *graph, int s, GraphOrd ord);
/**
 * generate a path from source to dest
 *
 * @param premap
 * @param to
 * @return
 */
LinkedList *single_source_path_to(Hashtable *premap, int to);

/**
 * test if 2 vertex has a path
 *
 * @param graph
 * @param v1
 * @param v2
 * @return
 */
int has_path(Graph *graph, int v1, int v2);

/**
 * Return one path between vertexes. Note that the path is not necessary a shortest one.
 * @param graph
 * @param v1
 * @param v2
 * @return
 */
LinkedList *one_path(Graph *graph, int v1, int v2);

/**
 * test if a graph has circle.
 * for a undirected graph:
 * - current visited vertex V1
 * - V1 has a adjacent vertex V2
 * - V2 is visited and not the parent vertex of V1
 *
 * for a directed graph:
 * - additional condition: all vertex that visited should on the same "path"
 *
 * @param graph
 * @return
 */
int has_circle(Graph *graph);

/**
 * enumerate paths that are circle of the graph
 *
 * @return
 */
ArrayList *enumerate_circle_path(Graph *graph);

/**
 * detect if the graph is a bipartite graph
 *
 * @param graph
 * @return
 */
int is_bipartite(Graph *graph);

VertexEntry *bfs_graph(Graph *graph);

/**
 * find bridges of the graph.
 * for undirected graph:
 *  - a bridge is an edge whose removal increases the number of connected components
 * for directed graph:
 *  - a bridge is an edge whose removal increases the number of strongly connected components
 *
 * @return a list of edges
 */
LinkedList *find_bridge(Graph *graph);

/**
 * find cut points of the graph.
 * for undirected graph:
 * - a cut points is a vertex whose removal changes the number of connected components.
 *
 * @param graph
 * @return
 */
LinkedList *find_cut_point(Graph *graph);

/**
 * get a hamilton loop path of the graph.
 *
 * @param graph
 * @return
 */
LinkedList *hamilton_loop_path(Graph *graph);

/**
 * get a hamilton path from start
 *
 * @param graph
 * @param start
 * @return
 */
LinkedList *hamilton_path(Graph *graph, int start);

/**
 * test if the graph has a euler loop
 *
 * @param graph
 * @return
 */
int has_euler_loop(Graph *graph);

/**
 * find a path with hierholzer algorithm if the graph has a euler loop
 *
 * @param graph
 * @return
 */
LinkedList *hierholzer_euler_loop(Graph *graph);

/**
 * kruskal minimum spanning tree algorithm
 *
 * @param graph
 * @return
 */
LinkedList *kruskal_mst(Graph *graph);

/**
 * Prim minimum spanning tree algorithm
 *
 * @param graph
 * @return
 */
LinkedList *prim_mst(Graph *graph);

/**
 * Dijkstra algorithm for shortest path of weighted graph with no negative edges.
 * return a map of <id,dis>
 * @param graph
 * @return
 */
Hashtable *dijkstra(Graph *graph, int s);

/**
 * return a map of <id,dis> and create a map of <id, pre_id> with which can
 * construct a shortest path. O(ElogE)
 *
 * @param graph
 * @param s
 * @param pre_map
 * @return
 */
Hashtable *dijkstra_path(Graph *graph, int s, Hashtable **pre_map);

/**
 * get a path of shortest path from source to target
 *
 * @param graph
 * @param s
 * @param t
 * @return
 */
LinkedList *dijkstra_path_to(Graph *graph, int s, int t);

/**
 * Bellman-Ford algorithm. Get the shortest path of weighted graph with negative edges.
 *
 * O(V*E)
 * @param graph
 * @param s
 * @return
 */
Hashtable *bellman_ford(Graph *graph, int s);

/**
 * get a path of shortest path from source to target with bellman-ford algorithm.
 *
 * @param graph
 * @param s
 * @param t
 * @return
 */
LinkedList *bellman_ford_path_to(Graph *graph, int s, int t);

/**
 * Floyed algorithm. Get the shortest path of weighted graph with negative edges.
 * O(V^3)
 *
 * @param graph
 * @return
 */
Hashtable *floyd(Graph *graph, int *has_negative_circle);

/**
 * To reconstruct the actual shortest path between two vertices using the Floyd-Warshall algorithm,
 * we need to maintain an additional 2D array to keep track of the intermediate vertices on the shortest paths.
 * This array will store the next vertex to go to on the shortest path from vertex i to vertex j.
 *
 * @param graph
 * @param has_negative_circle
 * @param next_matrix
 * @return
 */
Hashtable *floyd_path(Graph *graph, int *has_negative_circle, Hashtable **next_matrix);

/**
 * Reconstruct the path with next 2D array.
 *
 * @param next_matrix
 * @param source
 * @param to
 * @return
 */
LinkedList *floyd_path_to(Hashtable *next_matrix, int source, int to);

/**
 * get the topological sort of the graph. if this graph is not a DAG, return null
 * O(V+E)
 *
 * @param graph
 * @return
 */
LinkedList *topological_sort(Graph *graph);

/**
 * get strongly connected components of a directed graph.
 * O(V+E)
 *
 * @return
 */
Hashtable *scc_kosaraju(Graph *graph);

/**
 * Max flow of the graph with given source and to.
 * Use Edmonds-Karp algorithm. return a map of the flow of each edge.
 * O(V*E^2)
 *
 * @param graph
 * @param source
 * @param to
 * @param max_flow
 * @return
 */
Hashtable *max_flow(Graph *graph, int source, int to, int *max_flow);

/**
 * bipartite matching using max flow algorithm.
 *
 * @param graph
 * @return
 */
int bipartite_matching(Graph* graph);

/**
 * bipartite matching using hungarian algorithm.
 *
 * @param graph
 * @return
 */
int hungarian_matching(Graph* graph);
#ifdef __cplusplus
}
#endif
#endif //INTERPRETER_COLLECTION_GRAPH_GRAPH_H_
