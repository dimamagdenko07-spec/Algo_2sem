#include <math.h>
#include "graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "lab3/comparators.h"
#include "lab3/list/generic.h"
#include "lab4/hash_table/generic.h"
#ifdef _WIN32
#include <windows.h>
#endif

int hashLongLong(const void *key) {
    long long k = *(const long long *)key;
    int h = (int)(k ^ (k >> 32));
    return (h < 0) ? -h : h;
}

int compareLongLong(const void *a, const void *b) {
    long long arg1 = *(const long long *)a;
    long long arg2 = *(const long long *)b;
    return (arg1 == arg2); // Вернет 1 если равны, иначе 0
}

Graph *readNodes(const char *filename){
    int capacity = 100000;
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("File error");
        return NULL;
    }
    Graph *graph = (Graph *)malloc(sizeof(Graph));
    if (graph == NULL){
        printf("Memory error");
        return NULL;
    }
    graph->nodes = (GraphNode*)malloc(sizeof(GraphNode) * capacity);
    graph->nodes_count = 0;
    graph->id_to_index = createHashTable(sizeof(long long), sizeof(int));
    char line[256];
    fgets(line, sizeof(line), file);
    
    while (fgets(line, sizeof(line), file)){
        if (graph->nodes_count >= capacity){
            break;
        } 
        GraphNode *curr_node = &graph->nodes[graph->nodes_count];
        if (sscanf(line, "%lld,%lf,%lf", &curr_node->id, &curr_node->lat, &curr_node->lon) == 3) {
            curr_node->graph_index = graph->nodes_count;
            setItemHashTable(graph->id_to_index, &curr_node->id, &curr_node->graph_index, hashLongLong, compareLongLong);
            graph->nodes_count++;
        }
    }
    graph->adjs = (GenericList**)malloc(sizeof(GenericList*)*graph->nodes_count);
    if (graph->adjs == NULL){
        printf("Memory error");
        return NULL;
    }
    for (int i = 0; i < graph->nodes_count;i++){
        graph->adjs[i] = createList(sizeof(Edge));
    }
    fclose(file);
    return graph;
}

void freeGraph(Graph *graph) {
    if (graph == NULL) return;

    if (graph->adjs != NULL) {
        for (int i = 0; i < graph->nodes_count; i++) {
            if (graph->adjs[i] != NULL) {
                freeList(graph->adjs[i]); 
            }
        }
        free(graph->adjs);
    }

    if (graph->id_to_index != NULL) {
        freeHashTable(graph->id_to_index); 
    }

    if (graph->nodes != NULL) {
        free(graph->nodes);
    }

    free(graph);
}

void readEdges(const char *filename, Graph *graph){
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("File error");
        return;
    }
    char line[1024];
    fgets(line, sizeof(line), file);
    while (fgets(line, sizeof(line), file)){
        long long from;
        long long to;
        double length;
        char oneway[10];
        char name[50];
        oneway[0] = '\0';
        name[0] = '\0';
        if (sscanf(line, "%lld,%lld,%lf,%[^,],%[^\n\r]", &from, &to, &length, oneway, name) >= 4) {
            int* from_index = (int*)getItemHashTable(graph->id_to_index, &from, hashLongLong, compareLongLong);
            int* to_index = (int*)getItemHashTable(graph->id_to_index, &to, hashLongLong, compareLongLong);
            if (from_index != NULL && to_index != NULL){
                int u_index = *from_index;
                int v_index = *to_index;
                Edge u_v;
                u_v.from = u_index;
                u_v.to = v_index;
                u_v.length = length;
                strncpy(u_v.name, name, sizeof(name) - 1);
                u_v.name[sizeof(u_v.name) - 1] = '\0';
                appendItem(graph->adjs[u_index], &u_v);
                if (oneway[0] != 'F' && oneway[0] != 'f'){
                    u_v.to = u_index;
                    u_v.from = v_index;
                    appendItem(graph->adjs[v_index], &u_v);
                }
            }
        }
    }
    fclose(file);
}


double evklidDist(double lat1, double lat2, double lon1, double lon2){
    return ((lat1-lat2)*(lat1-lat2) + (lon1-lon2)*(lon1-lon2));
}


int findClosestNode(double lat, double lon, GraphNode *nodes, int num_nodes){
    int index = 0;
    double min_ = DBL_MAX;
    for (int i = 0; i < num_nodes; i++){
        double curr_lat = nodes[i].lat;
        double curr_lon = nodes[i].lon;
        double evkl = evklidDist(lat, curr_lat, lon, curr_lon);
        if (evkl < min_){
            min_ = evkl;
            index = i;
        }
    }
    return index;
}


static HeapItem *createHeapItem(int node_index, double distance) {
    HeapItem *item = (HeapItem*)malloc(sizeof(HeapItem));
    item->node_index = node_index;
    item->distance = distance;
    return item;
}


MinHeap *createMinHeap(int capacity) {
    MinHeap *heap = (MinHeap*)malloc(sizeof(MinHeap));
    if (!heap) return NULL;
    heap->size = 0;
    heap->capacity = capacity;
    heap->nodes = (HeapItem**)malloc(sizeof(HeapItem*) * capacity);
    return heap;
}

static void swapNodes(HeapItem **a, HeapItem **b) {
    HeapItem *temp = *a;
    *a = *b;
    *b = temp;
}

// Убедись, что в heapifyUp и heapifyDown вызывается правильный swap
static void heapifyUp(MinHeap *heap, int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (heap->nodes[parent]->distance > heap->nodes[idx]->distance) {
            swapNodes(&heap->nodes[parent], &heap->nodes[idx]); // Здесь
            idx = parent;
        } else break;
    }
}

static void heapifyDown(MinHeap *heap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < heap->size && heap->nodes[left]->distance < heap->nodes[smallest]->distance)
        smallest = left;
    if (right < heap->size && heap->nodes[right]->distance < heap->nodes[smallest]->distance)
        smallest = right;

    if (smallest != idx) {
        swapNodes(&heap->nodes[idx], &heap->nodes[smallest]); // И здесь
        heapifyDown(heap, smallest);
    }
}

void insertMinHeap(MinHeap *heap, int node_index, double distance) {
    if (heap->size >= heap->capacity) return;
    
    heap->nodes[heap->size] = createHeapItem(node_index, distance);
    heapifyUp(heap, heap->size);
    heap->size++;
}

HeapItem *extractMin(MinHeap *heap) {
    if (heap->size == 0) return NULL;
    
    HeapItem *min = heap->nodes[0];
    heap->nodes[0] = heap->nodes[heap->size - 1];
    heap->size--;
    heapifyDown(heap, 0);
    return min;
}

void freeMinHeap(MinHeap *heap) {
    if (!heap) return;
    for(int i = 0; i < heap->size; i++) free(heap->nodes[i]);
    free(heap->nodes);
    free(heap);
}

DejkstraResult *dejkstra(Graph *graph, int start_id){
    DejkstraResult *res = (DejkstraResult *)malloc(sizeof(DejkstraResult));
    if (res == NULL){
        printf("Memory error");
        return NULL;
    }
    MinHeap *heap = createMinHeap(graph->nodes_count*10);
    res->dist = (double *)malloc(sizeof(double)*graph->nodes_count);
    res->parents = (int *)malloc(sizeof(int) * graph->nodes_count);
    for (int i = 0; i < graph->nodes_count; i++){
        res->dist[i] = DBL_MAX;
        res->parents[i] = -1;
    }
    res->dist[start_id] = 0;
    
    insertMinHeap(heap, start_id, 0);
    HeapItem *node = extractMin(heap);
    while (node != NULL){
        if (res->dist[node->node_index] >= node->distance){
            Node *curr_neigh = getItem(graph->adjs[node->node_index], 0);
            while (curr_neigh != NULL){
                Edge *data = (Edge *)curr_neigh->data;
                double length = data->length;
                int curr_id = data->to;
                if (res->dist[curr_id] > res->dist[node->node_index] + length){
                    res->dist[curr_id] = res->dist[node->node_index] + length;
                    insertMinHeap(heap, curr_id, res->dist[curr_id]);
                    res->parents[curr_id] = node->node_index;
                }
                curr_neigh = curr_neigh->next;
            }
        }
        free(node);
        node = extractMin(heap);
    }
    freeMinHeap(heap);
    return res;
}

void savePath(const char *filename, Graph *g, DejkstraResult *res, int end_idx) {
    FILE *f = fopen(filename, "wb");
    if (!f) return;

    if (res->dist[end_idx] >= DBL_MAX - 1.0) {
        fclose(f);
        return;
    }

    int *path = malloc(sizeof(int) * g->nodes_count);
    int count = 0;
    
    for (int v = end_idx; v != -1; v = res->parents[v]) {
        path[count++] = v;
    }

    for (int i = count - 1; i >= 0; i--) {
        int idx = path[i];
        fprintf(f, "%.6lf %.6lf\n", g->nodes[idx].lat, g->nodes[idx].lon);
    }

    free(path);
    fclose(f);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <data_dir> <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    #ifdef _WIN32
    SetConsoleOutputCP(65001);
    #endif

    char *data_dir = argv[1];
    char *input_path = argv[2];
    char *output_path = argv[3];

    char nodes_csv[512], edges_csv[512];
    sprintf(nodes_csv, "%s/nodes.csv", data_dir);
    sprintf(edges_csv, "%s/edges.csv", data_dir);

    printf("Loading graph nodes\n");
    Graph *g = readNodes(nodes_csv);
    if (!g) return 1;

    printf("Loading graph edges\n");
    readEdges(edges_csv, g);

    FILE *in = fopen(input_path, "r");
    if (!in) { perror("Input file error"); return 1; }
    
    double s_lat, s_lon, e_lat, e_lon;
    if (fscanf(in, "%lf %lf %lf %lf", &s_lat, &s_lon, &e_lat, &e_lon) != 4) {
        rewind(in);
        fscanf(in, "%lf %lf", &s_lat, &s_lon);
        fscanf(in, "%lf %lf", &e_lat, &e_lon);
    }
    fclose(in);

    int start_idx = findClosestNode(s_lat, s_lon, g->nodes, g->nodes_count);
    int end_idx = findClosestNode(e_lat, e_lon, g->nodes, g->nodes_count);

    printf("Running Dejkstra\n");
    DejkstraResult *res = dejkstra(g, start_idx);

    printf("Saving result to %s\n", output_path);
    savePath(output_path, g, res, end_idx);


    free(res->dist);
    free(res->parents);
    free(res);
    freeGraph(g);
    
    printf("Done!\n");
    return 0;
}