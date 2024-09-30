//
// Created by ran on 2024/1/25.
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>

#include "help_test/framework.h"
#include "zgraph.h"

Graph *create_test_graph(int size, int directed, int weighted) {
  Graph *graph = create_graph(directed, weighted);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);

  for (int i = 0; i < size; ++i) {
    int id = add_graph_data_with_id(graph, i, NULL);
    assert(id == VERTEX_EXISTS);
  }
  assert(vertex_count(graph) == size);
  if (!directed) {
    for (int i = 0; i < size; ++i) {
      for (int j = 0; j < size; ++j) {
        if (i == j) continue;
        int ret = add_edge(graph, i, j, 0);
        assert(ret == GRAPH_SUCCESS);
        assert(is_vertex_connected(graph, i, j) == 1);
        assert(is_vertex_connected(graph, j, i) == 1);
      }
    }
    assert(edge_count(graph) == size * (size - 1) / 2);
  } else {
    for (int i = 0; i < size; ++i) {
      for (int j = 0; j < size; ++j) {
        if (i == j) continue;
        int ret = add_edge(graph, i, j, 0);
        assert(ret == GRAPH_SUCCESS);
        assert(is_vertex_connected(graph, i, j) == 1);
        if (i < j) {
          assert(is_vertex_connected(graph, j, i) == 0);
        } else {
          assert(is_vertex_connected(graph, j, i) == 1);
        }
      }
    }
    assert(edge_count(graph) == size * (size - 1));
  }
  return graph;
}

Graph *create_test_graph_unconnected(int size, int directed, int weighted) {
  Graph *graph = create_graph(directed, weighted);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);

  for (int i = 0; i < size; ++i) {
    int id = add_graph_data_with_id(graph, i, NULL);
    assert(id == VERTEX_EXISTS);
  }
  assert(vertex_count(graph) == size);
  if (!directed) {
    for (int i = 0; i < size - 1; i = i + 2) {
      int ret = add_edge(graph, i, i + 1, 0);
      assert(ret == GRAPH_SUCCESS);
      assert(is_vertex_connected(graph, i, i + 1) == 1);
      assert(is_vertex_connected(graph, i + 1, i) == 1);
    }
    assert(edge_count(graph) == size / 2);
  } else {
    for (int i = 0; i < size - 1; i = i + 2) {
      int ret = add_edge(graph, i, i + 1, 0);
      assert(ret == GRAPH_SUCCESS);
      assert(is_vertex_connected(graph, i, i + 1) == 1);
      assert(is_vertex_connected(graph, i + 1, i) == 0);
    }
  }
  assert(edge_count(graph) == size / 2);
  return graph;
}

void test_create_graph_undirected_unweighted() {
  Graph *graph = create_test_graph(100, 0, 0);
  free_graph(graph);
}

void test_create_graph_directed_unweighted() {
  Graph *graph = create_test_graph(100, 1, 0);
  free_graph(graph);
}

void test_create_graph_undirected_weighted() {
  Graph *graph = create_test_graph(100, 0, 1);
  free_graph(graph);
}

void test_create_graph_directed_weighted() {
  Graph *graph = create_test_graph(100, 1, 1);
  free_graph(graph);
}

void test_remove_vertex_directed_graph() {
  int size = 100;
  Graph *graph = create_test_graph(size, 1, 1);

  assert(edge_count(graph) == size * (size - 1));
  int edge_size = edge_count(graph);
  for (int i = 0; i < size; ++i) {
    int ret = remove_vertex(graph, i);
    assert(ret == 1);
    assert(vertex_count(graph) == size - i - 1);
    edge_size -= 2 * (size - i - 1);
    assert(edge_count(graph) == edge_size);
  }
  free_graph(graph);
}

void test_remove_vertex_undirected_graph() {
  int size = 100;
  Graph *graph = create_test_graph(size, 0, 1);

  int edge_size = edge_count(graph);
  for (int i = 0; i < size; ++i) {
    int ret = remove_vertex(graph, i);
    assert(ret == 1);
    assert(vertex_count(graph) == size - i - 1);
    edge_size -= 1 * (size - i - 1);
    assert(edge_count(graph) == edge_size);
  }
  free_graph(graph);
}

void test_remove_edge_undirected_graph() {
  int size = 100;
  Graph *graph = create_graph(0, 1);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data_with_id(graph, i, NULL);
    assert(id == VERTEX_EXISTS);
  }
  assert(vertex_count(graph) == size);
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      if (i == j) continue;
      int ret = add_edge(graph, i, j, i + j);
      assert(ret == GRAPH_SUCCESS);
      assert(is_vertex_connected(graph, i, j) == 1);
      assert(is_vertex_connected(graph, j, i) == 1);
    }
  }
  assert(edge_count(graph) == size * (size - 1) / 2);

  int edge_size = edge_count(graph);
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      if (i == j) continue;
      int ret = remove_edge(graph, i, j);
      if (i < j) {
        assert(ret == 2);
        edge_size--;
      } else {
        assert(ret == 0);
      }
      assert(is_vertex_connected(graph, i, j) == 0);
    }
  }
  assert(edge_size == edge_count(graph));
  free_graph(graph);
}

void test_remove_edge_directed_graph() {
  int size = 100;
  Graph *graph = create_test_graph(size, 1, 1);

  int edge_size = edge_count(graph);
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      if (i == j) continue;
      int ret = remove_edge(graph, i, j);
      assert(ret == 1);
      edge_size--;
      assert(is_vertex_connected(graph, i, j) == 0);
    }
  }
  assert(edge_size == edge_count(graph));
  free_graph(graph);
}

void test_dfs_undirected() {
  int size = 100;
  Graph *graph = create_test_graph(size, 0, 1);

  VertexEntry *v = dfs_graph(graph);
  assert(v->size == size);
  for (int i = 0; i < v->size; ++i) {
    assert(has_vertex(graph, v->id_list[i]) == 1);
  }
  free_vertex_entry(v);
  free_graph(graph);
}

void test_dfs_directed() {
  clock_t start, end;
  double cpu_time_used;
  int size = 100;
  Graph *graph = create_test_graph(size, 1, 1);
  assert(edge_count(graph) == size * (size - 1));
  start = clock();
  VertexEntry *v = dfs_graph(graph);
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("test_dfs_directed time: %f seconds\n", cpu_time_used);
  assert(v->size == size);
//  for (int i = 0; i < v->size; ++i) {
//    assert(has_vertex(graph, v->id_list[i]) == 1);
//  }
  free_vertex_entry(v);
  free_graph(graph);
}

void test_dfs_nr_undirected() {
  int size = 100;
  Graph *graph = create_test_graph(size, 0, 1);

  VertexEntry *v = dfs_graph_nr(graph);
  assert(v->size == size);
  for (int i = 0; i < v->size; ++i) {
    assert(has_vertex(graph, v->id_list[i]) == 1);
  }
  free_vertex_entry(v);
  free_graph(graph);
}

void test_dfs_nr_directed() {
  clock_t start, end;
  double cpu_time_used;
  int size = 100;
  Graph *graph = create_test_graph(size, 1, 1);

  assert(edge_count(graph) == size * (size - 1));
  start = clock();
  VertexEntry *v = dfs_graph_nr(graph);
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("test_dfs_nr_directed time: %f seconds\n", cpu_time_used);
  assert(v->size == size);
//  for (int i = 0; i < v->size; ++i) {
//    assert(has_vertex(graph, v->id_list[i]) == 1);
//  }
  free_vertex_entry(v);
  free_graph(graph);
}

void test_cc_count() {
  int size = 100;
  Graph *graph = create_test_graph(size, 1, 1);
  int c = component_count(graph);
  assert(c == 1);
  free_graph(graph);

  graph = create_test_graph_unconnected(size, 1, 1);
  c = component_count(graph);
  assert(c == size / 2);
  free_graph(graph);

  graph = create_test_graph_unconnected(size, 0, 1);
  c = component_count(graph);
  assert(c == size / 2);
  free_graph(graph);
}

void test_graph_component() {
  int size = 100;
  Graph *graph = create_test_graph(size, 1, 1);
  int c = component_count(graph);
  assert(c == 1);
  Hashtable *cmap = graph_components(graph);
  assert(size_of_hash_table(cmap) == 1);
  int vsize = 0;
  for (int i = 0; i < size_of_hash_table(cmap); ++i) {
    ArrayList *al = get_hash_table(cmap, &i);
    vsize += al->size;
    for (int j = 0; j < al->size; ++j) {
      int *id = get_data_arraylist(al, j);
      assert(has_vertex(graph, *id));
      free(id);
    }
  }
  assert(vsize == size);
  free_hash_table(cmap);
  free_graph(graph);

  // more components
  vsize = 0;
  graph = create_test_graph_unconnected(size, 1, 1);
  c = component_count(graph);
  assert(c == size / 2);
  cmap = graph_components(graph);
  assert(size_of_hash_table(cmap) == size / 2);
  HashtableEntrySet *e = hashtable_entry_set(cmap);
  for (int i = 0; i < e->size; ++i) {
    ArrayList *al = table_entry_value(e->entry_set[i]);
    vsize += al->size;
    for (int j = 0; j < al->size; ++j) {
      int *id = get_data_arraylist(al, j);
      assert(has_vertex(graph, *id));
      free(id);
    }
  }
  free_hashtable_entry_set(e);
  free_hash_table(cmap);
  free_graph(graph);

}

void print_ll(LinkedListNode *list_node) {
  printf("[ ");
  LinkedListNode *head = list_node;
  do {
    int *data = data_of_node_linked_list(list_node);
    printf("%d,", *data);
    list_node = next_node_linked_list(list_node);
  } while (list_node != head);

  printf("]\n");
  fflush(stdout);
}

void print_al(ArrayList *list) {
  printf("[");
  for (int i = 0; i < list->size; ++i) {
    int *data = get_data_arraylist(list, i);
    printf("%d,", *data);
  }
  printf("]\n");
  fflush(stdout);
}

void test_single_source_path_bfs() {
  int size = 100;
  Graph *graph = create_graph(1, 1);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  /**
   *  has circle for undirected graph but no circle for directed graph:
   *
   *            0 -- 1 --- 2 - 4
   *                 |\   /|
   *                 | \ / |
   *                 |  3 |
   *                  \  /
   *                   5
   *                   \
   *                   10 - 16
   *                    \  /
   *                     18
   */

  add_edge(graph, 0, 1, 0);
  add_edge(graph, 1, 2, 0);
  add_edge(graph, 2, 4, 0);
  add_edge(graph, 1, 3, 0);
  add_edge(graph, 3, 2, 0);
  add_edge(graph, 1, 5, 0);
  add_edge(graph, 5, 2, 0);

  add_edge(graph, 10, 16, 0);
  add_edge(graph, 10, 18, 0);
  add_edge(graph, 16, 18, 0);

  add_edge(graph, 5, 10, 0);
  add_edge(graph, 4, 16, 0);

  Hashtable *pre_map = single_source_path(graph, 0, BFS);
  assert(pre_map != NULL);
  HashtableIterator *iter = hashtable_iterator(pre_map);
  while (hashtable_iter_has_next(iter)) {
    KVEntry *entry = hashtable_next_entry(iter);
    printf("{%d : %d}\n",
           *(int *) table_entry_key(entry),
           *(int *) table_entry_value(entry));
  }

  free_hashtable_iter(iter);
  fflush(stdout);
  LinkedList *p = single_source_path_to(pre_map, 5);
  assert(p != NULL);
  print_ll(head_of_list(p));
  printf("path length: %d\n", list_size(p));
  fflush(stdout);
  free_linked_list(p, free);

  p = single_source_path_to(pre_map, 18);
  assert(p != NULL);
  print_ll(head_of_list(p));
  printf("path length: %d\n", list_size(p));
  fflush(stdout);
  free_linked_list(p, free);
  free_hash_table(pre_map);
  free_graph(graph);

}
void test_single_source_path_dfs() {
  int size = 100;
  Graph *graph = create_test_graph(size, 1, 1);
  Hashtable *pre_map = single_source_path(graph, 0, DFS);
  assert(pre_map != NULL);
  HashtableIterator *iter = hashtable_iterator(pre_map);
  while (hashtable_iter_has_next(iter)) {
    KVEntry *entry = hashtable_next_entry(iter);
    printf("{%d : %d}\n",
           *(int *) table_entry_key(entry),
           *(int *) table_entry_value(entry));
  }

  free_hashtable_iter(iter);

  fflush(stdout);
  LinkedList *p = single_source_path_to(pre_map, 42);
  assert(p != NULL);
  print_ll(head_of_list(p));
  printf("path length: %d\n", list_size(p));
  fflush(stdout);
  free_linked_list(p, free);
  free_hash_table(pre_map);
  free_graph(graph);

  graph = create_test_graph(size, 0, 1);
  pre_map = single_source_path(graph, 0, DFS);
  assert(pre_map != NULL);
  p = NULL;
  p = single_source_path_to(pre_map, 87);
  assert(p != NULL);
  print_ll(head_of_list(p));
  printf("path length: %d\n", list_size(p));
  fflush(stdout);
  free_linked_list(p, free);
  free_hash_table(pre_map);
  free_graph(graph);
}

void test_has_path() {
  int size = 100;
  Graph *graph = create_test_graph(size, 1, 1);
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      if (i == j) {
        assert(has_path(graph, i, j) == 0);
      } else {
        assert(has_path(graph, i, j) == 1);
      }
    }
  }
  free_graph(graph);
  graph = create_test_graph(size, 0, 1);
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      if (i == j) {
        assert(has_path(graph, i, j) == 0);
      } else {
        assert(has_path(graph, i, j) == 1);
      }
    }
  }
  free_graph(graph);
}

void test_one_path() {
  int size = 100;
  Graph *graph = create_test_graph(size, 1, 1);
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      if (i == j) {
        assert(one_path(graph, i, j) == NULL);
      } else {
        LinkedList *path = one_path(graph, i, j);
        // print_ll(head_of_list(path));
        free_linked_list(path, free);
      }
    }
  }
  free_graph(graph);

  graph = create_test_graph(size, 0, 1);
  for (int i = 0; i < size; i += 5) {
    for (int j = 0; j < size; j += 5) {
      if (i == j) {
        assert(one_path(graph, i, j) == NULL);
      } else {
        LinkedList *path = one_path(graph, i, j);
        //print_ll(head_of_list(path));
        free_linked_list(path, free);
      }
    }
  }
  free_graph(graph);

}

void test_has_circle() {
  int size = 100;
  Graph *graph = create_test_graph(size, 0, 1);
  int ret = has_circle(graph);
  assert(ret == 1);
  free_graph(graph);

  graph = create_graph(0, 0);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);
  ret = has_circle(graph);
  assert(ret == 0);

  /**
   *  has circle for undirected graph but no circle for directed graph:
   *
   *            0 -> 1 -> 2 -> 4
   *                 \    ^
   *                  \  /
   *                   >3
   *
   */
//  add_edge(graph, 8, 0, 0);
//  add_edge(graph, 0, 1, 0);
  add_edge(graph, 1, 2, 0);
  add_edge(graph, 3, 1, 0);
  add_edge(graph, 2, 3, 0);
  ret = has_circle(graph);
  assert(ret == 1);
  free_graph(graph);

  graph = create_test_graph(size, 1, 1);
  ret = has_circle(graph);
  assert(ret == 1);
  free_graph(graph);

  graph = create_graph(1, 0);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);

  ret = has_circle(graph);
  assert(ret == 0);

  /**
   *             ->
   *          1     2
   *             <-
   */

  add_edge(graph, 1, 2, 0);
  add_edge(graph, 2, 1, 0);

  assert(has_circle(graph));

  free_graph(graph);

  graph = create_graph(1, 0);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);

  ret = has_circle(graph);
  assert(ret == 0);

  /**
   *
   *
   *             0->1->2->4
   *                | >
   *                >3
   *
   */

  add_edge(graph, 0, 1, 0);
  add_edge(graph, 1, 2, 0);
  add_edge(graph, 2, 4, 0);
  add_edge(graph, 1, 3, 0);
  add_edge(graph, 3, 2, 0);
  assert(!has_circle(graph));
  add_edge(graph, 4, 3, 0);
  assert(has_circle(graph));
  free_graph(graph);
}

void test_enumerate_circle_path() {
  int size = 20;

  // undirected graph
  Graph *graph = create_graph(0, 0);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);
  /**
   *  has circle for undirected graph but no circle for directed graph:
   *
   *            0 -- 1 --- 2 - 4
   *                 |\   /|
   *                 | \ / |
   *                 |  3 |
   *                  \  /
   *                   5
   *
   *                   10 - 16
   *                    \  /
   *                     18
   */
  add_edge(graph, 0, 1, 0);
  add_edge(graph, 1, 2, 0);
  add_edge(graph, 2, 4, 0);
  add_edge(graph, 1, 3, 0);
  add_edge(graph, 3, 2, 0);
  add_edge(graph, 1, 5, 0);
  add_edge(graph, 5, 2, 0);

  add_edge(graph, 10, 16, 0);
  add_edge(graph, 10, 18, 0);
  add_edge(graph, 16, 18, 0);

  add_edge(graph, 5, 10, 0);
  add_edge(graph, 4, 16, 0);
  ArrayList *paths = enumerate_circle_path(graph);
  for (int i = 0; i < paths->size; ++i) {
    LinkedList *path = get_data_arraylist(paths, i);
    print_ll(head_of_list(path));

    free_linked_list(path, free);
  }
  free_arraylist(paths);
  free_graph(graph);

  // directed graph
  printf("------\n");
  // undirected graph
  graph = create_graph(1, 0);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);
  /**
   *  has circle for undirected graph but no circle for directed graph:
   *
   *            0 -- 1 --- 2 - 4
   *                 |\   /|
   *                 | \ / |
   *                 |  3 |
   *                  \  /
   *                   5
   *
   *                   10 - 16
   *                    \  /
   *                     18
   */

  add_edge(graph, 0, 1, 0);
  add_edge(graph, 1, 2, 0);
  add_edge(graph, 2, 4, 0);
  add_edge(graph, 2, 3, 0);
  add_edge(graph, 3, 1, 0);

  add_edge(graph, 2, 5, 0);
  add_edge(graph, 5, 1, 0);

  paths = enumerate_circle_path(graph);
  for (int i = 0; i < paths->size; ++i) {
    LinkedList *path = get_data_arraylist(paths, i);
    print_ll(head_of_list(path));
    free_linked_list(path, free);
  }
  free_arraylist(paths);
  free_graph(graph);

}

void test_bipartite() {
  int size = 20;
  Graph *graph = create_graph(0, 0);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);
  assert(is_bipartite(graph));
  /**
   *    0---1
   */
  add_edge(graph, 0, 1, 0);
  assert(is_bipartite(graph));

  /**
   *    0----1
   *    |
   *    2
   *
   */
  add_edge(graph, 0, 2, 0);
  assert(is_bipartite(graph));

  /**
   *    0----1
   *    |
   *    2---3
   *
   */
  add_edge(graph, 2, 3, 0);
  assert(is_bipartite(graph));

  /**
   *    0----1
   *    |   |
   *    2---3
   *
   */
  add_edge(graph, 3, 1, 0);
  assert(is_bipartite(graph));

  /**
   *    0----1
   *    | / |
   *    2---3
   *
   */
  add_edge(graph, 2, 1, 0);
  assert(!is_bipartite(graph));

  free_graph(graph);
}

void test_bfs() {
  int size = 100;
  Graph *graph = create_test_graph(size, 0, 1);

  VertexEntry *v = bfs_graph(graph);
  assert(v->size == size);
  for (int i = 0; i < v->size; ++i) {
    assert(has_vertex(graph, v->id_list[i]) == 1);
  }
  free_vertex_entry(v);
  free_graph(graph);

  graph = create_test_graph(size, 1, 1);

  v = bfs_graph(graph);
  assert(v->size == size);
  for (int i = 0; i < v->size; ++i) {
    assert(has_vertex(graph, v->id_list[i]) == 1);
  }
  free_vertex_entry(v);
  free_graph(graph);
}

void test_find_bridge() {
  int size = 20;
  Graph *graph = create_graph(0, 0);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);
  /**
   *
  *            0 -- 1 --- 2 - 4
  *                  \   /
  *                   \ /
  *                    3
  *                    |
  *                   10 - 16
  *                    \  /
  *                     18
   *
   *    this graph has 3 bridge:
   *    <0-1>, <2-4>, <3-10>
  */
  add_edge(graph, 0, 1, 0);
  add_edge(graph, 1, 2, 0);
  add_edge(graph, 2, 4, 0);
  add_edge(graph, 1, 3, 0);
  add_edge(graph, 2, 3, 0);

  add_edge(graph, 10, 16, 0);
  add_edge(graph, 10, 18, 0);
  add_edge(graph, 16, 18, 0);

  add_edge(graph, 3, 10, 0);

  LinkedList *result = find_bridge(graph);
  assert(list_size(result) == 3);

  LinkedListNode *head = head_of_list(result);
  LinkedListNode *list_node = head;
  do {
    Edge *data = data_of_node_linked_list(list_node);
    printf("<%d-%d>\n", get_edge_from(data), get_edge_to(data));
    list_node = next_node_linked_list(list_node);
  } while (list_node != head);

  fflush(stdout);
  free_linked_list(result, NULL);
  free_graph(graph);
}

void test_find_cut_points() {
  int size = 20;
  Graph *graph = create_graph(0, 0);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);
  /**
   *
  *            0 -- 1 --- 2 - 4
  *                  \   /
  *                   \ /
  *                    3
  *                    |
  *                   10 - 16
  *                    \  /
  *                     18
   *
   *    this graph has 4 cut points:
   *    2,10,3,1
  */
  add_edge(graph, 0, 1, 0);
  add_edge(graph, 1, 2, 0);
  add_edge(graph, 2, 4, 0);
  add_edge(graph, 1, 3, 0);
  add_edge(graph, 2, 3, 0);

  add_edge(graph, 10, 16, 0);
  add_edge(graph, 10, 18, 0);
  add_edge(graph, 16, 18, 0);

  add_edge(graph, 3, 10, 0);

  LinkedList *result = find_cut_point(graph);
  assert(list_size(result) == 4);

  LinkedListNode *head = head_of_list(result);
  LinkedListNode *list_node = head;
  do {
    int *data = data_of_node_linked_list(list_node);
    printf("<%d>\n", *data);
    list_node = next_node_linked_list(list_node);
  } while (list_node != head);

  fflush(stdout);
  free_linked_list(result, free);
  free_graph(graph);
}

void test_hamilton_loop_path() {
  int size = 8;
  Graph *graph = create_graph(0, 0);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);
  /**
   *
  *            0 -- 1 --- 2 - 4
  *              \   \   /   |
  *               \   \ /   |
  *                --- 3   |
  *                   |   |
  *                   5 - 6
  *                    \  /
  *                     7
   *
  */
  add_edge(graph, 0, 1, 0);
  add_edge(graph, 1, 2, 0);
  add_edge(graph, 2, 4, 0);
  add_edge(graph, 1, 3, 0);
  add_edge(graph, 2, 3, 0);

  add_edge(graph, 5, 6, 0);
  add_edge(graph, 6, 7, 0);
  add_edge(graph, 5, 7, 0);

  add_edge(graph, 3, 5, 0);
  add_edge(graph, 6, 4, 0);
  add_edge(graph, 0, 3, 0);

  LinkedList *list = hamilton_loop_path(graph);
  assert(list != NULL);
  print_ll(head_of_list(list));
  free_linked_list(list, free);
  free_graph(graph);
}

void test_hamilton_path() {
  int size = 8;
  Graph *graph = create_graph(0, 0);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);
  /**
   *
  *            0 -- 1 --- 2 - 4
  *              \   \   /   |
  *               \   \ /   |
  *                --- 3   |
  *                   |   |
  *                   5 - 6
  *                    \  /
  *                     7
   *

  */
  add_edge(graph, 0, 1, 0);
  add_edge(graph, 1, 2, 0);
  add_edge(graph, 2, 4, 0);
  add_edge(graph, 1, 3, 0);
  add_edge(graph, 2, 3, 0);

  add_edge(graph, 5, 6, 0);
  add_edge(graph, 6, 7, 0);
  add_edge(graph, 5, 7, 0);

  add_edge(graph, 3, 5, 0);
  add_edge(graph, 6, 4, 0);
  add_edge(graph, 0, 3, 0);

  LinkedList *list = hamilton_path(graph, 0);
  assert(list != NULL);
  print_ll(head_of_list(list));
  free_linked_list(list, free);
  free_graph(graph);

  graph = create_graph(0, 0);
  for (int i = 0; i < 4; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == 4);
  /**
   *     0    3
   *     | \ |
   *    2 -- 1
   *
   */
  add_edge(graph, 0, 1, 0);
  add_edge(graph, 0, 2, 0);
  add_edge(graph, 1, 2, 0);
  add_edge(graph, 3, 1, 0);

  list = hamilton_path(graph, 0);
  assert(list != NULL);
  print_ll(head_of_list(list));
  free_linked_list(list, free);

  list = hamilton_path(graph, 1);
  assert(list == NULL);
  free_graph(graph);
}

void test_has_euler_loop() {
  int size = 5;
  Graph *graph = create_graph(0, 0);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);

  /**
   *    0      3
   *    | \  / |
   *    |  2   |
   *    | / \ |
   *    1    4
   *
   */
  add_edge(graph, 0, 1, 0);
  add_edge(graph, 0, 2, 0);
  add_edge(graph, 1, 2, 0);
  add_edge(graph, 2, 3, 0);
  add_edge(graph, 2, 4, 0);
  add_edge(graph, 3, 4, 0);
  assert(has_euler_loop(graph));
  add_edge(graph, 0, 3, 0);
  assert(!has_euler_loop(graph));
  free_graph(graph);

  // directed graph
  graph = create_graph(1, 0);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);
  /**
   * ↗↘↖↙
   *    0----->1----->2
   *           |    ↗ |
   *           |  /   |
   *           ↓/     ↓
   *           3      4
   */

  add_edge(graph, 0, 1, 0);
  add_edge(graph, 1, 2, 0);
  add_edge(graph, 1, 3, 0);
  add_edge(graph, 3, 2, 0);
  add_edge(graph, 2, 4, 0);
  assert(!has_euler_loop(graph));
  free_graph(graph);

  graph = create_graph(1, 0);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);
  /**
   * ↗↘↖↙
   *
   *         0
   *       /  ↖
   *     /     \
   *    ↙       \
   *   1-------->2
   *   ↗\       ↗|
   *  |  \    /  |
   *  \  |  /    |
   *   \↙ /     ↙
   *    3<-----4
   *
   *
   */
  add_edge(graph, 0, 1, 0);
  add_edge(graph, 2, 0, 0);
  add_edge(graph, 1, 2, 0);
  add_edge(graph, 1, 3, 0);
  add_edge(graph, 3, 1, 0);
  add_edge(graph, 3, 2, 0);
  add_edge(graph, 4, 3, 0);
  add_edge(graph, 2, 4, 0);
  assert(has_euler_loop(graph));
  free_graph(graph);
}

void test_hierholzer_euler_loop() {
  int size = 5;
  Graph *graph = create_graph(0, 0);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);

  /**
   *    0      3
   *    | \  / |
   *    |  2   |
   *    | / \ |
   *    1    4
   *
   */
  add_edge(graph, 0, 1, 0);
  add_edge(graph, 0, 2, 0);
  add_edge(graph, 1, 2, 0);
  add_edge(graph, 2, 3, 0);
  add_edge(graph, 2, 4, 0);
  add_edge(graph, 3, 4, 0);
  assert(has_euler_loop(graph));
  LinkedList *ret = hierholzer_euler_loop(graph);
  assert(ret != NULL);
  print_ll(head_of_list(ret));

  free_linked_list(ret, free);
  add_edge(graph, 0, 3, 0);
  ret = hierholzer_euler_loop(graph);
  assert(ret == NULL);
  free_graph(graph);

  graph = create_graph(1, 0);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);
  /**
   * ↗↘↖↙
   *
   *         0
   *       /  ↖
   *     /     \
   *    ↙       \
   *   1-------->2
   *   ↗\       ↗|
   *  |  \    /  |
   *  \  |  /    |
   *   \↙ /     ↙
   *    3<-----4
   *
   *
   */
  add_edge(graph, 0, 1, 0);
  add_edge(graph, 2, 0, 0);
  add_edge(graph, 1, 2, 0);
  add_edge(graph, 1, 3, 0);
  add_edge(graph, 3, 1, 0);
  add_edge(graph, 3, 2, 0);
  add_edge(graph, 4, 3, 0);
  add_edge(graph, 2, 4, 0);
  ret = hierholzer_euler_loop(graph);
  assert(ret != NULL);
  print_ll(head_of_list(ret));

  free_linked_list(ret, free);
  free_graph(graph);
}

void test_kruskal() {
  int size = 7;
  Graph *graph = create_graph(0, 1);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);

  /**
   *
   */
  add_edge(graph, 0, 1, 2);
  add_edge(graph, 0, 5, 2);
  add_edge(graph, 0, 3, 7);
  add_edge(graph, 1, 2, 1);
  add_edge(graph, 1, 5, 5);
  add_edge(graph, 2, 5, 4);
  add_edge(graph, 2, 4, 4);
  add_edge(graph, 1, 3, 4);
  add_edge(graph, 1, 4, 3);
  add_edge(graph, 3, 4, 1);
  add_edge(graph, 3, 6, 5);
  add_edge(graph, 4, 6, 7);

  LinkedList *mst = kruskal_mst(graph);
  assert(mst != NULL);
  LinkedListNode *list_node = head_of_list(mst);
  LinkedListNode *head = list_node;
  do {
    Edge *data = data_of_node_linked_list(list_node);
    printf("[%d-%d]: %d, ", get_edge_from(data), get_edge_to(data), get_edge_weight(data));
    list_node = next_node_linked_list(list_node);
  } while (list_node != head);
  printf("\n");
  fflush(stdout);
  free_linked_list(mst, free);
  free_graph(graph);
}

void test_prim() {
  int size = 7;
  Graph *graph = create_graph(0, 1);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);

  /**
   *
   */
  add_edge(graph, 0, 1, 2);
  add_edge(graph, 0, 5, 2);
  add_edge(graph, 0, 3, 7);
  add_edge(graph, 1, 2, 1);
  add_edge(graph, 1, 5, 5);
  add_edge(graph, 2, 5, 4);
  add_edge(graph, 2, 4, 4);
  add_edge(graph, 1, 3, 4);
  add_edge(graph, 1, 4, 3);
  add_edge(graph, 3, 4, 1);
  add_edge(graph, 3, 6, 5);
  add_edge(graph, 4, 6, 7);

  LinkedList *mst = prim_mst(graph);
  assert(mst != NULL);
  LinkedListNode *list_node = head_of_list(mst);
  LinkedListNode *head = list_node;
  do {
    Edge *data = data_of_node_linked_list(list_node);
    printf("[%d-%d]: %d, ", get_edge_from(data), get_edge_to(data), get_edge_weight(data));
    list_node = next_node_linked_list(list_node);
  } while (list_node != head);
  printf("\n");
  fflush(stdout);
  free_linked_list(mst, free);
  free_graph(graph);
}

void test_dijkstra() {
  int size = 5;
  Graph *graph = create_graph(0, 1);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);
  /**
   *
   *             [1] * * -2- * * [3]
   *           *  * *           * *
   *        -4-   *   *       -4- *
   *      *       *   -3-   *     *
   *   [0]       -1-     \ /     -1-
   *      *       *      / \      *
   *        -2-   *    *     *    *
   *           *  *  *         *  *
   *             [2] * * -5- * * [4]
   *
   *
   */
  add_edge(graph, 0, 1, 4);
  add_edge(graph, 0, 2, 2);
  add_edge(graph, 1, 2, 1);
  add_edge(graph, 1, 3, 2);
  add_edge(graph, 1, 4, 3);
  add_edge(graph, 2, 3, 4);
  add_edge(graph, 2, 4, 5);
  add_edge(graph, 3, 4, 1);

  Hashtable *dis = dijkstra(graph, 0);

  HashtableEntrySet *entry_set = hashtable_entry_set(dis);
  for (int i = 0; i < entry_set->size; ++i) {
    printf("{%d : %d}\n",
           *(int *) table_entry_key(entry_set->entry_set[i]),
           *(int *) table_entry_value(entry_set->entry_set[i]));
  }
  fflush(stdout);
  free_hashtable_entry_set(entry_set);
  free_hash_table(dis);
  free_graph(graph);
}

static int *new_id(int value) {
  int *id = malloc(sizeof(int));
  if (!id) return NULL;
  *id = value;
  return id;
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

void test_dijkstra_path() {
  int size = 5;
  Graph *graph = create_graph(0, 1);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);

  /**
   *
   *             [1] * * -2- * * [3]
   *           *  * *           * *
   *        -4-   *   *       -4- *
   *      *       *   -3-   *     *
   *   [0]       -1-     \ /     -1-
   *      *       *      / \      *
   *        -2-   *    *     *    *
   *           *  *  *         *  *
   *             [2] * * -5- * * [4]
   *
   *
   */
  add_edge(graph, 0, 1, 4);
  add_edge(graph, 0, 2, 2);
  add_edge(graph, 1, 2, 1);
  add_edge(graph, 1, 3, 2);
  add_edge(graph, 1, 4, 3);
  add_edge(graph, 2, 3, 4);
  add_edge(graph, 2, 4, 5);
  add_edge(graph, 3, 4, 1);

  Hashtable *pre = NULL;
  Hashtable *dis = dijkstra_path(graph, 0, &pre);

  HashtableEntrySet *entry_set = hashtable_entry_set(dis);
  for (int i = 0; i < entry_set->size; ++i) {
    printf("{%d : %d}\n",
           *(int *) table_entry_key(entry_set->entry_set[i]),
           *(int *) table_entry_value(entry_set->entry_set[i]));
  }

  for (int i = 1; i < size; ++i) {
    LinkedList *p = track_path(pre, i, 0);
    printf("shortest path 0-%d: ", i);
    print_ll(head_of_list(p));
    printf("\n");
    free_linked_list(p, free);
  }
  fflush(stdout);

  free_hashtable_entry_set(entry_set);
  free_hash_table(dis);
  free_graph(graph);
  free_hash_table(pre);
}

void test_dijkstra_path_to() {
  printf("---\n");
  int size = 5;
  Graph *graph = create_graph(0, 1);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);

  /**
   *
   *             [1] * * -2- * * [3]
   *           *  * *           * *
   *        -4-   *   *       -4- *
   *      *       *   -3-   *     *
   *   [0]       -1-     \ /     -1-
   *      *       *      / \      *
   *        -2-   *    *     *    *
   *           *  *  *         *  *
   *             [2] * * -5- * * [4]
   *
   *
   */
  add_edge(graph, 0, 1, 4);
  add_edge(graph, 0, 2, 2);
  add_edge(graph, 1, 2, 1);
  add_edge(graph, 1, 3, 2);
  add_edge(graph, 1, 4, 3);
  add_edge(graph, 2, 3, 4);
  add_edge(graph, 2, 4, 5);
  add_edge(graph, 3, 4, 1);

  for (int i = 1; i < size; ++i) {
    LinkedList *p = dijkstra_path_to(graph, 0, i);
    printf("shortest path 0-%d: ", i);
    print_ll(head_of_list(p));
    printf("\n");
    free_linked_list(p, free);
  }
  fflush(stdout);
  free_graph(graph);
}

void test_bellman_ford() {
  printf("---\n");
  int size = 5;
  Graph *graph = create_graph(0, 1);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);

  /**
   *
   *             [1] * * -2- * * [3]
   *           *  * *           * *
   *        -4-   *   *       -4- *
   *      *       *   -3-   *     *
   *   [0]       -1-     \ /     -1-
   *      *       *      / \      *
   *        -2-   *    *     *    *
   *           *  *  *         *  *
   *             [2] * * -5- * * [4]
   *
   *
   */
  add_edge(graph, 0, 1, 4);
  add_edge(graph, 0, 2, 2);
  add_edge(graph, 1, 2, 1);
  add_edge(graph, 1, 3, 2);
  add_edge(graph, 1, 4, 3);
  add_edge(graph, 2, 3, 4);
  add_edge(graph, 2, 4, 5);
  add_edge(graph, 3, 4, 1);
  // bellman ford can be applied on non-negative graph
  Hashtable *dis = dijkstra(graph, 0);
  Hashtable *dis_2 = bellman_ford(graph, 0);
  HashtableEntrySet *entry_set = hashtable_entry_set(dis);
  printf("dijkstra: \n");
  for (int i = 0; i < entry_set->size; ++i) {
    printf("{%d : %d}\n",
           *(int *) table_entry_key(entry_set->entry_set[i]),
           *(int *) table_entry_value(entry_set->entry_set[i]));
    assert(contains_in_hash_table(dis_2, table_entry_key(entry_set->entry_set[i])));
    assert(*(int *) get_hash_table(dis_2, table_entry_key(entry_set->entry_set[i]))
               == *(int *) table_entry_value(entry_set->entry_set[i]));
  }

  HashtableEntrySet *entry_set2 = hashtable_entry_set(dis_2);
  printf("bellman ford: \n");
  for (int i = 0; i < entry_set2->size; ++i) {
    printf("{%d : %d}\n",
           *(int *) table_entry_key(entry_set2->entry_set[i]),
           *(int *) table_entry_value(entry_set2->entry_set[i]));
  }
  fflush(stdout);
  free_hashtable_entry_set(entry_set);
  free_hashtable_entry_set(entry_set2);
  free_hash_table(dis);
  free_hash_table(dis_2);
  /**
   * make the undirected graph has a negative edge so this graph contains a negative edge circle.
   * Bellman-Ford Algorithm won't be able to use.
   */
  remove_edge(graph, 0, 1);
  add_edge(graph, 0, 1, -4);
  Hashtable *dis_3 = bellman_ford(graph, 0);
  assert(dis_3 == NULL);

  free_graph(graph);


  // negative edges
  size = 8;
  graph = create_graph(1, 1);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);
  add_edge(graph, 4, 5, 35);
  add_edge(graph, 5, 4, 35);
  add_edge(graph, 4, 7, 37);
  add_edge(graph, 5, 7, 28);
  add_edge(graph, 7, 5, 28);
  add_edge(graph, 5, 1, 32);
  add_edge(graph, 0, 4, 38);
  add_edge(graph, 0, 2, 26);
  add_edge(graph, 7, 3, 39);
  add_edge(graph, 1, 3, 29);
  add_edge(graph, 2, 7, 34);
  add_edge(graph, 6, 2, -120);
  add_edge(graph, 3, 6, 52);
  add_edge(graph, 6, 0, -140);
  add_edge(graph, 6, 4, -125);

  // bellman ford can be applied on non-negative graph
  dis_2 = bellman_ford(graph, 0);
  entry_set2 = hashtable_entry_set(dis_2);
  printf("bellman ford: \n");
  for (int i = 0; i < entry_set2->size; ++i) {
    printf("{%d : %d}\n",
           *(int *) table_entry_key(entry_set2->entry_set[i]),
           *(int *) table_entry_value(entry_set2->entry_set[i]));
  }
  fflush(stdout);
  free_hashtable_entry_set(entry_set2);
  free_hash_table(dis_2);
  free_graph(graph);
}

void test_bellman_ford_path_to() {
  printf("---\n");
  int size = 5;
  Graph *graph = create_graph(0, 1);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);

  /**
   *
   *             [1] * * -2- * * [3]
   *           *  * *           * *
   *        -4-   *   *       -4- *
   *      *       *   -3-   *     *
   *   [0]       -1-     \ /     -1-
   *      *       *      / \      *
   *        -2-   *    *     *    *
   *           *  *  *         *  *
   *             [2] * * -5- * * [4]
   *
   *
   */
  add_edge(graph, 0, 1, 4);
  add_edge(graph, 0, 2, 2);
  add_edge(graph, 1, 2, 1);
  add_edge(graph, 1, 3, 2);
  add_edge(graph, 1, 4, 3);
  add_edge(graph, 2, 3, 4);
  add_edge(graph, 2, 4, 5);
  add_edge(graph, 3, 4, 1);
  // bellman ford can be applied on non-negative graph
  for (int i = 1; i < size; ++i) {
    LinkedList *p = bellman_ford_path_to(graph, 0, i);
    printf("bellman ford shortest path 0-%d: ", i);
    print_ll(head_of_list(p));
    printf("\n");
    free_linked_list(p, free);
  }

  /**
   * make the undirected graph has a negative edge so this graph contains a negative edge circle.
   * Bellman-Ford Algorithm won't be able to use.
   */
  remove_edge(graph, 0, 1);
  add_edge(graph, 0, 1, -4);
  for (int i = 1; i < size; ++i) {
    LinkedList *p = bellman_ford_path_to(graph, 0, i);
    assert(p == NULL);
  }
  free_graph(graph);

  // negative edges
  size = 8;
  graph = create_graph(1, 1);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);
  add_edge(graph, 4, 5, 35);
  add_edge(graph, 5, 4, 35);
  add_edge(graph, 4, 7, 37);
  add_edge(graph, 5, 7, 28);
  add_edge(graph, 7, 5, 28);
  add_edge(graph, 5, 1, 32);
  add_edge(graph, 0, 4, 38);
  add_edge(graph, 0, 2, 26);
  add_edge(graph, 7, 3, 39);
  add_edge(graph, 1, 3, 29);
  add_edge(graph, 2, 7, 34);
  add_edge(graph, 6, 2, -120);
  add_edge(graph, 3, 6, 52);
  add_edge(graph, 6, 0, -140);
  add_edge(graph, 6, 4, -125);

  // bellman ford can be applied on non-negative graph
  for (int i = 1; i < size; ++i) {
    LinkedList *p = bellman_ford_path_to(graph, 0, i);
    printf("bellman ford shortest path 0-%d: ", i);
    print_ll(head_of_list(p));
    printf("\n");
    free_linked_list(p, free);
  }
  free_graph(graph);

}

void test_floyd() {
  printf("---\n");
  int size = 5;
  Graph *graph = create_graph(0, 1);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);

  /**
   *
   *             [1] * * -2- * * [3]
   *           *  * *           * *
   *        -4-   *   *       -4- *
   *      *       *   -3-   *     *
   *   [0]       -1-     \ /     -1-
   *      *       *      / \      *
   *        -2-   *    *     *    *
   *           *  *  *         *  *
   *             [2] * * -5- * * [4]
   *
   *
   */
  add_edge(graph, 0, 1, 4);
  add_edge(graph, 0, 2, 2);
  add_edge(graph, 1, 2, 1);
  add_edge(graph, 1, 3, 2);
  add_edge(graph, 1, 4, 3);
  add_edge(graph, 2, 3, 4);
  add_edge(graph, 2, 4, 5);
  add_edge(graph, 3, 4, 1);

  printf("--floyd:\n");
  int has_negative_circle;
  Hashtable *dis = floyd(graph, &has_negative_circle);
  assert(has_negative_circle == 0);
  HashtableIterator *iter = hashtable_iterator(dis);
  while (hashtable_iter_has_next(iter)) {
    KVEntry *entry = hashtable_next_entry(iter);
    int *id = table_entry_key(entry);
    Hashtable *distance_to = table_entry_value(entry);
    HashtableIterator *iter2 = hashtable_iterator(distance_to);
    while (hashtable_iter_has_next(iter2)) {
      KVEntry *distance = hashtable_next_entry(iter2);
      int *to = table_entry_key(distance);
      int *d = table_entry_value(distance);
      printf("%d to %d: %d\n", *id, *to, *d);
    }
    free_hashtable_iter(iter2);
  }
  fflush(stdout);
  free_hashtable_iter(iter);
  free_hash_table(dis);
  /**
   * make the undirected graph has a negative edge so this graph contains a negative edge circle.
   * Bellman-Ford Algorithm won't be able to use.
   */
  remove_edge(graph, 0, 1);
  add_edge(graph, 0, 1, -4);
  Hashtable *dis2 = floyd(graph, &has_negative_circle);
  assert(has_negative_circle == 1);
  free_hash_table(dis2);
  free_graph(graph);

  // negative edges
  size = 8;
  graph = create_graph(1, 1);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);
  add_edge(graph, 4, 5, 35);
  add_edge(graph, 5, 4, 35);
  add_edge(graph, 4, 7, 37);
  add_edge(graph, 5, 7, 28);
  add_edge(graph, 7, 5, 28);
  add_edge(graph, 5, 1, 32);
  add_edge(graph, 0, 4, 38);
  add_edge(graph, 0, 2, 26);
  add_edge(graph, 7, 3, 39);
  add_edge(graph, 1, 3, 29);
  add_edge(graph, 2, 7, 34);
  add_edge(graph, 6, 2, -120);
  add_edge(graph, 3, 6, 52);
  add_edge(graph, 6, 0, -140);
  add_edge(graph, 6, 4, -125);

  Hashtable *dis3 = floyd(graph, &has_negative_circle);
  assert(has_negative_circle == 0);
  printf("---\n");
  HashtableIterator *iter3 = hashtable_iterator(dis3);
  while (hashtable_iter_has_next(iter3)) {
    KVEntry *entry = hashtable_next_entry(iter3);
    int *id = table_entry_key(entry);
    Hashtable *distance_to = table_entry_value(entry);
    HashtableIterator *iter2 = hashtable_iterator(distance_to);
    while (hashtable_iter_has_next(iter2)) {
      KVEntry *distance = hashtable_next_entry(iter2);
      int *to = table_entry_key(distance);
      int *d = table_entry_value(distance);
      printf("%d to %d: %d\n", *id, *to, *d);
    }
    free_hashtable_iter(iter2);
  }
  fflush(stdout);
  free_hashtable_iter(iter3);
  free_hash_table(dis3);

  free_graph(graph);

}

void test_floyd_path() {
  printf("---\n");
  int size = 5;
  Graph *graph = create_graph(0, 1);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);

  /**
   *
   *             [1] * * -2- * * [3]
   *           *  * *           * *
   *        -4-   *   *       -4- *
   *      *       *   -3-   *     *
   *   [0]       -1-     \ /     -1-
   *      *       *      / \      *
   *        -2-   *    *     *    *
   *           *  *  *         *  *
   *             [2] * * -5- * * [4]
   *
   *
   */
  add_edge(graph, 0, 1, 4);
  add_edge(graph, 0, 2, 2);
  add_edge(graph, 1, 2, 1);
  add_edge(graph, 1, 3, 2);
  add_edge(graph, 1, 4, 3);
  add_edge(graph, 2, 3, 4);
  add_edge(graph, 2, 4, 5);
  add_edge(graph, 3, 4, 1);

  printf("--floyd path:\n");
  int has_negative_circle;
  Hashtable *next_matrix = NULL;
  Hashtable *dis = floyd_path(graph, &has_negative_circle, &next_matrix);
  assert(has_negative_circle == 0);
  free_hash_table(dis);
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      LinkedList *path = floyd_path_to(next_matrix, i, j);
      if (path) {
        printf("%d to %d: ", i, j);
        print_ll(head_of_list(path));
        free_linked_list(path, free);
      }
    }
  }
  free_hash_table(next_matrix);
  free_graph(graph);

  // negative edges
  size = 8;
  graph = create_graph(1, 1);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);
  add_edge(graph, 4, 5, 35);
  add_edge(graph, 5, 4, 35);
  add_edge(graph, 4, 7, 37);
  add_edge(graph, 5, 7, 28);
  add_edge(graph, 7, 5, 28);
  add_edge(graph, 5, 1, 32);
  add_edge(graph, 0, 4, 38);
  add_edge(graph, 0, 2, 26);
  add_edge(graph, 7, 3, 39);
  add_edge(graph, 1, 3, 29);
  add_edge(graph, 2, 7, 34);
  add_edge(graph, 6, 2, -120);
  add_edge(graph, 3, 6, 52);
  add_edge(graph, 6, 0, -140);
  add_edge(graph, 6, 4, -125);

  printf("-- negative floyd path:\n");
  dis = floyd_path(graph, &has_negative_circle, &next_matrix);
  assert(has_negative_circle == 0);
  free_hash_table(dis);
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      LinkedList *path = floyd_path_to(next_matrix, i, j);
      if (path) {
        printf("%d to %d: ", i, j);
        print_ll(head_of_list(path));
        free_linked_list(path, free);
      }
    }
  }
  free_hash_table(next_matrix);
  free_graph(graph);

}

void test_toposort() {
  printf("topo sort\n");
// directed graph
  int size = 5;
  Graph *graph = create_graph(1, 0);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);
  /**
   * ↗↘↖↙
   *    0----->1----->2
   *           |    ↗ |
   *           |  /   |
   *           ↓/     ↓
   *           3      4
   */

  add_edge(graph, 0, 1, 0);
  add_edge(graph, 1, 2, 0);
  add_edge(graph, 1, 3, 0);
  add_edge(graph, 3, 2, 0);
  add_edge(graph, 2, 4, 0);

  LinkedList *ret = topological_sort(graph);
  assert(ret);
  print_ll(head_of_list(ret));
  free_linked_list(ret, free);
  free_graph(graph);
}

void test_kosaraju() {
  printf("scc_kosaraju\n");
// directed graph
  int size = 5;
  Graph *graph = create_graph(1, 0);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  assert(vertex_count(graph) == size);
  /**
   * ↗↘↖↙
   *    0----->1----->2
   *           ^    / |
   *           |  /   |
   *           |↙     ↓
   *           3      4
   */

  add_edge(graph, 0, 1, 0);
  add_edge(graph, 1, 2, 0);
  add_edge(graph, 2, 3, 0);
  add_edge(graph, 3, 1, 0);
  add_edge(graph, 2, 4, 0);
  Hashtable *strongly_components = scc_kosaraju(graph);
  HashtableIterator *iterator = hashtable_iterator(strongly_components);
  while (hashtable_iter_has_next(iterator)) {
    KVEntry *kv = hashtable_next_entry(iterator);
    int *cid = table_entry_key(kv);
    LinkedList *ids = table_entry_value(kv);
    int length = list_size(ids) * 2;
    char s[length];
    for (int i = 0; i < length; ++i) {
      s[i] = '\0';
    }

    LinkedListNode *list_node = head_of_list(ids);
    LinkedListNode *head = list_node;
    do {
      int *data = data_of_node_linked_list(list_node);
      char sid[4];
      sprintf(sid, "%d,", *data);
      strcat(s, sid);
      list_node = next_node_linked_list(list_node);
    } while (list_node != head);

    printf("cid: %d, ids: {%s}\n", *cid, s);
    free_linked_list(ids, free);
  }
  fflush(stdout);
  free_hashtable_iter(iterator);
  free_hash_table(strongly_components);
  free_graph(graph);

}
static void print_flow(Graph *graph, Hashtable *flow_map) {
  HashtableIterator *iter = hashtable_iterator(flow_map);
  while (hashtable_iter_has_next(iter)) {
    KVEntry *entry = hashtable_next_entry(iter);
    Hashset *adj = table_entry_value(entry);
    HashsetIterator *iterator = hashset_iterator(adj);
    while (hashset_iter_has_next(iterator)) {
      Edge *edge = set_entry_key(hashset_next_entry(iterator));
      int from = get_edge_from(edge);
      int to = get_edge_to(edge);
      int weight = get_edge_weight(edge);
      int cap = get_edge_weight(get_edge(graph, from, to));
      printf("<%d - %d>: %d/%d\n", from, to, weight, cap);
    }
    free_hashset_iter(iterator);
  }
  free_hashtable_iter(iter);
  fflush(stdout);
}
void test_max_flow() {
  int size = 4;
  Graph *graph = create_graph(1, 1);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  /**
   *
   */
  add_edge(graph, 0, 1, 3);
  add_edge(graph, 0, 2, 2);
  add_edge(graph, 1, 2, 5);
  add_edge(graph, 1, 3, 2);
  add_edge(graph, 2, 3, 3);
  int maxflow = 0;
  Hashtable *flow_map = max_flow(graph, 0, 3, &maxflow);
  printf("max flow: %d\n", maxflow);
  print_flow(graph, flow_map);

  register_hashtable_free_functions(flow_map, free, (HashtableValueFreeFunc) free_hash_set);
  free_hash_table(flow_map);

  free_graph(graph);

  size = 9;
  graph = create_graph(1, 1);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  add_edge(graph, 0, 1, 9);
  add_edge(graph, 0, 3, 9);
  add_edge(graph, 1, 2, 8);
  add_edge(graph, 1, 3, 10);
  add_edge(graph, 2, 5, 10);
  add_edge(graph, 3, 2, 1);
  add_edge(graph, 3, 4, 3);
  add_edge(graph, 4, 2, 8);
  add_edge(graph, 4, 5, 7);

  maxflow = 0;
  flow_map = max_flow(graph, 0, 5, &maxflow);
  printf("max flow: %d\n", maxflow);
  print_flow(graph, flow_map);

  register_hashtable_free_functions(flow_map, free, (HashtableValueFreeFunc) free_hash_set);
  free_hash_table(flow_map);

  free_graph(graph);
}

void test_bipartite_matching() {
  int size = 8;
  Graph *graph = create_graph(0, 0);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  add_edge(graph, 0, 4, 0);
  add_edge(graph, 0, 6, 0);
  add_edge(graph, 1, 4, 0);
  add_edge(graph, 2, 6, 0);
  add_edge(graph, 3, 5, 0);
  add_edge(graph, 3, 7, 0);

  int matching = bipartite_matching(graph);
  printf("bipartite matching: %d\n", matching);
  fflush(stdout);
  add_edge(graph, 1, 7, 0);
  matching = bipartite_matching(graph);
  printf("bipartite matching: %d\n", matching);
  free_graph(graph);
  fflush(stdout);
}

void test_hungarian(){
  int size = 8;
  Graph *graph = create_graph(0, 0);
  for (int i = 0; i < size; ++i) {
    int id = add_graph_data(graph, NULL);
    assert(id == i);
  }
  add_edge(graph, 0, 4, 0);
  add_edge(graph, 0, 6, 0);
  add_edge(graph, 1, 4, 0);
  add_edge(graph, 2, 6, 0);
  add_edge(graph, 3, 5, 0);
  add_edge(graph, 3, 7, 0);

  int matching = hungarian_matching(graph);
  printf("hungarian matching: %d\n", matching);
  fflush(stdout);
  add_edge(graph, 1, 7, 0);
  matching = hungarian_matching(graph);
  printf("hungarian matching: %d\n", matching);
  free_graph(graph);
  fflush(stdout);
}

static UnitTestFunction tests[] = {
    test_create_graph_undirected_unweighted,
    test_create_graph_undirected_weighted,
    test_create_graph_directed_unweighted,
    test_create_graph_directed_weighted,
    test_remove_vertex_directed_graph,
    test_remove_vertex_undirected_graph,
    test_remove_edge_undirected_graph,
    test_remove_edge_directed_graph,
    test_dfs_undirected,
    test_dfs_directed,
    test_dfs_nr_undirected,
    test_dfs_nr_directed,
    test_cc_count,
    test_graph_component,
    test_single_source_path_dfs,
    test_single_source_path_bfs,
    test_has_path,
    test_one_path,
    test_has_circle,
    test_enumerate_circle_path,
    test_bipartite,
    test_bfs,
    test_find_bridge,
    test_find_cut_points,
    test_hamilton_loop_path,
    test_hamilton_path,
    test_has_euler_loop,
    test_hierholzer_euler_loop,
    test_kruskal,
    test_prim,
    test_dijkstra,
    test_dijkstra_path,
    test_dijkstra_path_to,
    test_bellman_ford,
    test_bellman_ford_path_to,
    test_floyd,
    test_floyd_path,
    test_toposort,
    test_kosaraju,
    test_max_flow,
    test_bipartite_matching,
    test_hungarian,
    NULL
};

int main(int argc, char *argv[]) {
  run_tests(tests);
  return 0;
}
