//
// Created by ran on 2024/1/25.
//
#include "zgraph.h"
#include "hashtable/hash-int.h"
#include "hashtable/compare-int.h"
#include "queue/dqueue_ll.h"
#include "unionfind/union_find.h"
#include "queue/priority_queue.h"
#include <assert.h>
#include <stdlib.h>
#include <limits.h>

struct Vertex {
  int id;
  GraphData data;
};

struct Edge {
  int weight;
  int from;
  int to;
};

struct Graph {
  int last_continuous_id;
  int vertex_size; // amount of vertex
  int edge_size; // amount of edge
  int directed; // 1 if graph is directed, otherwise 0
  int weighted; // i if graph is weighted, otherwise 0
  Hashtable *represent; // <id:int*,vertex:Vertex*> map of all vertexes
  Hashtable *edges; // <id:int*, hashset<Edge*>> map
  Hashtable *in_degree; // in degree map
  Hashtable *out_degree; // out degree map
};

static unsigned int default_vertex_hash_func(void *v);
static int default_vertex_equal_func(void *v1, void *v2);
static unsigned int default_edge_hash_func(void *);
static int default_edge_equal_func(void *e1, void *e2);
static Edge *create_edge(int from, int to, int weight);
static void free_edges(Hashtable *edges);
static int add_vertex(Graph *g, Vertex *v);
static int next_id(Graph *g);
static int add_edge_undirected_unweighted(Graph *g, int from, int to);
static int add_edge_undirected_weighted(Graph *g, int from, int to, int weight);

static int add_edge_directed_unweighted(Graph *g, int from, int to);
static int add_edge_directed_weighted(Graph *g, int from, int to, int weight);
static Hashset *get_or_create_adj_set(Graph *g, Vertex *from_vertex);

static int remove_edges_from(Hashset *hashset, int from_id);
static int remove_edges_to(Hashset *hashset, int to_id);
static int remove_edge_from_to(Hashset *hashset, int from_id, int to_id);

static void dfs(Graph *graph, Hashset *visited, int id, VertexEntry *vertex_entry);
static int *new_id(int value);
static VertexEntry *new_vertex_entry(int v_size);
static void dfs_nr(Graph *graph, Hashset *visited, int id, VertexEntry *vertex_entry);
static void dfs_visit(Graph *graph, Hashset *visited, int id);
static void dfs_cid(Graph *graph, Hashtable *visited, int id, int cid);
static void dfs_par(Graph *graph, Hashtable *par, int id, int pid);
static int dfs_cmp(Graph *graph, Hashtable *par, int id, int pid, int target);

static int has_circle_undirected_graph(Graph *graph);
static int has_circle_directed_graph(Graph *graph);
static int dfs_circle_test_undirected(Graph *graph, Hashtable *visited, int id, int pid);
static int dfs_circle_test_directed(Graph *graph, Hashtable *visited, Hashset *on_path, int id, int pid);
static void ud_circle_path(Graph *graph,
                           Hashset *visited,
                           Hashtable *path_visited,
                           int id,
                           int pid,
                           ArrayList *result);
static void d_circle_path(Graph *graph, Hashset *visited, Hashtable *on_path, int id, int pid, ArrayList *result);
static ArrayList *truncate_list(ArrayList *list, int start_element);
static LinkedList *track_path(Hashtable *path, int last_element, int start_element);

static int dfs_bipartite_test(Graph *graph, Hashtable *visited, int id, int color);
static void bfs(Graph *graph, Hashset *visited, int id, VertexEntry *vertex_entry);
static void bfs_par(Graph *graph, Hashtable *par, int id, int pid);
static int bfs_cmp(Graph *graph, Hashtable *visited, int id, int pid, int target);
static void find_bridge_ud(Graph *graph,
                           Hashset *visited,
                           Hashtable *ord,
                           Hashtable *low,
                           int visited_count,
                           int id,
                           int pid,
                           LinkedList *result);
static void find_cut_point_ud(Graph *graph,
                              Hashset *visited,
                              Hashtable *ord,
                              Hashtable *low,
                              int visited_count,
                              int id,
                              int pid,
                              LinkedList *result);
static int dfs_hamilton_loop_path(Graph *graph, Hashtable *visited, int start, int *end, int id, int pid);
static int dfs_hamilton_path(Graph *graph, Hashtable *visited, int *end, int id, int pid);
static int pick_one_id(Graph *graph);
static Edge *pick_one_edge(Hashset *edge);
static Hashtable *copy_edges(Graph *graph);
static int edge_weight_compare(void *e1, void *e2);
static int edge_weight_compare_pq(void *e1, void *e2);
// for dijkstra algorithm in priority queue
typedef struct DIS DIS;
static DIS *create_dis(int id, int dis);
static int dis_compare_pq(DIS *d1, DIS *d2);
static Graph *create_residual_graph(Graph *graph);
static LinkedList *get_augmenting_path(Graph *rg, int s, int t);
static int bfs_hungarian(Graph *graph, Hashtable *matching, int id);
// ------------------Graph operations-----------------------------
int add_graph_data(Graph *g, GraphData data) {
  int id = next_id(g);
  return add_graph_data_with_id(g, id, data);
}

int add_graph_data_with_id(Graph *g, int id, GraphData data) {
  Vertex *vertex = malloc(sizeof(Vertex));
  if (!vertex) return GRAPH_ERROR;
  vertex->data = data;
  vertex->id = id;
  return add_vertex(g, vertex);
}

Graph *create_graph(int directed, int weighted) {
  Graph *g = malloc(sizeof(Graph));
  if (!g) return NULL;
  Hashtable *r = new_hash_table(int_hash, int_compare);
  if (!r) {
    free(g);
    return NULL;
  }
  Hashtable *edges = new_hash_table(int_hash, int_compare);
  if (!edges) {
    free(g);
    free(r);
    return NULL;
  }
  g->vertex_size = 0;
  g->edge_size = 0;
  g->directed = directed;
  g->weighted = weighted;
  g->represent = r;
  g->edges = edges;
  g->last_continuous_id = 0;
  if (directed) {
    g->in_degree = new_hash_table(int_hash, int_compare);
    g->out_degree = new_hash_table(int_hash, int_compare);
  }
  return g;
}

int add_vertex(Graph *g, Vertex *v) {
  if (contains_in_hash_table(g->represent, &v->id)) {
    free(v);
    return VERTEX_EXISTS;
  }
  int ret = put_hash_table(g->represent, &v->id, v);
  if (ret == 0) return GRAPH_ERROR;
  g->vertex_size++;
  return v->id;
}

int add_edge(Graph *g, int from, int to, int weight) {
  // self loop is not supported
  if (from == to) {
    return SELF_LOOP;
  }
  if (g->directed) {
    if (g->weighted) {
      // directed weighted
      return add_edge_directed_weighted(g, from, to, weight);
    } else {
      return add_edge_directed_unweighted(g, from, to);
    }
  } else {
    if (g->weighted) {
      return add_edge_undirected_weighted(g, from, to, weight);
    } else {
      return add_edge_undirected_unweighted(g, from, to);
    }
  }
}

Edge *get_edge(Graph *graph, int from, int to) {
  Hashset *adj = get_adj_set(graph, from);
  if (!adj) return NULL;
  Edge e = {.from=from, .to=to};
  return get_key_in_hash_set(adj, &e);
}

void set_weight(Graph *graph, int from, int to, int weight) {
  assert(graph->weighted);
  Edge *edge = get_edge(graph, from, to);
  assert(edge != NULL);
  edge->weight = weight;
  if (!graph->directed) {
    edge = get_edge(graph, to, from);
    edge->weight = weight;
  }
}

int vertex_count(Graph *graph) {
  return graph->vertex_size;
}

int edge_count(Graph *graph) {
  return graph->edge_size;
}

int is_vertex_connected(Graph *g, int id1, int id2) {
  Vertex *from_vertex = get_hash_table(g->represent, &id1);
  if (!from_vertex) {
    return 0;
  }
  Vertex *to_vertex = get_hash_table(g->represent, &id2);
  if (!to_vertex) {
    return 0;
  }
  Hashset *adj_set = get_adj_set(g, id1);
  if (adj_set == NULL) return 0;
  Edge d = {.from=id1, .to=id2};
  return contains_in_hash_set(adj_set, &d);
}

void free_graph(Graph *graph) {
  if (graph) {
    register_hashtable_free_functions(graph->represent, NULL, free);
    free_hash_table(graph->represent);

    free_edges(graph->edges);
    if (graph->directed) {
      register_hashtable_free_functions(graph->in_degree, free, free);
      register_hashtable_free_functions(graph->out_degree, free, free);
      free_hash_table(graph->in_degree);
      free_hash_table(graph->out_degree);
    }

    free(graph);
  }
}

int remove_vertex(Graph *g, int id) {
  Vertex *v = get_hash_table(g->represent, &id);
  if (!v) {
    return 0;
  }

  if (g->directed) {
    // remove edges starts from this vertex
    int removed = 0;
    HashtableIterator *iter = hashtable_iterator(g->edges);
    while (hashtable_iter_has_next(iter)) {
      KVEntry *kv = hashtable_next_entry(iter);
      Hashset *adj_set = table_entry_value(kv);
      removed += remove_edges_from(adj_set, id);
      removed += remove_edges_to(adj_set, id);
    }
    free_hashtable_iter(iter);
    g->edge_size -= removed;
  } else {
    Hashset *adj_set = get_adj_set(g, id);
    if (adj_set) {
      HashsetIterator *iter = hashset_iterator(adj_set);
      while (hashset_iter_has_next(iter)) {
        Edge *edge = set_entry_key(hashset_next_entry(iter));
        int to = edge->to;
        Hashset *to_adj = get_adj_set(g, to);
        // remove undirected graph edges
        remove_edge_from_to(to_adj, to, id);
      }
      int count = remove_edges_from(adj_set, id);
      g->edge_size -= count;
      free_hashset_iter(iter);
    }
  }
  int ret = 0;
  remove_with_flag_hash_table(g->represent, &id, &ret);
  if (ret) {
    free(v);
    g->vertex_size--;
    return 1;
  } else {
    return 0;
  }
}

int remove_edge(Graph *g, int from, int to) {
  Vertex *f = get_hash_table(g->represent, &from);
  if (!f) {
    return 0;
  }
  Vertex *t = get_hash_table(g->represent, &to);
  if (!t) {
    return 0;
  }
  Hashset *adj = get_adj_set(g, from);
  int ret = 0;
  if (adj) {
    ret = remove_edge_from_to(adj, from, to);
  }
  if (!g->directed) {
    adj = get_adj_set(g, to);
    if (adj) {
      ret += remove_edge_from_to(adj, to, from);
    }
  }
  if (ret) {
    g->edge_size--;
  }
  return ret;
}

Hashset *get_adj_set(Graph *graph, int id) {
  return get_hash_table(graph->edges, &id);
}

void free_vertex_entry(VertexEntry *vertex_entry) {
  if (vertex_entry) {
    free(vertex_entry->id_list);
    free(vertex_entry);
  }
}

int has_vertex(Graph *graph, int id) {
  return contains_in_hash_table(graph->represent, &id);
}
int degree_of(Graph *graph, int id) {
  assert(!graph->directed);
  assert(has_vertex(graph, id));
  Hashset *adj = get_adj_set(graph, id);
  return adj ? size_of_hash_set(adj) : 0;
}

int in_degree_of(Graph *graph, int id) {
  assert(graph->directed);
  assert(has_vertex(graph, id));
  int *degree = get_hash_table(graph->in_degree, &id);
  return degree ? *degree : 0;
}

int out_degree_of(Graph *graph, int id) {
  assert(graph->directed);
  assert(has_vertex(graph, id));
  int *degree = get_hash_table(graph->out_degree, &id);
  return degree ? *degree : 0;
}

int get_edge_to(Edge *edge) {
  return edge->to;
}
int get_edge_from(Edge *edge) {
  return edge->from;
}
int get_edge_weight(Edge *edge) {
  return edge->weight;
}

Graph *reverse_graph(Graph *graph) {
  if (!graph->directed) return graph;
  Graph *rg = create_graph(graph->directed, graph->weighted);
  // copy vertexes
  HashtableIterator *iter = hashtable_iterator(graph->represent);
  while (hashtable_iter_has_next(iter)) {
    KVEntry *entry = hashtable_next_entry(iter);
    Vertex *v = table_entry_value(entry);
    add_graph_data_with_id(rg, v->id, v->data);

    // reverse edges
    Hashset *adj = get_adj_set(graph, v->id);
    if (adj) {
      HashsetIterator *iterator = hashset_iterator(adj);
      while (hashset_iter_has_next(iterator)) {
        Edge *edge = set_entry_key(hashset_next_entry(iterator));
        add_edge(rg, edge->to, edge->from, edge->weight);
      }
      free_hashset_iter(iterator);
    }
  }
  free_hashtable_iter(iter);
  return rg;
}

// ------------------Graph operations-----------------------------


/**
 * Algorithms of Graph
 */
//---------------Graph Algorithms-----------------

VertexEntry *dfs_graph(Graph *graph) {
  VertexEntry *vertex_entry = new_vertex_entry(graph->vertex_size);
  if (!vertex_entry) return NULL;
  Hashset *visited = new_hash_set(int_hash, int_compare);
  register_hashset_free_functions(visited, free);
  // iterate vertexes
  HashtableIterator *iter = hashtable_iterator(graph->represent);
  while (hashtable_iter_has_next(iter)) {
    int *id = table_entry_key(hashtable_next_entry(iter));
    if (!contains_in_hash_set(visited, id)) {
      dfs(graph, visited, *id, vertex_entry);
    }
  }
  free_hashtable_iter(iter);
  free_hash_set(visited);
  return vertex_entry;
}

VertexEntry *dfs_graph_nr(Graph *graph) {
  VertexEntry *vertex_entry = new_vertex_entry(graph->vertex_size);
  if (!vertex_entry) return NULL;
  Hashset *visited = new_hash_set(int_hash, int_compare);
  register_hashset_free_functions(visited, free);

  // iterate vertexes
  HashtableIterator *iter = hashtable_iterator(graph->represent);
  while (hashtable_iter_has_next(iter)) {
    int *id = table_entry_key(hashtable_next_entry(iter));
    if (!contains_in_hash_set(visited, id)) {
      dfs_nr(graph, visited, *id, vertex_entry);
    }
  }

  free_hashtable_iter(iter);
  free_hash_set(visited);
  return vertex_entry;
}

int component_count(Graph *graph) {
  int c = 0;
  Hashset *visited = new_hash_set(int_hash, int_compare);
  register_hashset_free_functions(visited, free);
  // iterate vertexes
  HashtableIterator *iter = hashtable_iterator(graph->represent);
  while (hashtable_iter_has_next(iter)) {
    int *id = table_entry_key(hashtable_next_entry(iter));
    if (!contains_in_hash_set(visited, id)) {
      dfs_visit(graph, visited, *id);
      c++;
    }
  }

  free_hashtable_iter(iter);
  free_hash_set(visited);
  return c;
}

Hashtable *graph_components(Graph *graph) {
  int c = 0;
  Hashtable *visited = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(visited, free, free);
  // iterate vertexes
  HashtableIterator *iter = hashtable_iterator(graph->represent);
  while (hashtable_iter_has_next(iter)) {
    int *id = table_entry_key(hashtable_next_entry(iter));
    if (!contains_in_hash_table(visited, id)) {
      dfs_cid(graph, visited, *id, c);
      c++;
    }
  }

  free_hashtable_iter(iter);

  Hashtable *cmap = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(cmap, free, (HashtableValueFreeFunc) free_arraylist);

  iter = hashtable_iterator(visited);
  while (hashtable_iter_has_next(iter)) {
    KVEntry *entry = hashtable_next_entry(iter);
    int *id = table_entry_key(entry);
    int *cid = table_entry_value(entry);
    ArrayList *al;
    if (!contains_in_hash_table(cmap, cid)) {
      al = new_arraylist(0);
      put_hash_table(cmap, new_id(*cid), al);
    } else {
      al = get_hash_table(cmap, cid);
    }
    append_arraylist(al, new_id(*id));
  }

  free_hash_table(visited);
  free_hashtable_iter(iter);
  return cmap;
}

Hashtable *single_source_path(Graph *graph, int s, GraphOrd ord) {
  if (!has_vertex(graph, s)) {
    return NULL;
  }
  Hashtable *pre = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(pre, free, free);
  if (ord == DFS) {
    dfs_par(graph, pre, s, s);
  } else {
    bfs_par(graph, pre, s, s);
  }
  return pre;

}

LinkedList *single_source_path_to(Hashtable *premap, int to) {
  if (!premap) return NULL;
  LinkedList *ret = new_linked_list();
  int *p = get_hash_table(premap, &to);
  while (p != NULL && *p != to) {
    prepend_list(ret, new_id(to));
    to = *p;
    p = get_hash_table(premap, &to);
  }
  return ret;
}

int has_path(Graph *graph, int v1, int v2) {
  if (!has_vertex(graph, v1) || !has_vertex(graph, v2)) {
    return 0;
  }
  if (v1 == v2) {
    return 0;
  }
  Hashtable *pre = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(pre, free, free);
  int ret = dfs_cmp(graph, pre, v1, v1, v2);
  free_hash_table(pre);
  return ret;
}

LinkedList *one_path(Graph *graph, int v1, int v2) {
  if (!has_vertex(graph, v1) || !has_vertex(graph, v2)) {
    return NULL;
  }
  if (v1 == v2) {
    return NULL;
  }
  Hashtable *pre = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(pre, free, free);
  int ret = dfs_cmp(graph, pre, v1, v1, v2);
  if (!ret) {
    free_hash_table(pre);
    return NULL;
  }
  LinkedList *path = new_linked_list();
  int *p = get_hash_table(pre, &v2);
  while (*p != v2) {
    prepend_list(path, new_id(v2));
    v2 = *p;
    p = get_hash_table(pre, &v2);
  }
  free_hash_table(pre);
  return path;
}

int has_circle(Graph *graph) {
  if (graph->directed) {
    return has_circle_directed_graph(graph);
  } else {
    return has_circle_undirected_graph(graph);
  }
}

ArrayList *enumerate_circle_path(Graph *graph) {
  if (graph->directed) {
    ArrayList *result = new_arraylist(0);
    Hashset *visited = new_hash_set(int_hash, int_compare);
    register_hashset_free_functions(visited, free);

    Hashtable *on_path = new_hash_table(int_hash, int_compare);
    register_hashtable_free_functions(on_path, free, free);

    HashtableIterator *iter = hashtable_iterator(graph->represent);
    while (hashtable_iter_has_next(iter)) {
      int *id = table_entry_key(hashtable_next_entry(iter));
      if (!contains_in_hash_set(visited, id)) {
        d_circle_path(graph, visited, on_path, *id, *id, result);
      }
    }
    free_hashtable_iter(iter);

    free_hash_set(visited);
    free_hash_table(on_path);
    return result;
  } else {
    ArrayList *result = new_arraylist(0);

    Hashtable *path_visited = new_hash_table(int_hash, int_compare);
    register_hashtable_free_functions(path_visited, free, free);

    Hashset *visited = new_hash_set(int_hash, int_compare);
    register_hashset_free_functions(visited, free);

    HashtableIterator *iter = hashtable_iterator(graph->represent);

    while (hashtable_iter_has_next(iter)) {
      int *id = table_entry_key(hashtable_next_entry(iter));
      if (!contains_in_hash_set(visited, id)) {
        ud_circle_path(graph, visited, path_visited, *id, *id, result);
      }
    }

    free_hashtable_iter(iter);
    free_hash_table(path_visited);
    free_hash_set(visited);
    return result;
  }
}

int is_bipartite(Graph *graph) {
  assert(!graph->directed);
  // <id,color>, color:0,1
  Hashtable *visited = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(visited, free, free);

  HashtableIterator *iter = hashtable_iterator(graph->represent);
  int ret = 1;
  while (hashtable_iter_has_next(iter)) {
    int *id = table_entry_key(hashtable_next_entry(iter));
    if (!contains_in_hash_table(visited, id)) {
      if (!dfs_bipartite_test(graph, visited, *id, 0)) {
        ret = 0;
        goto clean_return;
      }
    }
  }

  clean_return:
  free_hashtable_iter(iter);
  free_hash_table(visited);
  return ret;
}

VertexEntry *bfs_graph(Graph *graph) {
  VertexEntry *vertex_entry = new_vertex_entry(graph->vertex_size);
  if (!vertex_entry) return NULL;
  Hashset *visited = new_hash_set(int_hash, int_compare);
  register_hashset_free_functions(visited, free);
  // iterate vertexes
  HashtableIterator *iter = hashtable_iterator(graph->represent);

  while (hashtable_iter_has_next(iter)) {
    int *id = table_entry_key(hashtable_next_entry(iter));
    if (!contains_in_hash_set(visited, id)) {
      bfs(graph, visited, *id, vertex_entry);
    }
  }

  free_hashtable_iter(iter);
  free_hash_set(visited);
  return vertex_entry;
}

LinkedList *find_bridge(Graph *graph) {
  if (graph->directed) {
    // TODO
    assert(0);
  } else {
    Hashset *visited = new_hash_set(int_hash, int_compare);
    register_hashset_free_functions(visited, free);

    Hashtable *ord = new_hash_table(int_hash, int_compare);
    register_hashtable_free_functions(ord, free, free);

    Hashtable *low = new_hash_table(int_hash, int_compare);
    register_hashtable_free_functions(low, free, free);

    int visited_count = 0;
    LinkedList *result = new_linked_list();

    HashtableIterator *iter = hashtable_iterator(graph->represent);

    while (hashtable_iter_has_next(iter)) {
      int *id = table_entry_key(hashtable_next_entry(iter));
      if (!contains_in_hash_set(visited, id)) {
        find_bridge_ud(graph, visited, ord, low, visited_count, *id, *id, result);
      }
    }

    free_hashtable_iter(iter);
    free_hash_set(visited);
    free_hash_table(ord);
    free_hash_table(low);
    return result;
  }
}

LinkedList *find_cut_point(Graph *graph) {
  if (graph->directed) {
    // TODO
    assert(0);
  } else {
    Hashset *visited = new_hash_set(int_hash, int_compare);
    register_hashset_free_functions(visited, free);

    Hashtable *ord = new_hash_table(int_hash, int_compare);
    register_hashtable_free_functions(ord, free, free);

    Hashtable *low = new_hash_table(int_hash, int_compare);
    register_hashtable_free_functions(low, free, free);

    int visited_count = 0;
    LinkedList *result = new_linked_list();

    HashtableIterator *iter = hashtable_iterator(graph->represent);

    while (hashtable_iter_has_next(iter)) {
      int *id = table_entry_key(hashtable_next_entry(iter));
      if (!contains_in_hash_set(visited, id)) {
        find_cut_point_ud(graph, visited, ord, low, visited_count, *id, *id, result);
      }
    }

    free_hashtable_iter(iter);
    free_hash_set(visited);
    free_hash_table(ord);
    free_hash_table(low);
    return result;
  }
}

LinkedList *hamilton_loop_path(Graph *graph) {
  assert(!graph->directed);

  Hashtable *visited = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(visited, free, free);

  LinkedList *result = NULL;

  // only pick one vertex as start
  if (graph->vertex_size > 0) {
    int id = pick_one_id(graph);
    int end = -1;
    if (dfs_hamilton_loop_path(graph, visited, id, &end, id, id)) {
      result = track_path(visited, end, id);
      goto clean;
    }
  }
  clean:
  free_hash_table(visited);
  return result;
}

LinkedList *hamilton_path(Graph *graph, int start) {
  assert(!graph->directed);
  assert(has_vertex(graph, start));

  Hashtable *visited = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(visited, free, free);

  LinkedList *result = NULL;

  // only pick one vertex as start

  int end = -1;
  if (dfs_hamilton_path(graph, visited, &end, start, start)) {
    result = track_path(visited, end, start);
    goto clean;
  }

  clean:
  free_hash_table(visited);
  return result;
}
int has_euler_loop(Graph *graph) {
  if (graph->directed) {
    // TODO component test

    int ret = 1;
    HashtableIterator *iterator = hashtable_iterator(graph->represent);
    while (hashtable_iter_has_next(iterator)) {
      int *id = table_entry_key(hashtable_next_entry(iterator));
      int in_degree = in_degree_of(graph, *id);
      int out_degree = out_degree_of(graph, *id);
      if (in_degree == 0 || in_degree != out_degree) {
        ret = 0;
        break;
      }
    }
    free_hashtable_iter(iterator);
    return ret;
  } else {
    int cc = component_count(graph);
    if (cc > 1) {
      return 0;
    }
    int ret = 1;
    HashtableIterator *iterator = hashtable_iterator(graph->represent);
    while (hashtable_iter_has_next(iterator)) {
      int *id = table_entry_key(hashtable_next_entry(iterator));
      int degree = degree_of(graph, *id);
      if (degree == 0 || degree % 2 == 1) {
        ret = 0;
        break;
      }
    }
    free_hashtable_iter(iterator);
    return ret;
  }
}

LinkedList *hierholzer_euler_loop(Graph *graph) {
  if (!has_euler_loop(graph)) {
    return NULL;
  }
  LinkedList *ret = new_linked_list();
  Hashtable *cpe = copy_edges(graph);

  Dqueue *stack = new_dqueue();
  int cur = pick_one_id(graph);
  dqueue_push_tail(stack, new_id(cur));
  while (!dqueue_is_empty(stack)) {
    Hashset *edges = get_hash_table(cpe, &cur);
    if (size_of_hash_set(edges) != 0) {
      dqueue_push_tail(stack, new_id(cur));
      Edge *e = pick_one_edge(edges);
      int t = e->to;
      remove_edge_from_to(edges, cur, t);
      if (!graph->directed) {
        edges = get_hash_table(cpe, &t);
        remove_edge_from_to(edges, t, cur);
      }
      cur = t;
    } else {
      prepend_list(ret, new_id(cur));
      int *top = dqueue_pop_tail(stack);
      cur = *top;
      free(top);
    }
  }
  free_edges(cpe);
  free_dqueue(stack);
  return ret;
}

LinkedList *kruskal_mst(Graph *graph) {
  assert(!graph->directed);
  assert(graph->weighted);
  int cc = component_count(graph);
  if (cc > 1) {
    return NULL;
  }
  LinkedList *mst = new_linked_list();
  // sort the edges
  ArrayList *edge_list = new_arraylist(0);
  HashtableIterator *iter = hashtable_iterator(graph->edges);
  while (hashtable_iter_has_next(iter)) {
    Hashset *edge = table_entry_value(hashtable_next_entry(iter));
    HashsetIterator *iterator = hashset_iterator(edge);
    while (hashset_iter_has_next(iterator)) {
      Edge *e = set_entry_key(hashset_next_entry(iterator));
      if (e->from < e->to) {
        append_arraylist(edge_list, e);
      }
    }
    free_hashset_iter(iterator);
  }
  free_hashtable_iter(iter);
  sort_arraylist(edge_list, edge_weight_compare);

  // TODO size of union find should be the max id of the graph because id can be designated as wish
  // however, usually id of vertex is auto-generated sequentially so the size of vertexes is the union find size.
  UnionFind *uf = create_uf(graph->vertex_size);
  for (int i = 0; i < edge_list->size; ++i) {
    Edge *edge = get_data_arraylist(edge_list, i);
    int f = edge->from;
    int t = edge->to;
    if (!uf_same_set(uf, f, t)) {
      append_list(mst, create_edge(f, t, edge->weight));
      uf_union(uf, f, t);
    }
  }
  free_arraylist(edge_list);
  free_uf(uf);
  return mst;
}

LinkedList *prim_mst(Graph *graph) {
  assert(!graph->directed);
  assert(graph->weighted);
  int cc = component_count(graph);
  if (cc > 1) {
    return NULL;
  }
  LinkedList *mst = new_linked_list();
  PriorityQueue *pq = create_pq(edge_weight_compare_pq);
  Hashset *visited = new_hash_set(int_hash, int_compare);

  int v = pick_one_id(graph);
  put_hash_set(visited, &v);
  Hashset *ve = get_adj_set(graph, v);

  HashsetIterator *iterator = hashset_iterator(ve);
  while (hashset_iter_has_next(iterator)) {
    Edge *e = set_entry_key(hashset_next_entry(iterator));
    pq_enqueue(pq, e);
  }
  free_hashset_iter(iterator);

  while (!is_pq_empty(pq)) {
    Edge *min_edge = pq_dequeue(pq);
    if (contains_in_hash_set(visited, &min_edge->to)
        && contains_in_hash_set(visited, &min_edge->from)) {
      continue;
    }
    append_list(mst, create_edge(min_edge->from, min_edge->to, min_edge->weight));
    int *new_v = contains_in_hash_set(visited, &min_edge->from) ? &min_edge->to : &min_edge->from;
    put_hash_set(visited, new_v);
    ve = get_adj_set(graph, *new_v);

    iterator = hashset_iterator(ve);
    while (hashset_iter_has_next(iterator)) {
      Edge *edge = set_entry_key(hashset_next_entry(iterator));
      if (!contains_in_hash_set(visited, &edge->to)) {
        pq_enqueue(pq, edge);
      }
    }
    free_hashset_iter(iterator);
  }
  free_pq(pq, NULL);
  free_hash_set(visited);
  return mst;
}

// for dijkstra algorithm
struct DIS {
  int id;
  int dis;
};

Hashtable *dijkstra(Graph *graph, int s) {
  // TODO check if there is any negative weight edges
  assert(graph->weighted);
  assert(has_vertex(graph, s));
  Hashtable *dis = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(dis, free, free);
  Hashset *confirmed = new_hash_set(int_hash, int_compare);
  register_hashset_free_functions(confirmed, free);
  PriorityQueue *pq = create_pq((cmp_func) dis_compare_pq);

  put_hash_table(dis, new_id(s), new_id(0));
  DIS *d = create_dis(s, 0);
  pq_enqueue(pq, d);

  while (!is_pq_empty(pq)) {
    DIS *e = pq_dequeue(pq);
    int cur = e->id;
    free(e);
    if (contains_in_hash_set(confirmed, &cur)) continue;

    put_hash_set(confirmed, new_id(cur));

    int cur_dis = *(int *) get_hash_table(dis, &cur);
    Hashset *adj = get_adj_set(graph, cur);

    HashsetIterator *iterator = hashset_iterator(adj);
    while (hashset_iter_has_next(iterator)) {
      Edge *edge = set_entry_key(hashset_next_entry(iterator));
      int w = edge->to;
      if (!contains_in_hash_set(confirmed, &w)) {
        if (!contains_in_hash_table(dis, &w)) {
          int w_dis = cur_dis + edge->weight;
          put_hash_table(dis, new_id(w), new_id(w_dis));
          pq_enqueue(pq, create_dis(w, w_dis));
        } else {
          int *w_dis = get_hash_table(dis, &w);
          if (cur_dis + edge->weight < *w_dis) {
            int new_dis = cur_dis + edge->weight;
            put_free_exist_hash_table(dis, &w, new_id(new_dis), free);
            pq_enqueue(pq, create_dis(w, new_dis));
            //free(w_dis);
          }
        }
      }
    }
    free_hashset_iter(iterator);
  }
  free_hash_set(confirmed);
  free_pq(pq, free);
  return dis;
}

Hashtable *dijkstra_path(Graph *graph, int s, Hashtable **pre_map) {
  // TODO check if there is any negative weight edges
  assert(graph->weighted);
  assert(has_vertex(graph, s));
  assert(*pre_map == NULL);
  *pre_map = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(*pre_map, free, free);
  Hashtable *dis = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(dis, free, free);
  Hashset *confirmed = new_hash_set(int_hash, int_compare);
  register_hashset_free_functions(confirmed, free);
  PriorityQueue *pq = create_pq((cmp_func) dis_compare_pq);

  put_hash_table(dis, new_id(s), new_id(0));
  DIS *d = create_dis(s, 0);
  pq_enqueue(pq, d);

  while (!is_pq_empty(pq)) {
    DIS *e = pq_dequeue(pq);
    int cur = e->id;
    free(e);
    if (contains_in_hash_set(confirmed, &cur)) continue;

    put_hash_set(confirmed, new_id(cur));

    int cur_dis = *(int *) get_hash_table(dis, &cur);
    Hashset *adj = get_adj_set(graph, cur);
    HashsetIterator *iterator = hashset_iterator(adj);
    while (hashset_iter_has_next(iterator)) {
      Edge *edge = set_entry_key(hashset_next_entry(iterator));
      int w = edge->to;
      if (!contains_in_hash_set(confirmed, &w)) {
        if (!contains_in_hash_table(dis, &w)) {
          int w_dis = cur_dis + edge->weight;
          put_hash_table(dis, new_id(w), new_id(w_dis));
          pq_enqueue(pq, create_dis(w, w_dis));
          put_hash_table(*pre_map, new_id(w), new_id(cur));
        } else {
          int *w_dis = get_hash_table(dis, &w);
          if (cur_dis + edge->weight < *w_dis) {
            int new_dis = cur_dis + edge->weight;
            put_free_exist_hash_table(dis, &w, new_id(new_dis), free);
            pq_enqueue(pq, create_dis(w, new_dis));
            //free(get_hash_table(*pre_map, &w));
            put_free_exist_hash_table(*pre_map, &w, new_id(cur), free);
            //free(w_dis);
          }
        }
      }
    }
    free_hashset_iter(iterator);
  }
  free_hash_set(confirmed);
  free_pq(pq, free);
  return dis;
}

LinkedList *dijkstra_path_to(Graph *graph, int s, int t) {
  assert(graph->weighted);
  assert(has_vertex(graph, s));
  assert(has_vertex(graph, t));
  assert(s != t);
  Hashtable *pre_map = new_hash_table(int_hash, int_compare);
  Hashtable *dis = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(dis, free, free);
  register_hashtable_free_functions(pre_map, free, free);
  Hashset *confirmed = new_hash_set(int_hash, int_compare);
  register_hashset_free_functions(confirmed, free);
  PriorityQueue *pq = create_pq((cmp_func) dis_compare_pq);

  put_hash_table(dis, new_id(s), new_id(0));
  DIS *d = create_dis(s, 0);
  pq_enqueue(pq, d);

  while (!is_pq_empty(pq)) {
    DIS *e = pq_dequeue(pq);
    int cur = e->id;
    free(e);

    if (contains_in_hash_set(confirmed, &cur)) {
      if (cur == t) {
        break;
      }
      continue;
    }

    put_hash_set(confirmed, new_id(cur));

    int cur_dis = *(int *) get_hash_table(dis, &cur);
    Hashset *adj = get_adj_set(graph, cur);

    HashsetIterator *iterator = hashset_iterator(adj);
    while (hashset_iter_has_next(iterator)) {
      Edge *edge = set_entry_key(hashset_next_entry(iterator));
      int w = edge->to;
      if (!contains_in_hash_set(confirmed, &w)) {
        if (!contains_in_hash_table(dis, &w)) {
          int w_dis = cur_dis + edge->weight;
          put_hash_table(dis, new_id(w), new_id(w_dis));
          pq_enqueue(pq, create_dis(w, w_dis));
          put_hash_table(pre_map, new_id(w), new_id(cur));
        } else {
          int *w_dis = get_hash_table(dis, &w);
          if (cur_dis + edge->weight < *w_dis) {
            int new_dis = cur_dis + edge->weight;
            put_free_exist_hash_table(dis, &w, new_id(new_dis), free);
            pq_enqueue(pq, create_dis(w, new_dis));
            // free(get_hash_table(pre_map, &w));
            put_free_exist_hash_table(pre_map, &w, new_id(cur), free);
            // free(w_dis);
          }
        }
      }
    }
    free_hashset_iter(iterator);
  }

  LinkedList *p = NULL;
  if (contains_in_hash_set(confirmed, &t)) {
    p = track_path(pre_map, t, s);
  }
  free_hash_set(confirmed);
  free_pq(pq, free);
  free_hash_table(pre_map);
  free_hash_table(dis);
  return p;
}

Hashtable *bellman_ford(Graph *graph, int s) {
  assert(graph->weighted);
  assert(has_vertex(graph, s));
  Hashtable *dis = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(dis, free, free);
  put_hash_table(dis, new_id(s), new_id(0));
  // v-1 round of relaxation operations
  for (int i = 0; i < graph->vertex_size - 1; ++i) {
    HashtableIterator *iter = hashtable_iterator(graph->represent);
    while (hashtable_iter_has_next(iter)) {
      int *id = table_entry_key(hashtable_next_entry(iter));
      Hashset *adj = get_adj_set(graph, *id);
      HashsetIterator *adj_iter = hashset_iterator(adj);
      while (hashset_iter_has_next(adj_iter)) {
        Edge *edge = set_entry_key(hashset_next_entry(adj_iter));
        int t = edge->to;
        if (contains_in_hash_table(dis, id)) {
          int *d = get_hash_table(dis, id);
          if (!contains_in_hash_table(dis, &t)) {
            put_hash_table(dis, new_id(t), new_id(*d + edge->weight));
          } else {
            int *t_dis = get_hash_table(dis, &t);
            if (*d + edge->weight < *t_dis) {
              // free(t_dis);
              put_free_exist_hash_table(dis, &t, new_id(*d + edge->weight), free);
            }
          }
        }
      }
      free_hashset_iter(adj_iter);
    }
    free_hashtable_iter(iter);
  }

  // extra relaxation operation to check if graph has negative edge circle
  HashtableIterator *iter = hashtable_iterator(graph->represent);
  while (hashtable_iter_has_next(iter)) {
    int *id = table_entry_key(hashtable_next_entry(iter));
    Hashset *adj = get_adj_set(graph, *id);
    HashsetIterator *adj_iter = hashset_iterator(adj);
    while (hashset_iter_has_next(adj_iter)) {
      Edge *edge = set_entry_key(hashset_next_entry(adj_iter));
      int t = edge->to;
      if (contains_in_hash_table(dis, id)) {
        int *d = get_hash_table(dis, id);
        if (!contains_in_hash_table(dis, &t)) {
          put_hash_table(dis, new_id(t), new_id(*d + edge->weight));
        } else {
          int *t_dis = get_hash_table(dis, &t);
          if (*d + edge->weight < *t_dis) {
            // has negative edge circle. can't use this algorithm.
            free_hash_table(dis);
            free_hashset_iter(adj_iter);
            free_hashtable_iter(iter);
            return NULL;
          }
        }
      }
    }
    free_hashset_iter(adj_iter);
  }
  free_hashtable_iter(iter);
  return dis;
}

LinkedList *bellman_ford_path_to(Graph *graph, int source, int to) {
  assert(graph->weighted);
  assert(has_vertex(graph, source));
  assert(has_vertex(graph, to));
  assert(source != to);

  Hashtable *pre_map = new_hash_table(int_hash, int_compare);
  Hashtable *dis = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(dis, free, free);
  register_hashtable_free_functions(pre_map, free, free);

  put_hash_table(dis, new_id(source), new_id(0));
  put_hash_table(pre_map, new_id(source), new_id(source));

  // v-1 round of relaxation operations
  for (int i = 0; i < graph->vertex_size - 1; ++i) {
    HashtableIterator *iter = hashtable_iterator(graph->represent);
    while (hashtable_iter_has_next(iter)) {
      int *id = table_entry_key(hashtable_next_entry(iter));
      Hashset *adj = get_adj_set(graph, *id);
      HashsetIterator *adj_iter = hashset_iterator(adj);
      while (hashset_iter_has_next(adj_iter)) {
        Edge *edge = set_entry_key(hashset_next_entry(adj_iter));
        int t = edge->to;
        if (contains_in_hash_table(dis, id)) {
          int *d = get_hash_table(dis, id);
          if (!contains_in_hash_table(dis, &t)) {
            put_hash_table(dis, new_id(t), new_id(*d + edge->weight));
            put_hash_table(pre_map, new_id(t), new_id(*id));
          } else {
            int *t_dis = get_hash_table(dis, &t);
            if (*d + edge->weight < *t_dis) {
              // free(t_dis);
              put_free_exist_hash_table(dis, &t, new_id(*d + edge->weight), free);
              put_free_exist_hash_table(pre_map, &t, new_id(*id), free);
            }
          }
        }
      }
      free_hashset_iter(adj_iter);
    }
    free_hashtable_iter(iter);
  }

  // extra relaxation operation to check if graph has negative edge circle
  HashtableIterator *iter = hashtable_iterator(graph->represent);
  while (hashtable_iter_has_next(iter)) {
    int *id = table_entry_key(hashtable_next_entry(iter));
    Hashset *adj = get_adj_set(graph, *id);
    HashsetIterator *adj_iter = hashset_iterator(adj);
    while (hashset_iter_has_next(adj_iter)) {
      Edge *edge = set_entry_key(hashset_next_entry(adj_iter));
      int t = edge->to;
      if (contains_in_hash_table(dis, id)) {
        int *d = get_hash_table(dis, id);
        if (!contains_in_hash_table(dis, &t)) {
          put_hash_table(dis, new_id(t), new_id(*d + edge->weight));
        } else {
          int *t_dis = get_hash_table(dis, &t);
          if (*d + edge->weight < *t_dis) {
            // has negative edge circle. can't use this algorithm.
            free_hash_table(dis);
            free_hashset_iter(adj_iter);
            free_hashtable_iter(iter);
            free_hash_table(pre_map);
            return NULL;
          }
        }
      }
    }
    free_hashset_iter(adj_iter);
  }
  free_hashtable_iter(iter);
  free_hash_table(dis);
  LinkedList *path = track_path(pre_map, to, source);
  free_hash_table(pre_map);
  return path;
}

Hashtable *floyd(Graph *graph, int *has_negative_circle) {
  assert(graph->weighted);
  Hashtable *dis = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(dis, free, (HashtableValueFreeFunc) free_hash_table);
  HashtableIterator *vs = hashtable_iterator(graph->represent);
  while (hashtable_iter_has_next(vs)) {
    int *id = table_entry_key(hashtable_next_entry(vs));
    // initialize distance map. dis[v][v]=0
    Hashtable *distance = new_hash_table(int_hash, int_compare);
    register_hashtable_free_functions(distance, free, free);
    put_hash_table(dis, new_id(*id), distance);

    put_hash_table(distance, new_id(*id), new_id(0));

    Hashset *adj = get_adj_set(graph, *id);
    if (!adj) continue;
    // initialize distance map. dis[v][w] = weight
    HashsetIterator *iterator = hashset_iterator(adj);
    while (hashset_iter_has_next(iterator)) {
      Edge *edge = set_entry_key(hashset_next_entry(iterator));
      put_hash_table(distance, new_id(edge->to), new_id(edge->weight));
    }
    free_hashset_iter(iterator);
  }
  free_hashtable_iter(vs);

  HashtableIterator *iter1 = hashtable_iterator(graph->represent);
  while (hashtable_iter_has_next(iter1)) {
    int *t = table_entry_key(hashtable_next_entry(iter1));
    HashtableIterator *iter2 = hashtable_iterator(graph->represent);
    while (hashtable_iter_has_next(iter2)) {
      int *v = table_entry_key(hashtable_next_entry(iter2));
      HashtableIterator *iter3 = hashtable_iterator(graph->represent);
      while (hashtable_iter_has_next(iter3)) {
        int *w = table_entry_key(hashtable_next_entry(iter3));
        int *vt = get_hash_table(get_hash_table(dis, v), t);
        int *tw = get_hash_table(get_hash_table(dis, t), w);
        int *vw = get_hash_table(get_hash_table(dis, v), w);
        if (vt != NULL && tw != NULL) {
          if (vw == NULL) {
            put_hash_table(get_hash_table(dis, v), new_id(*w), new_id((*vt) + (*tw)));
          } else {
            if ((*vt) + (*tw) < (*vw)) {
              put_free_exist_hash_table(get_hash_table(dis, v), w, new_id((*vt) + (*tw)), free);
            }
          }
        }
      }
      free_hashtable_iter(iter3);
    }
    free_hashtable_iter(iter2);
  }
  free_hashtable_iter(iter1);

  HashtableIterator *iterator = hashtable_iterator(graph->represent);
  while (hashtable_iter_has_next(iterator)) {
    int *id = table_entry_key(hashtable_next_entry(iterator));
    int *d = get_hash_table(get_hash_table(dis, id), id);
    *has_negative_circle = *d < 0 ? 1 : 0;
  }
  free_hashtable_iter(iterator);
  return dis;
}

Hashtable *floyd_path(Graph *graph, int *has_negative_circle, Hashtable **next_matrix) {
  assert(graph->weighted);
  Hashtable *dis = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(dis, free, (HashtableValueFreeFunc) free_hash_table);
  *next_matrix = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(*next_matrix, free, (HashtableValueFreeFunc) free_hash_table);

  HashtableIterator *vs = hashtable_iterator(graph->represent);
  while (hashtable_iter_has_next(vs)) {
    int *id = table_entry_key(hashtable_next_entry(vs));
    // initialize distance map. dis[v][v]=0
    Hashtable *distance = new_hash_table(int_hash, int_compare);
    register_hashtable_free_functions(distance, free, free);
    put_hash_table(dis, new_id(*id), distance);

    put_hash_table(distance, new_id(*id), new_id(0));

    Hashset *adj = get_adj_set(graph, *id);
    if (!adj) continue;
    // initialize distance map. dis[v][w] = weight
    HashsetIterator *iterator = hashset_iterator(adj);

    Hashtable *next = new_hash_table(int_hash, int_compare);
    register_hashtable_free_functions(next, free, free);
    put_hash_table(*next_matrix, new_id(*id), next);

    while (hashset_iter_has_next(iterator)) {
      Edge *edge = set_entry_key(hashset_next_entry(iterator));
      put_hash_table(distance, new_id(edge->to), new_id(edge->weight));
      // For each pair of vertices (i, j), if there is a direct edge from i to j, set next[i][j] to j.
      put_hash_table(next, new_id(edge->to), new_id(edge->to));
    }
    free_hashset_iter(iterator);
  }
  free_hashtable_iter(vs);

  HashtableIterator *iter1 = hashtable_iterator(graph->represent);
  while (hashtable_iter_has_next(iter1)) {
    int *t = table_entry_key(hashtable_next_entry(iter1));
    HashtableIterator *iter2 = hashtable_iterator(graph->represent);
    while (hashtable_iter_has_next(iter2)) {
      int *v = table_entry_key(hashtable_next_entry(iter2));
      HashtableIterator *iter3 = hashtable_iterator(graph->represent);
      while (hashtable_iter_has_next(iter3)) {
        int *w = table_entry_key(hashtable_next_entry(iter3));
        int *vt = get_hash_table(get_hash_table(dis, v), t);
        int *tw = get_hash_table(get_hash_table(dis, t), w);
        int *vw = get_hash_table(get_hash_table(dis, v), w);
        if (vt != NULL && tw != NULL) {
          //During the main loop of the Floyd-Warshall algorithm, when find a shorter path from i to j through k
          // (i.e., when dis[i][j] > dis[i][k] + dis[k][j]), update dis[i][j] as before, and also update the next
          // matrix to reflect the change in path. Specifically, set next[i][j] to next[i][k], because the next
          // step from i to reach j via k is to go from i to k.
          if (vw == NULL) {
            put_hash_table(get_hash_table(dis, v), new_id(*w), new_id((*vt) + (*tw)));
            Hashtable *next = get_hash_table(*next_matrix, v);
            int *k = get_hash_table(next, t);
            put_hash_table(next, new_id(*w), new_id(*k));
          } else {
            if ((*vt) + (*tw) < (*vw)) {
              put_free_exist_hash_table(get_hash_table(dis, v), w, new_id((*vt) + (*tw)), free);
              Hashtable *next = get_hash_table(*next_matrix, v);
              int *k = get_hash_table(next, t);
              put_free_exist_hash_table(next, w, new_id(*k), free);
            }
          }
        }
      }
      free_hashtable_iter(iter3);
    }
    free_hashtable_iter(iter2);
  }
  free_hashtable_iter(iter1);

  HashtableIterator *iterator = hashtable_iterator(graph->represent);
  while (hashtable_iter_has_next(iterator)) {
    int *id = table_entry_key(hashtable_next_entry(iterator));
    int *d = get_hash_table(get_hash_table(dis, id), id);
    *has_negative_circle = *d < 0 ? 1 : 0;
  }
  free_hashtable_iter(iterator);
  return dis;
}

LinkedList *floyd_path_to(Hashtable *next_matrix, int source, int to) {
  Hashtable *next = get_hash_table(next_matrix, &source);
  if (!next) return NULL;
  if (!contains_in_hash_table(next, &to)) {
    return NULL;
  }
  LinkedList *path = new_linked_list();
  append_list(path, new_id(source));
  while (source != to) {
    next = get_hash_table(next_matrix, &source);
    source = *(int *) get_hash_table(next, &to);
    append_list(path, new_id(source));
  }
  return path;
}

LinkedList *topological_sort(Graph *graph) {
  assert(graph->directed);
  Hashtable *in_degree = new_hash_table(int_hash, int_compare);
  Dqueue *queue = new_dqueue();
  HashtableIterator *iter = hashtable_iterator(graph->represent);
  while (hashtable_iter_has_next(iter)) {
    int *id = table_entry_key(hashtable_next_entry(iter));
    int degree = in_degree_of(graph, *id);
    if (degree == 0) dqueue_push_tail(queue, id);
    put_hash_table(in_degree, id, new_id(degree));
  }
  free_hashtable_iter(iter);
  LinkedList *ret = new_linked_list();
  while (!dqueue_is_empty(queue)) {
    int *id = dqueue_pop_head(queue);
    append_list(ret, new_id(*id));
    Hashset *adj = get_adj_set(graph, *id);
    if (adj) {
      HashsetIterator *iterator = hashset_iterator(adj);
      while (hashset_iter_has_next(iterator)) {
        Edge *edge = set_entry_key(hashset_next_entry(iterator));
        int in = *(int *) get_hash_table(in_degree, &edge->to);
        put_free_exist_hash_table(in_degree, &edge->to, new_id(in - 1), free);
        if (in - 1 == 0) dqueue_push_tail(queue, &edge->to);
      }
      free_hashset_iter(iterator);
    }
  }
  free_dqueue(queue);
  register_hashtable_free_functions(in_degree, NULL, free);
  free_hash_table(in_degree);
  if (list_size(ret) != graph->vertex_size) {
    // has circle
    free_linked_list(ret, free);
    return NULL;
  }
  return ret;
}

Hashtable *scc_kosaraju(Graph *graph) {
  assert(graph->directed);
  // get reverse graph
  Graph *tg = reverse_graph(graph);
  // dfs(post order) reversed graph
  VertexEntry *entry = dfs_graph(tg);
  free_graph(tg);

  Hashtable *strongly_components = new_hash_table(int_hash, int_compare);
  int cid = 0;
  Hashtable *visited = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(visited, free, free);

  // dfs original graph
  for (int i = entry->size - 1; i >= 0; --i) {
    int id = entry->id_list[i];
    if (!contains_in_hash_table(visited, &id)) {
      dfs_cid(graph, visited, id, cid);
      cid++;
    }
  }
  free_vertex_entry(entry);
  register_hashtable_free_functions(strongly_components, free, NULL);

  HashtableIterator *iter = hashtable_iterator(visited);
  while (hashtable_iter_has_next(iter)) {
    KVEntry *kv = hashtable_next_entry(iter);
    int *id = table_entry_key(kv);
    int *c_id = table_entry_value(kv);
    LinkedList *ll;
    if (!contains_in_hash_table(strongly_components, c_id)) {
      ll = new_linked_list();
      put_hash_table(strongly_components, new_id(*c_id), ll);
    } else {
      ll = get_hash_table(strongly_components, c_id);
    }
    append_list(ll, new_id(*id));
  }

  free_hash_table(visited);
  free_hashtable_iter(iter);
  return strongly_components;

}

Hashtable *max_flow(Graph *graph, int source, int to, int *max_flow) {
  assert(graph->directed);
  assert(graph->weighted);
  assert(graph->vertex_size > 1);
  assert(source != to);
  assert(has_vertex(graph, source));
  assert(has_vertex(graph, to));
  *max_flow = 0;

  Graph *rg = create_residual_graph(graph);
  while (1) {
    LinkedList *path = get_augmenting_path(rg, source, to);
    if (path == NULL) break;
    // get the minimum weight in path
    int f = INT_MAX;
    LinkedListNode *list_node = head_of_list(path);
    int i = 1;
    int size = list_size(path);
    while (i < size) {
      int *v = data_of_node_linked_list(list_node);
      int *w = data_of_node_linked_list(next_node_linked_list(list_node));
      Edge *edge = get_edge(rg, *v, *w);
      if (f > edge->weight) f = edge->weight;
      i++;
      list_node = next_node_linked_list(list_node);
    }
    *max_flow = *max_flow + f;

    // update residual graph
    i = 1;
    list_node = head_of_list(path);
    while (i < size) {
      int *v = data_of_node_linked_list(list_node);
      int *w = data_of_node_linked_list(next_node_linked_list(list_node));
      set_weight(rg, *v, *w, get_edge(rg, *v, *w)->weight - f);
      set_weight(rg, *w, *v, get_edge(rg, *w, *v)->weight + f);
      i++;
      list_node = next_node_linked_list(list_node);
    }
    free_linked_list(path, free);
  }
  Hashtable *flow = copy_edges(graph);
  HashtableIterator *iter = hashtable_iterator(flow);
  while (hashtable_iter_has_next(iter)) {
    Hashset *adj = table_entry_value(hashtable_next_entry(iter));
    if (adj) {
      HashsetIterator *iterator = hashset_iterator(adj);
      while (hashset_iter_has_next(iterator)) {
        Edge *edge = set_entry_key(hashset_next_entry(iterator));
        Edge *e = get_edge(rg, edge->to, edge->from);
        edge->weight = e->weight;
      }
      free_hashset_iter(iterator);
    }
  }
  free_hashtable_iter(iter);
  free_graph(rg);
  return flow;
}

int bipartite_matching(Graph *graph) {
  assert(!graph->directed);
  // <id,color>, color:0,1
  Hashtable *visited = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(visited, free, free);

  HashtableIterator *iter = hashtable_iterator(graph->represent);
  int is_bipartite = 1;
  while (hashtable_iter_has_next(iter)) {
    int *id = table_entry_key(hashtable_next_entry(iter));
    if (!contains_in_hash_table(visited, id)) {
      if (!dfs_bipartite_test(graph, visited, *id, 0)) {
        is_bipartite = 0;
        break;
      }
    }
  }
  free_hashtable_iter(iter);
  if (is_bipartite == 0) {
    free_hash_table(visited);
    assert(is_bipartite == 1);
  }
  // Make a new directed graph with 2 extra vertexes: source and target.
  // Add edges from source to color0 vertexes and color1 vertexes to target.
  // The weight of edges are 1.
  // Get the max flow from source to target and its result is the max matching.
  Graph *g = create_graph(1, 1);
  HashtableIterator *vs = hashtable_iterator(graph->represent);
  while (hashtable_iter_has_next(vs)) {
    KVEntry *kv = hashtable_next_entry(vs);
    int *id = table_entry_key(kv);
    // copy vertexes
    add_graph_data_with_id(g, *id, NULL);
  }
  free_hashtable_iter(vs);
  // add source
  int source_id = graph->vertex_size;
  int target_id = source_id + 1;
  add_graph_data_with_id(g, source_id, NULL);
  add_graph_data_with_id(g, target_id, NULL);

  // add edges
  vs = hashtable_iterator(graph->represent);
  while (hashtable_iter_has_next(vs)) {
    KVEntry *kv = hashtable_next_entry(vs);
    int *id = table_entry_key(kv);
    Hashset *adj = get_adj_set(graph, *id);
    if (adj) {
      HashsetIterator *es = hashset_iterator(adj);
      while (hashset_iter_has_next(es)) {
        Edge *e = set_entry_key(hashset_next_entry(es));
        add_graph_data_with_id(g, e->to, NULL);
        if (e->from < e->to) {
          int *color = get_hash_table(visited, &e->from);
          if (*color == 0) {
            add_edge(g, e->from, e->to, 1);
            add_edge(g, source_id, e->from, 1);
            add_edge(g, e->to, target_id, 1);
          } else {
            add_edge(g, e->to, e->from, 1);
            add_edge(g, source_id, e->to, 1);
            add_edge(g, e->from, target_id, 1);
          }
        }
      }
      free_hashset_iter(es);
    }
  }
  free_hashtable_iter(vs);
  int maxflow = 0;
  Hashtable *flow = max_flow(g, source_id, target_id, &maxflow);
  register_hashtable_free_functions(flow, free, (HashtableValueFreeFunc) free_hash_set);
  free_hash_table(flow);
  free_graph(g);
  free_hash_table(visited);
  return maxflow;
}

int hungarian_matching(Graph *graph) {
  assert(!graph->directed);
  // <id,color>, color:0,1
  Hashtable *color = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(color, free, free);

  HashtableIterator *iter = hashtable_iterator(graph->represent);
  int is_bipartite = 1;
  while (hashtable_iter_has_next(iter)) {
    int *id = table_entry_key(hashtable_next_entry(iter));
    if (!contains_in_hash_table(color, id)) {
      if (!dfs_bipartite_test(graph, color, *id, 0)) {
        is_bipartite = 0;
        break;
      }
    }
  }
  free_hashtable_iter(iter);
  if (is_bipartite == 0) {
    free_hash_table(color);
    assert(is_bipartite == 1);
  }

  // Choose an un-matching vertex in left size(color0)
  // Start finding a path from the vertex, if the vertex in right side(color1) is already matched, only
  // go through along matching edge.
  // If the vertex on right side is un-matched. find an augmenting path and increase max flow.

  Hashtable *matching = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(matching, free, free);

  int maxflow = 0;
  HashtableIterator *vs = hashtable_iterator(graph->represent);
  while (hashtable_iter_has_next(vs)) {
    int *id = table_entry_key(hashtable_next_entry(vs));
    if (!contains_in_hash_table(matching, id) && *(int *) get_hash_table(color, id) == 0) {
      if (bfs_hungarian(graph, matching, *id)) maxflow++;
    }
  }
  free_hashtable_iter(vs);
  free_hash_table(matching);
  free_hash_table(color);
  return maxflow;
}
//---------------Graph Algorithms-----------------

//--------------- static functions ----------------------
static int bfs_hungarian(Graph *graph, Hashtable *matching, int id) {
  Hashtable *pre = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(pre, free, free);
  Dqueue *queue = new_dqueue();

  put_hash_table(pre, new_id(id), new_id(id));
  dqueue_push_tail(queue, &id);
  while (!dqueue_is_empty(queue)) {
    int *cur = dqueue_pop_head(queue);
    Hashset *adj = get_adj_set(graph, *cur);
    HashsetIterator *iter = hashset_iterator(adj);
    while (hashset_iter_has_next(iter)) {
      Edge *edge = set_entry_key(hashset_next_entry(iter));
      int next = get_edge_to(edge);
      if (!contains_in_hash_table(pre, &next)) {
        // find a matched vertex on right side
        if (contains_in_hash_table(matching, &next)) {
          put_hash_table(pre, new_id(next), new_id(*cur));
          int *match = get_hash_table(matching, &next);
          put_hash_table(pre, new_id(*match), new_id(next));
          dqueue_push_tail(queue, match);
        } else {
          // find the end of augmenting path
          put_hash_table(pre, new_id(next), new_id(*cur));
          LinkedList *path = track_path(pre, next, id);
          int i = 0;
          int size = list_size(path);
          LinkedListNode *node = head_of_list(path);
          while (i < size) {
            int *v = data_of_node_linked_list(node);
            node = next_node_linked_list(node);
            int *v_next = data_of_node_linked_list(node);
            if (contains_in_hash_table(matching, v)) {
              put_free_exist_hash_table(matching, v, new_id(*v_next), free);
            } else {
              put_hash_table(matching, new_id(*v), new_id(*v_next));
            }
            if (contains_in_hash_table(matching, v_next)) {
              put_free_exist_hash_table(matching, v_next, new_id(*v), free);
            } else {
              put_hash_table(matching, new_id(*v_next), new_id(*v));
            }
            node = next_node_linked_list(node);
            i += 2;
          }
          free_linked_list(path, free);
          free_hashset_iter(iter);
          free_hash_table(pre);
          free_dqueue(queue);
          return 1;
        }
      }
    }
    free_hashset_iter(iter);
  }
  free_hash_table(pre);
  free_dqueue(queue);
  return 0;
}

static LinkedList *get_augmenting_path(Graph *rg, int s, int t) {
  Dqueue *queue = new_dqueue();
  Hashtable *pre = new_hash_table(int_hash, int_compare);
  dqueue_push_tail(queue, &s);
  put_hash_table(pre, &s, &s);
  while (!dqueue_is_empty(queue)) {
    int *q = dqueue_pop_head(queue);
    if (*q == t) break;
    Hashset *adj = get_adj_set(rg, *q);
    if (adj) {
      HashsetIterator *iter = hashset_iterator(adj);
      while (hashset_iter_has_next(iter)) {
        Edge *edge = set_entry_key(hashset_next_entry(iter));
        if (!contains_in_hash_table(pre, &edge->to) && edge->weight > 0) {
          put_hash_table(pre, &edge->to, q);
          dqueue_push_tail(queue, &edge->to);
        }
      }
      free_hashset_iter(iter);
    }
  }
  LinkedList *ret = NULL;
  if (contains_in_hash_table(pre, &t)) {
    ret = new_linked_list();
    int cur = t;
    while (cur != s) {
      prepend_list(ret, new_id(cur));
      cur = *(int *) get_hash_table(pre, &cur);
    }
    prepend_list(ret, new_id(cur));
  }
  free_hash_table(pre);
  free_dqueue(queue);
  return ret;

}
static Graph *create_residual_graph(Graph *graph) {
  Graph *rg = create_graph(1, 1);
  HashtableIterator *iter = hashtable_iterator(graph->represent);
  while (hashtable_iter_has_next(iter)) {
    int *id = table_entry_key(hashtable_next_entry(iter));
    add_graph_data_with_id(rg, *id, NULL);
    Hashset *adj = get_adj_set(graph, *id);
    if (adj) {
      HashsetIterator *iterator = hashset_iterator(adj);
      while (hashset_iter_has_next(iterator)) {
        Edge *edge = set_entry_key(hashset_next_entry(iterator));
        // add to vertex
        add_graph_data_with_id(rg, edge->to, NULL);
        add_edge(rg, edge->from, edge->to, edge->weight);
        // residual edge
        add_edge(rg, edge->to, edge->from, 0);
      }
      free_hashset_iter(iterator);
    }
  }
  free_hashtable_iter(iter);
  return rg;
}

static Hashtable *copy_edges(Graph *graph) {
  Hashtable *copy = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(copy, free, NULL);

  HashtableIterator *iter = hashtable_iterator(graph->edges);
  while (hashtable_iter_has_next(iter)) {
    KVEntry *entry = hashtable_next_entry(iter);
    int *id = table_entry_key(entry);
    Hashset *edges = table_entry_value(entry);
    HashsetIterator *iterator = hashset_iterator(edges);
    Hashset *copy_edge = new_hash_set(default_edge_hash_func, default_edge_equal_func);
    register_hashset_free_functions(copy_edge, free);
    while (hashset_iter_has_next(iterator)) {
      Edge *edge = set_entry_key(hashset_next_entry(iterator));
      Edge *ce = create_edge(edge->from, edge->to, edge->weight);
      put_hash_set(copy_edge, ce);
    }
    free_hashset_iter(iterator);
    put_hash_table(copy, new_id(*id), copy_edge);
  }
  free_hashtable_iter(iter);
  return copy;
}
static int pick_one_id(Graph *graph) {
  HashtableIterator *iter = hashtable_iterator(graph->represent);
  int *id = table_entry_key(hashtable_next_entry(iter));
  free_hashtable_iter(iter);
  return *id;
}

static Edge *pick_one_edge(Hashset *edge) {
  HashsetIterator *iter = hashset_iterator(edge);
  Edge *ret = set_entry_key(hashset_next_entry(iter));
  free_hashset_iter(iter);
  return ret;
}

static int dfs_hamilton_path(Graph *graph, Hashtable *visited, int *end, int id, int pid) {
  put_hash_table(visited, new_id(id), new_id(pid));
  if (size_of_hash_table(visited) == graph->vertex_size) {
    *end = id;
    return 1;
  }
  Hashset *adj = get_adj_set(graph, id);
  if (adj) {
    HashsetIterator *iterator = hashset_iterator(adj);
    while (hashset_iter_has_next(iterator)) {
      Edge *edge = set_entry_key(hashset_next_entry(iterator));
      int is_visited = contains_in_hash_table(visited, &edge->to);
      if (!is_visited) {
        if (dfs_hamilton_path(graph, visited, end, edge->to, id)) {
          free_hashset_iter(iterator);
          return 1;
        }
      }
    }
    free_hashset_iter(iterator);
  }

  free(remove_hash_table(visited, &id));
  return 0;
}

static int dfs_hamilton_loop_path(Graph *graph, Hashtable *visited, int start, int *end, int id, int pid) {
  put_hash_table(visited, new_id(id), new_id(pid));
  if (size_of_hash_table(visited) == graph->vertex_size
      && is_vertex_connected(graph, id, start)) {
    *end = id;
    return 1;
  }
  Hashset *adj = get_adj_set(graph, id);
  if (adj) {
    HashsetIterator *iterator = hashset_iterator(adj);
    while (hashset_iter_has_next(iterator)) {
      Edge *edge = set_entry_key(hashset_next_entry(iterator));
      int is_visited = contains_in_hash_table(visited, &edge->to);
      if (!is_visited) {
        if (dfs_hamilton_loop_path(graph, visited, start, end, edge->to, id)) {
          free_hashset_iter(iterator);
          return 1;
        }
      }
    }

    free_hashset_iter(iterator);
  }
  free(remove_hash_table(visited, &id));
  return 0;
}

static void find_cut_point_ud(Graph *graph,
                              Hashset *visited,
                              Hashtable *ord,
                              Hashtable *low,
                              int visited_count,
                              int id,
                              int pid,
                              LinkedList *result) {
  put_hash_set(visited, new_id(id));
  put_hash_table(ord, new_id(id), new_id(visited_count));
  put_hash_table(low, new_id(id), new_id(visited_count));
  visited_count++;

  Hashset *adj = get_adj_set(graph, id);
  if (adj) {
    int child = 0;

    HashsetIterator *iterator = hashset_iterator(adj);
    while (hashset_iter_has_next(iterator)) {
      Edge *edge = set_entry_key(hashset_next_entry(iterator));
      int is_visited = contains_in_hash_set(visited, &edge->to);
      if (!is_visited) {
        find_cut_point_ud(graph, visited, ord, low, visited_count, edge->to, id, result);
        int *low_v = get_hash_table(low, &id);
        int *low_t = get_hash_table(low, &edge->to);
        if (*low_v > *low_t) {
          *low_v = *low_t;
        }
        // find a cut points
        if (id != pid && *low_t >= *(int *) get_hash_table(ord, &id)) {
          append_list(result, new_id(id));
        }
        child++;
        if (id == pid && child > 1) {
          append_list(result, new_id(id));
        }
      } else if (edge->to != pid) {
        // this is a circle, must not be a bridge
        int *low_v = get_hash_table(low, &id);
        int *low_t = get_hash_table(low, &edge->to);
        if (*low_v > *low_t) {
          *low_v = *low_t;
        }
      }
    }
    free_hashset_iter(iterator);
  }
}

/**
 * find bridge for undirected graph
 *
 * @param graph
 * @param visited
 * @param ord
 * @param low
 * @param visited_count
 * @param id
 * @param pid
 */
static void find_bridge_ud(Graph *graph,
                           Hashset *visited,
                           Hashtable *ord,
                           Hashtable *low,
                           int visited_count,
                           int id,
                           int pid,
                           LinkedList *result) {
  put_hash_set(visited, new_id(id));
  put_hash_table(ord, new_id(id), new_id(visited_count));
  put_hash_table(low, new_id(id), new_id(visited_count));
  visited_count++;

  Hashset *adj = get_adj_set(graph, id);
  if (adj) {
    HashsetIterator *iterator = hashset_iterator(adj);
    while (hashset_iter_has_next(iterator)) {
      Edge *edge = set_entry_key(hashset_next_entry(iterator));
      int is_visited = contains_in_hash_set(visited, &edge->to);
      if (!is_visited) {
        find_bridge_ud(graph, visited, ord, low, visited_count, edge->to, id, result);
        int *low_v = get_hash_table(low, &id);
        int *low_t = get_hash_table(low, &edge->to);
        if (*low_v > *low_t) {
          *low_v = *low_t;
        }
        // find a bridge
        if (*low_t > *(int *) get_hash_table(ord, &id)) {
          append_list(result, edge);
        }
      } else if (edge->to != pid) {
        // this is a circle, must not be a bridge
        int *low_v = get_hash_table(low, &id);
        int *low_t = get_hash_table(low, &edge->to);
        if (*low_v > *low_t) {
          *low_v = *low_t;
        }
      }
    }
    free_hashset_iter(iterator);
  }
}

static void bfs(Graph *graph, Hashset *visited, int id, VertexEntry *vertex_entry) {
  put_hash_set(visited, new_id(id));
  Dqueue *dqueue = new_dqueue();
  dqueue_push_tail(dqueue, new_id(id));
  while (!dqueue_is_empty(dqueue)) {
    int *v = dqueue_pop_head(dqueue);
    int vid = *v;
    vertex_entry->id_list[vertex_entry->size++] = *v;
    free(v);
    Hashset *adj = get_adj_set(graph, vid);
    if (adj) {
      HashsetIterator *iterator = hashset_iterator(adj);
      while (hashset_iter_has_next(iterator)) {
        Edge *edge = set_entry_key(hashset_next_entry(iterator));
        int is_visited = contains_in_hash_set(visited, &edge->to);
        if (!is_visited) {
          dqueue_push_tail(dqueue, new_id(edge->to));
          put_hash_set(visited, new_id(edge->to));
        }
      }
      free_hashset_iter(iterator);
    }
  }
  free_dqueue(dqueue);
}

static int bfs_cmp(Graph *graph, Hashtable *visited, int id, int pid, int target) {
  // TODO
}

static int dfs_bipartite_test(Graph *graph, Hashtable *visited, int id, int color) {
  put_hash_table(visited, new_id(id), new_id(color));
  Hashset *adj = get_adj_set(graph, id);
  if (adj) {
    HashsetIterator *iterator = hashset_iterator(adj);
    while (hashset_iter_has_next(iterator)) {
      Edge *edge = set_entry_key(hashset_next_entry(iterator));
      int is_visited = contains_in_hash_table(visited, &edge->to);
      if (!is_visited) {
        if (!dfs_bipartite_test(graph, visited, edge->to, 1 - color)) {
          free_hashset_iter(iterator);
          return 0;
        }
      } else if (*(int *) get_hash_table(visited, &edge->to) == color) {
        free_hashset_iter(iterator);
        return 0;
      }
    }
    free_hashset_iter(iterator);
  }
  return 1;

}

static int has_circle_undirected_graph(Graph *graph) {
  Hashtable *visited = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(visited, free, free);
  HashtableIterator *iter = hashtable_iterator(graph->represent);

  while (hashtable_iter_has_next(iter)) {
    int *id = table_entry_key(hashtable_next_entry(iter));
    if (!contains_in_hash_table(visited, id)) {
      int ret = dfs_circle_test_undirected(graph, visited, *id, *id);
      if (ret) {
        free_hashtable_iter(iter);
        free_hash_table(visited);
        return 1;
      }
    }
  }

  free_hashtable_iter(iter);
  free_hash_table(visited);
  return 0;
}

static ArrayList *truncate_list(ArrayList *list, int start_element) {
  int index = index_of_arraylist(list, &start_element, int_compare);
  //if (index == -1) return NULL;
  ArrayList *sublist = new_arraylist(0);
  for (int i = index; i < list->size; ++i) {
    append_arraylist(sublist, new_id(*(int *) get_data_arraylist(list, i)));
  }
  return sublist;
}

static LinkedList *track_path(Hashtable *path, int last_element, int start_element) {
  LinkedList *list = new_linked_list();
  prepend_list(list, new_id(last_element));
  while (1) {
    int *pre = get_hash_table(path, &last_element);
    if (*pre == last_element) {
      break;
    }
    if (*pre == start_element) {
      prepend_list(list, new_id(*pre));
      break;
    }
    prepend_list(list, new_id(*pre));
    last_element = *pre;
  }
  return list;
}

static void d_circle_path(Graph *graph, Hashset *visited, Hashtable *on_path, int id, int pid, ArrayList *result) {
  put_free_new_key_hash_set(visited, new_id(id), free);
//  if (!contains_in_hash_set(visited, &id)) {
//    put_hash_set(visited, new_id(id));
//  }
  put_hash_table(on_path, new_id(id), new_id(pid));
  Hashset *adj = get_adj_set(graph, id);

  if (adj) {
    HashsetIterator *iterator = hashset_iterator(adj);
    while (hashset_iter_has_next(iterator)) {
      Edge *edge = set_entry_key(hashset_next_entry(iterator));
      int is_visited = contains_in_hash_table(on_path, &edge->to);
      if (!is_visited) {
        // not visited
        d_circle_path(graph, visited, on_path, edge->to, id, result);
      } else {
        // find a circle
        LinkedList *node = track_path(on_path, id, edge->to);
        append_list(node, new_id(edge->to));
        append_arraylist(result, node);
      }
    }

    free_hashset_iter(iterator);
  }
  free(remove_hash_table(on_path, &id));
}

static void ud_circle_path(Graph *graph,
                           Hashset *visited,
                           Hashtable *path_visited,
                           int id,
                           int pid,
                           ArrayList *result) {
  if (contains_in_hash_set(visited, &id)) {
    put_hash_set(visited, new_id(id));
  }
  put_hash_table(path_visited, new_id(id), new_id(pid));
  Hashset *adj = get_adj_set(graph, id);
  if (adj) {
    HashsetIterator *iterator = hashset_iterator(adj);
    while (hashset_iter_has_next(iterator)) {
      Edge *edge = set_entry_key(hashset_next_entry(iterator));
      int is_visited = contains_in_hash_table(path_visited, &edge->to);
      if (is_visited) {
        if (edge->to != pid) {
          // find a circle
          LinkedList *sublist = track_path(path_visited, id, edge->to);
          append_list(sublist, new_id(edge->to));
          append_arraylist(result, sublist);
        } else {
          continue;
        }
      } else {
        // not visited
        ud_circle_path(graph, visited, path_visited, edge->to, id, result);
      }
    }
    free_hashset_iter(iterator);
  }
  free(remove_hash_table(path_visited, &id));
}

static int has_circle_directed_graph(Graph *graph) {
  Hashtable *visited = new_hash_table(int_hash, int_compare);
  register_hashtable_free_functions(visited, free, free);

  Hashset *on_path = new_hash_set(int_hash, int_compare);
  register_hashset_free_functions(on_path, free);
  HashtableIterator *iter = hashtable_iterator(graph->represent);

  while (hashtable_iter_has_next(iter)) {
    int *id = table_entry_key(hashtable_next_entry(iter));
    if (!contains_in_hash_table(visited, id)) {
      int ret = dfs_circle_test_directed(graph, visited, on_path, *id, *id);
      if (ret) {
        free_hashtable_iter(iter);
        free_hash_table(visited);
        free_hash_set(on_path);
        return 1;
      }
    }
  }

  free_hashtable_iter(iter);
  free_hash_table(visited);
  free_hash_set(on_path);
  return 0;
}

static VertexEntry *new_vertex_entry(int v_size) {
  VertexEntry *vertex_entry = malloc(sizeof(VertexEntry));
  if (!vertex_entry) return NULL;

  int *id_list = malloc(sizeof(int) * v_size);
  if (!id_list) {
    free(vertex_entry);
    return NULL;
  }
  vertex_entry->id_list = id_list;
  vertex_entry->size = 0;
  return vertex_entry;
}

static int *new_id(int value) {
  int *id = malloc(sizeof(int));
  if (!id) return NULL;
  *id = value;
  return id;
}

/**
 * dfs a graph using post order
 * @param graph
 * @param visited
 * @param id
 * @param vertex_entry
 */
static void dfs(Graph *graph, Hashset *visited, int id, VertexEntry *vertex_entry) {
  put_hash_set(visited, new_id(id));

  Hashset *adj = get_adj_set(graph, id);
  if (adj) {
    HashsetIterator *iterator = hashset_iterator(adj);
    while (hashset_iter_has_next(iterator)) {
      Edge *edge = set_entry_key(hashset_next_entry(iterator));
      if (!contains_in_hash_set(visited, &edge->to)) {
        dfs(graph, visited, edge->to, vertex_entry);
      }
    }

    free_hashset_iter(iterator);
  }
  vertex_entry->id_list[vertex_entry->size++] = id;
}

/**
  * dfs with component id
  *
  * @param graph
  * @param visited  hashmap for <vertex id, component id>
  * @param id       vertex id
  * @param cid      component id
  */
static void dfs_cid(Graph *graph, Hashtable *visited, int id, int cid) {
  put_hash_table(visited, new_id(id), new_id(cid));
  Hashset *adj = get_adj_set(graph, id);
  if (adj) {
    HashsetIterator *iterator = hashset_iterator(adj);
    while (hashset_iter_has_next(iterator)) {
      Edge *edge = set_entry_key(hashset_next_entry(iterator));
      if (!contains_in_hash_table(visited, &edge->to)) {
        dfs_cid(graph, visited, edge->to, cid);
      }
    }

    free_hashset_iter(iterator);
  }
}
/**
 * dfs with parent id
 *
 * @param graph
 * @param par
 * @param id
 * @param pid
 */
static void dfs_par(Graph *graph, Hashtable *par, int id, int pid) {
  put_hash_table(par, new_id(id), new_id(pid));
  Hashset *adj = get_adj_set(graph, id);
  if (adj) {
    HashsetIterator *iterator = hashset_iterator(adj);
    while (hashset_iter_has_next(iterator)) {
      Edge *edge = set_entry_key(hashset_next_entry(iterator));
      if (!contains_in_hash_table(par, &edge->to)) {
        dfs_par(graph, par, edge->to, id);
      }
    }

    free_hashset_iter(iterator);
  }
}

static void bfs_par(Graph *graph, Hashtable *par, int id, int pid) {
  put_hash_table(par, new_id(id), new_id(pid));
  Dqueue *dqueue = new_dqueue();
  dqueue_push_tail(dqueue, new_id(id));
  while (!dqueue_is_empty(dqueue)) {
    int *v = dqueue_pop_head(dqueue);
    int p = *v;
    free(v);
    Hashset *adj = get_adj_set(graph, p);
    if (adj) {
      HashsetIterator *iterator = hashset_iterator(adj);
      while (hashset_iter_has_next(iterator)) {
        Edge *edge = set_entry_key(hashset_next_entry(iterator));
        int is_visited = contains_in_hash_table(par, &edge->to);
        if (!is_visited) {
          dqueue_push_tail(dqueue, new_id(edge->to));
          put_hash_table(par, new_id(edge->to), new_id(p));
        }
      }
      free_hashset_iter(iterator);
    }
  }
  free_dqueue(dqueue);
}

/**
 * test if undirected graph has circle
 *
 * @param graph
 * @param visited
 * @param id
 * @param pid
 * @return
 */
static int dfs_circle_test_undirected(Graph *graph, Hashtable *visited, int id, int pid) {
  put_hash_table(visited, new_id(id), new_id(pid));
  Hashset *adj = get_adj_set(graph, id);
  if (adj) {
    HashsetIterator *iterator = hashset_iterator(adj);
    while (hashset_iter_has_next(iterator)) {
      Edge *edge = set_entry_key(hashset_next_entry(iterator));
      int is_visited = contains_in_hash_table(visited, &edge->to);
      if (is_visited && edge->to != pid) {
        free_hashset_iter(iterator);
        return 1;
      } else if (!visited) {
        if (dfs_circle_test_undirected(graph, visited, edge->to, id)) {
          free_hashset_iter(iterator);
          return 1;
        }
      }
    }

    free_hashset_iter(iterator);
  }
  return 0;
}

static int dfs_circle_test_directed(Graph *graph, Hashtable *visited, Hashset *on_path, int id, int pid) {
  put_hash_table(visited, new_id(id), new_id(pid));
  put_hash_set(on_path, new_id(id));
  Hashset *adj = get_adj_set(graph, id);
  if (adj) {
    HashsetIterator *iterator = hashset_iterator(adj);
    while (hashset_iter_has_next(iterator)) {
      Edge *edge = set_entry_key(hashset_next_entry(iterator));
      int is_visited = contains_in_hash_table(visited, &edge->to);
      if (!is_visited) {
        if (dfs_circle_test_directed(graph, visited, on_path, edge->to, id)) {
          free_hashset_iter(iterator);
          return 1;
        }
      } else if (contains_in_hash_set(on_path, &edge->to)) {
        free_hashset_iter(iterator);
        return 1;
      }
    }

    free_hashset_iter(iterator);
  }
  remove_hash_set(on_path, &id);
  return 0;
}
static int dfs_cmp(Graph *graph, Hashtable *par, int id, int pid, int target) {
  put_hash_table(par, new_id(id), new_id(pid));
  if (id == target) {
    return 1;
  }
  Hashset *adj = get_adj_set(graph, id);
  if (adj) {
    HashsetIterator *iterator = hashset_iterator(adj);
    while (hashset_iter_has_next(iterator)) {
      Edge *edge = set_entry_key(hashset_next_entry(iterator));
      if (!contains_in_hash_table(par, &edge->to)) {
        if (dfs_cmp(graph, par, edge->to, id, target)) {
          free_hashset_iter(iterator);
          return 1;
        }
      }
    }

    free_hashset_iter(iterator);
  }
  return 0;
}

static void dfs_visit(Graph *graph, Hashset *visited, int id) {
  put_hash_set(visited, new_id(id));
  Hashset *adj = get_adj_set(graph, id);
  if (adj) {
    HashsetIterator *iterator = hashset_iterator(adj);
    while (hashset_iter_has_next(iterator)) {
      Edge *edge = set_entry_key(hashset_next_entry(iterator));
      if (!contains_in_hash_set(visited, &edge->to)) {
        dfs_visit(graph, visited, edge->to);
      }
    }
    free_hashset_iter(iterator);
  }
}

static void dfs_nr(Graph *graph, Hashset *visited, int id, VertexEntry *vertex_entry) {
  Dqueue *stack = new_dqueue();
  int *vid = new_id(id);
  put_hash_set(visited, vid);
  dqueue_push_head(stack, vid);

  while (!dqueue_is_empty(stack)) {
    int *v = dqueue_pop_head(stack);
    vertex_entry->id_list[vertex_entry->size++] = *v;
    Hashset *adj = get_adj_set(graph, *v);
    if (adj) {

      HashsetIterator *iterator = hashset_iterator(adj);
      while (hashset_iter_has_next(iterator)) {
        Edge *edge = set_entry_key(hashset_next_entry(iterator));
        if (!contains_in_hash_set(visited, &edge->to)) {
          dqueue_push_head(stack, &edge->to);
          put_hash_set(visited, new_id(edge->to));
        }
      }

      free_hashset_iter(iterator);
    }
  }
  free_dqueue(stack);
}

static Edge *create_edge(int from, int to, int weight) {
  Edge *edge = malloc(sizeof(Edge));
  if (!edge) return NULL;
  edge->from = from;
  edge->to = to;
  edge->weight = weight;
  return edge;
}

static DIS *create_dis(int id, int dis) {
  DIS *d = malloc(sizeof(DIS));
  if (!d) return NULL;
  d->id = id;
  d->dis = dis;
  return d;
}

static void free_edges(Hashtable *edges) {
  HashtableIterator *iter = hashtable_iterator(edges);
  while (hashtable_iter_has_next(iter)) {
    Hashset *hashset = table_entry_value(hashtable_next_entry(iter));
    register_hashset_free_functions(hashset, free);
    free_hash_set(hashset);
  }
  free_hashtable_iter(iter);
  free_hash_table(edges);
}

static Hashset *get_or_create_adj_set(Graph *g, Vertex *from_vertex) {
  Hashset *hashset = get_hash_table(g->edges, &from_vertex->id);
  if (!hashset) {
    hashset = new_hash_set(default_edge_hash_func, default_edge_equal_func);
    put_hash_table(g->edges, &from_vertex->id, hashset);
  }
  return hashset;
}

static int add_edge_undirected_unweighted(Graph *g, int from, int to) {
  return add_edge_undirected_weighted(g, from, to, 0);
}

static int add_edge_undirected_weighted(Graph *g, int from, int to, int weight) {
  Vertex *from_vertex = get_hash_table(g->represent, &from);
  if (!from_vertex) {
    return FROM_VERTEX_NOT_EXISTS;
  }
  Vertex *to_vertex = get_hash_table(g->represent, &to);
  if (!to_vertex) {
    return TO_VERTEX_NOT_EXISTS;
  }
  Hashset *hashset1 = get_or_create_adj_set(g, from_vertex);
  Hashset *hashset2 = get_or_create_adj_set(g, to_vertex);
  Edge *from_to_edge = create_edge(from, to, weight);
  if (!from_to_edge) return GRAPH_ERROR;
  // undirected graph need edges for both from-to and to-from
  Edge *to_from_edge = create_edge(to, from, weight);
  if (!to_from_edge) {
    free(from_to_edge);
    return GRAPH_ERROR;
  }
  int ret1 = put_free_new_key_hash_set(hashset1, from_to_edge, free);
  int ret2 = put_free_new_key_hash_set(hashset2, to_from_edge, free);
  if (ret1 == -1 || ret1 == 0) {
    // create new edge
    //free(from_to_edge);
  }
  if (ret2 == -1 || ret2 == 0) {
    //free(to_from_edge);
  }
  if (ret1 == 1 && ret2 == 1) {
    g->edge_size++;
    return GRAPH_SUCCESS;
  }
  if (ret1 == 0 || ret2 == 0) {
    return GRAPH_ERROR;
  }
  // already exists
  return GRAPH_SUCCESS;
}

static int add_edge_directed_unweighted(Graph *g, int from, int to) {
  return add_edge_directed_weighted(g, from, to, 0);
}

static int add_edge_directed_weighted(Graph *g, int from, int to, int weight) {
  Vertex *from_entry = get_hash_table(g->represent, &from);
  if (!from_entry) {
    return FROM_VERTEX_NOT_EXISTS;
  }
  if (!contains_in_hash_table(g->represent, &to)) {
    return TO_VERTEX_NOT_EXISTS;
  }

  Hashset *hashset = get_or_create_adj_set(g, from_entry);

  Edge *from_to_edge = create_edge(from, to, weight);
  if (!from_to_edge) return GRAPH_ERROR;
  int ret = put_free_new_key_hash_set(hashset, from_to_edge, free);
  if (ret == 1) {
    g->edge_size++;
    if (!contains_in_hash_table(g->in_degree, &from)) {
      put_hash_table(g->in_degree, new_id(from), new_id(0));
    }
    if (!contains_in_hash_table(g->out_degree, &from)) {
      put_hash_table(g->out_degree, new_id(from), new_id(0));
    }
    if (!contains_in_hash_table(g->in_degree, &to)) {
      put_hash_table(g->in_degree, new_id(to), new_id(0));
    }
    if (!contains_in_hash_table(g->out_degree, &to)) {
      put_hash_table(g->out_degree, new_id(to), new_id(0));
    }
    int out = *(int *) get_hash_table(g->out_degree, &from);
    put_free_exist_hash_table(g->out_degree, &from, new_id(out + 1), free);

    int in = *(int *) get_hash_table(g->in_degree, &to);
    put_free_exist_hash_table(g->in_degree, &to, new_id(in + 1), free);

    return GRAPH_SUCCESS;
  } else if (ret == 0) {
    //free(from_to_edge);
    return GRAPH_ERROR;
  } else {
    //free(from_to_edge);
    return GRAPH_SUCCESS;
  }
}

static unsigned int default_vertex_hash_func(void *v) {
  if (v == NULL) {
    return 0;
  }
  Vertex *vertex = (Vertex *) v;
  unsigned int hash = 5381;
  unsigned long data_as_long = vertex->data == NULL ? 0 : (unsigned long) (vertex->data);

  // Combine id and data pointer
  hash = ((hash << 5) + hash) + vertex->id;
  hash = ((hash << 5) + hash) + (data_as_long & 0xFFFFFFFF);

  // If on a 64-bit system, also incorporate the higher part of the pointer
  if (sizeof(void *) == 8) {
    hash = ((hash << 5) + hash) + (data_as_long >> 32);
  }

  return hash;
}

static int default_vertex_equal_func(void *v1, void *v2) {
  Vertex *vertex1 = (Vertex *) v1;
  Vertex *vertex2 = (Vertex *) v2;
  return vertex1->id == vertex2->id && vertex1->data == vertex2->data;
}

static unsigned int default_edge_hash_func(void *e) {
  if (e == NULL) return 0;
  Edge *edge = (Edge *) e;
  unsigned int hash = 5381;

  // Combine from and to
  hash = ((hash << 5) + hash) + edge->from;
  hash = ((hash << 5) + hash) + edge->to;

  return hash;
}

static int default_edge_equal_func(void *e1, void *e2) {
  Edge *edge1 = (Edge *) e1;
  Edge *edge2 = (Edge *) e2;
  if (edge1->from == edge2->from && edge1->to == edge2->to) {
    return 0;
  } else if (edge1->from < edge2->from) {
    return -1;
  } else {
    return 1;
  }
}
static int edge_weight_compare(void *e1, void *e2) {
  Edge *edge1 = (Edge *) e1;
  Edge *edge2 = (Edge *) e2;
  if (edge1->weight == edge2->weight) {
    return 0;
  } else if (edge1->weight < edge2->weight) {
    return -1;
  } else {
    return 1;
  }
}

static int edge_weight_compare_pq(void *e1, void *e2) {
  Edge *edge1 = (Edge *) e1;
  Edge *edge2 = (Edge *) e2;
  if (edge1->weight == edge2->weight) {
    return 0;
  } else if (edge1->weight < edge2->weight) {
    return 1;
  } else {
    return -1;
  }
}

static int dis_compare_pq(DIS *d1, DIS *d2) {
  return d2->dis - d1->dis;
}

static int next_id(Graph *g) {
  while (contains_in_hash_table(g->represent, &g->last_continuous_id)) {
    g->last_continuous_id++;
  }
  return g->last_continuous_id;
}

static int remove_edges_from(Hashset *hashset, int from_id) {
  HashsetIterator *iterator = hashset_iterator(hashset);
  int remove_count = 0;
  while (hashset_iter_has_next(iterator)) {
    Edge *edge = set_entry_key(hashset_next_entry(iterator));
    if (edge->from == from_id) {
      int ret = remove_hash_set(hashset, edge);
      if (ret) {
        free(edge);
        remove_count++;
      }
    }
  }
  free_hashset_iter(iterator);
  return remove_count;
}

static int remove_edges_to(Hashset *hashset, int to_id) {
  HashsetIterator *iterator = hashset_iterator(hashset);
  int remove_count = 0;
  while (hashset_iter_has_next(iterator)) {
    Edge *edge = set_entry_key(hashset_next_entry(iterator));
    if (edge->to == to_id) {
      HashsetKeyFreeFunc free_func = get_hashset_free_function(hashset);
      int ret = remove_hash_set(hashset, edge);
      if (ret) {
        if (free_func == NULL) {
          free(edge);
        }
        remove_count++;
      }
    }
  }
  free_hashset_iter(iterator);
  return remove_count;
}

static int remove_edge_from_to(Hashset *hashset, int from_id, int to_id) {
  Edge q = {.from=from_id, .to=to_id};
  Edge *edge = get_key_in_hash_set(hashset, &q);
  int removed = 0;
  if (edge) {
    HashsetKeyFreeFunc free_func = get_hashset_free_function(hashset);
    int ret = remove_hash_set(hashset, edge);

    if (ret == 1) {
      if (free_func == NULL) {
        free(edge);
      }
      removed = 1;
    }
  }
  return removed;
}

//--------------- static functions ----------------------