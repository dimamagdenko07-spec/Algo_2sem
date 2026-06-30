#ifndef GRAPH_H
#define GRAPH_H

#include <stddef.h>
#include "lab3/list/generic.h"
#include "lab4/hash_table/generic.h"

typedef struct {
    long long id;
    double lat;
    double lon;
    int graph_index;
} GraphNode;

typedef struct {
    long long from;
    long long to;
    double length;
    char name[50];
} Edge;

typedef struct {
    GraphNode *nodes;
    int nodes_count;
    HashTable *id_to_index;
    GenericList **adjs;
} Graph;

typedef struct {
    int node_index;   // Индекс узла в массиве graph->nodes
    double distance;  // Текущее расстояние до этого узла
} HeapItem;

typedef struct {
    int size;
    int capacity;
    HeapItem **nodes; // Массив указателей на элементы
} MinHeap;

typedef struct {
    double *dist;
    int *parents;
} DejkstraResult;

Graph *readNodes(const char* filename);

void freeGraph(Graph *graph);

void readEdges(const char* filename, Graph *graph);

double evklidDist(double lat1, double lat2, double lon1, double lon2);

int findClosestNode(double lat, double lon, GraphNode *nodes, int num_nodes);

MinHeap *createMinHeap(int capacity);

void insertMinHeap(MinHeap *heap, int node_index, double distance);

HeapItem *extractMin(MinHeap *heap);

void freeMinHeap(MinHeap *heap);

DejkstraResult *dejkstra(Graph *graph, int start_id);

#endif // GRAPH_H
