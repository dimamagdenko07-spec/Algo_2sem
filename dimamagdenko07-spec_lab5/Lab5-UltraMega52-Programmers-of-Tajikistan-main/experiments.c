#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <psapi.h>
#include "index/index.h"
#include "index/search.h"

#define MAX_LINE 8192
#define MAX_TOKENS 512

// Вспомогательная структура для хранения списка слов
typedef struct {
    char** words;
    size_t count;
    size_t capacity;
} WordCollector;

static double nowMs(void) {
    // Возвращает текущее время
    return (double)clock() * 1000.0 / CLOCKS_PER_SEC;
}

static double getMemoryMB(void) {
    // Замена линуксовскому методу измерения памяти
    PROCESS_MEMORY_COUNTERS pmc;

    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.PeakWorkingSetSize / 1024.0 / 1024.0;
    }

    return -1.0;
}

static const char* treeTypeName(TreeType type) {
    if (type == TREE_AVL) return "avl";
    if (type == TREE_RB) return "rb";
    if (type == TREE_BTREE) return "btree";
    return "unknown";
}

static int parseJsonLine(char* line, int* doc_id, char* title, const char** tokens, int* n_tokens) {
    // Парсит json строку на токены
    static char token_storage[MAX_TOKENS][256];

    char* id_pos = strstr(line, "\"doc_id\"");
    char* title_pos = strstr(line, "\"title\"");
    char* tokens_pos = strstr(line, "\"tokens\"");

    if (!id_pos || !title_pos || !tokens_pos) {
        return 0;
    }

    char* id_colon = strchr(id_pos, ':');
    if (!id_colon) return 0;

    id_colon++;

    while (*id_colon == ' ' || *id_colon == '"') {
        id_colon++;
    }

    *doc_id = atoi(id_colon);

    char* title_colon = strchr(title_pos, ':');
    if (!title_colon) return 0;

    char* title_begin = strchr(title_colon, '"');
    if (!title_begin) return 0;

    title_begin++;

    char* title_end = strchr(title_begin, '"');
    if (!title_end) return 0;

    size_t title_len = title_end - title_begin;
    if (title_len > 255) title_len = 255;

    memcpy(title, title_begin, title_len);
    title[title_len] = '\0';

    char* arr_begin = strchr(tokens_pos, '[');
    char* arr_end = strchr(tokens_pos, ']');

    if (!arr_begin || !arr_end) {
        return 0;
    }

    *n_tokens = 0;

    char* p = arr_begin;

    while ((p = strchr(p, '"')) && p < arr_end) {
        p++;

        char* token_end = strchr(p, '"');
        if (!token_end || token_end > arr_end) {
            break;
        }

        size_t len = token_end - p;
        if (len > 255) len = 255;

        memcpy(token_storage[*n_tokens], p, len);
        token_storage[*n_tokens][len] = '\0';

        tokens[*n_tokens] = token_storage[*n_tokens];
        (*n_tokens)++;

        if (*n_tokens >= MAX_TOKENS) {
            break;
        }

        p = token_end + 1;
    }

    return *n_tokens > 0;
}

static Index* buildIndex(TreeType type, const char* docs_path, int docs_limit) {
    // Идексирует файл построчно
    FILE* file = fopen(docs_path, "r");

    if (!file) {
        printf("Err in %s:\nFailed to open file %s\n", __func__, docs_path);
        return NULL;
    }

    Index* idx = createIndex(type);

    if (!idx) {
        printf("Err in %s:\nSomething went wrong\n", __func__);
        fclose(file);
        return NULL;
    }

    char line[MAX_LINE];
    int docs_count = 0;

    while (docs_count < docs_limit && fgets(line, sizeof(line), file)) {
        int doc_id;
        char title[256];
        const char* tokens[MAX_TOKENS];
        int n_tokens = 0;

        if (!parseJsonLine(line, &doc_id, title, tokens, &n_tokens)) {
            continue;
        }

        indexDocument(idx, doc_id, title, tokens, n_tokens);
        docs_count++;
    }

    fclose(file);
    return idx;
}

double benchmarkSearch(Index* idx, char** queries, int query_count) {
    // Замеряет время, которое нужно на поиск query_count запросов
    if (!idx || !queries || query_count <= 0) {
        return -1.0;
    }

    double start = nowMs();

    for (int i = 0; i < query_count; i++) {
        if (!queries[i]) continue;

        SearchResults* sr = search(idx, queries[i]);
        freeSearchResults(sr);
    }

    return (nowMs() - start) / query_count;
}

static void collectWords(const char* key, Vector* postings, void* ctx) {
    // Собирает все ключи из идекса в массив для дальнейшей генерации списков из них
    (void)postings;

    WordCollector* collector = (WordCollector*)ctx;

    if (!collector || !key) {
        return;
    }

    if (collector->count == collector->capacity) {
        collector->capacity *= 2;

        char** tmp = realloc(
            collector->words,
            collector->capacity * sizeof(char*)
        );

        if (!tmp) {
            return;
        }

        collector->words = tmp;
    }

    collector->words[collector->count] = strdup(key);

    if (collector->words[collector->count]) {
        collector->count++;
    }
}

static char** generateQueries(Index* idx, int query_count, int words_per_query) {
    // Генерирует список на query_count запросов из words_per_query слов
    if (!idx || query_count <= 0 || words_per_query <= 0) {
        return NULL;
    }

    WordCollector collector;

    collector.count = 0;
    collector.capacity = 1024;
    collector.words = malloc(collector.capacity * sizeof(char*));

    if (!collector.words) {
        return NULL;
    }

    traverseIndex(idx, collectWords, &collector);

    if (collector.count == 0) {
        free(collector.words);
        return NULL;
    }

    char** queries = malloc(query_count * sizeof(char*));

    if (!queries) {
        for (size_t i = 0; i < collector.count; i++) {
            free(collector.words[i]);
        }

        free(collector.words);
        return NULL;
    }

    for (int q = 0; q < query_count; q++) {
        size_t query_len = 1;

        size_t* positions = malloc(words_per_query * sizeof(size_t));

        if (!positions) {
            queries[q] = NULL;
            continue;
        }

        for (int w = 0; w < words_per_query; w++) {
            positions[w] = (size_t)(rand() % collector.count);
            query_len += strlen(collector.words[positions[w]]) + 1;
        }

        queries[q] = malloc(query_len);

        if (!queries[q]) {
            free(positions);
            continue;
        }

        queries[q][0] = '\0';

        for (int w = 0; w < words_per_query; w++) {
            strcat(queries[q], collector.words[positions[w]]);

            if (w + 1 < words_per_query) {
                strcat(queries[q], " ");
            }
        }

        free(positions);
    }

    for (size_t i = 0; i < collector.count; i++) {
        free(collector.words[i]);
    }

    free(collector.words);

    return queries;
}

void freeQueries(char** queries, int query_count) {
    if (!queries) return;

    for (int i = 0; i < query_count; i++) {
        free(queries[i]);
    }

    free(queries);
}

static void runExperiment(FILE* csv, TreeType type,
                          const char* docs_path, int docs_limit) {
    /*
    Основная функция
    Создаёт txt файлы для сохранения индексов и
    csv файл для сохранения результатов экспериментов
    */
    char index_path[256];

    sprintf(index_path, "data/index_%s_%d.txt",
            treeTypeName(type), docs_limit);

    double start_build = nowMs();
    Index* idx = buildIndex(type, docs_path, docs_limit);
    double build_ms = nowMs() - start_build;

    if (!idx) {
        fprintf(csv, "%s;%d;BUILD_ERROR;None;None;None;None;None;None;None;None\n",
                treeTypeName(type), docs_limit);
        fflush(csv);
        return;
    }

    double memory_after_build = getMemoryMB();

    double start_save = nowMs();
    saveIndex(idx, index_path);
    double save_ms = nowMs() - start_save;

    double start_load = nowMs();
    Index* loaded = loadIndex(index_path, type);
    double load_ms = nowMs() - start_load;

    if (!loaded) {
        fprintf(csv, "%s;%d;LOAD_ERROR;%.3f;%.3f;None;None;None;None;%.2f;None\n",
                treeTypeName(type),
                docs_limit,
                build_ms / 1000.0,
                save_ms / 1000.0,
                memory_after_build);
        fflush(csv);
        return;
    }

    double memory_after_load = getMemoryMB();

    char** q1 = generateQueries(idx, 1000, 1);
    char** q2 = generateQueries(idx, 1000, 2);
    char** q3 = generateQueries(idx, 1000, 3);

    freeIndex(idx); // Он нам больше не нужен

    double benchmarkSearch1Word = benchmarkSearch(loaded, q1, 1000);
    double benchmarkSearch2Words = benchmarkSearch(loaded, q2, 1000);
    double benchmarkSearch3Words = benchmarkSearch(loaded, q3, 1000);

    fprintf(csv, "%s;%d;OK;%.3f;%.3f;%.3f;%.6f;%.6f;%.6f;%.2f;%.2f\n",
            treeTypeName(type),
            docs_limit,
            build_ms / 1000.0,
            save_ms / 1000.0,
            load_ms / 1000.0,
            benchmarkSearch1Word,
            benchmarkSearch2Words,
            benchmarkSearch3Words,
            memory_after_build,
            memory_after_load);

    fflush(csv);
    freeIndex(loaded);
    freeQueries(q1, 1000);
    freeQueries(q2, 1000);
    freeQueries(q3, 1000);
}

int main(void) {
    const char* docs_path = "data/processed/docs.jsonl";
    const char* csv_path = "data/experiments.csv";

    FILE* csv = fopen(csv_path, "w");

    if (!csv) {
        return 1;
    }

    fprintf(csv,
            "tree;documents;status;index_sec;save_sec;load_sec;"
            "avg_search_1_word;avg_search_2_words;avg_search_3_words;memory_after_build_mb;"
            "memory_after_load_mb\n");

    fflush(csv);

    int limits[] = {50000, 200000, 500000};
    int limits_count = sizeof(limits) / sizeof(limits[0]);

    for (int i = 0; i < limits_count; i++) {
        runExperiment(csv, TREE_AVL, docs_path, limits[i]);
        runExperiment(csv, TREE_RB, docs_path, limits[i]);
        runExperiment(csv, TREE_BTREE, docs_path, limits[i]);
    }

    fclose(csv);
    return 0;
}