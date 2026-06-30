#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "index/index.h"
#include "index/search.h"

#define MAX_LINE 8192
#define MAX_TOKENS 512

static void runSearch(TreeType type, const char* idx_path,
                      const char* query, int json_out) {
    Index* idx = loadIndex(idx_path, type);
    if (!idx) {
        fprintf(stderr, "Failed to load index: %s\n", idx_path);
        exit(1);
    }

    SearchResults* sr = search(idx, query);
    if (!sr) {
        fprintf(stderr, "Search failed\n");
        freeIndex(idx);
        exit(1);
    }

    if (json_out) {
        printResultsJSON(sr);
    } else {
        printResultsText(sr);
    }

    freeSearchResults(sr);
    freeIndex(idx);
}

static void usage(const char* prog) {
    fprintf(stderr,
        "Usage:\n"
        "  %s index  --type=<avl|rb|btree> [--data=PATH] [--index=PATH]\n"
        "  %s search --type=<avl|rb|btree> [--index=PATH] [--json] \"query\"\n",
        prog, prog);
}

const char* typeName(TreeType type) {
    switch (type) {
        case TREE_AVL:   return "avl";
        case TREE_RB:    return "rb";
        case TREE_BTREE: return "btree";
        default:         return "unknown";
    }
}

TreeType parseType(const char* s) {
    if (strcmp(s, "avl") == 0) {
        return TREE_AVL;
    }
    if (strcmp(s, "rb") == 0) {
        return TREE_RB;
    }
    if (strcmp(s, "btree") == 0) {
        return TREE_BTREE;
    }

    fprintf(stderr, "Unknown tree type: %s\n", s);
    exit(1);
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

static Index* buildIndex(TreeType type, const char* docs_path) {
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
    size_t cnt = 0;

    int docs_count = 0;

    while (docs_count < 100000 && fgets(line, sizeof(line), file)) {
        int doc_id;
        char title[256];
        const char* tokens[MAX_TOKENS];
        int n_tokens = 0;

        if (!parseJsonLine(line, &doc_id, title, tokens, &n_tokens)) {
            continue;
        }

        indexDocument(idx, doc_id, title, tokens, n_tokens);

        cnt++;
        if (cnt % 10000 == 0) {
            printf("Indexed %zu documents\n", cnt);
            fflush(stdout);
        }

        docs_count++;
    }

    fclose(file);
    return idx;
}

static void runIndex(TreeType type, const char* data_path, const char* idx_path) {
    Index* idx = buildIndex(type, data_path);

    if (!idx) {
        fprintf(stderr, "Err in %s:\nFailed to build index\n", __func__);
        exit(1);
    }

    saveIndex(idx, idx_path);
    freeIndex(idx);
}

int main(int argc, char* argv[]) {
    if (argc < 3) { usage(argv[0]); return 1; }

    const char* mode = argv[1];
    TreeType    type = TREE_AVL;
    const char* data_path = "data/processed/docs.jsonl";
    char        idx_path[512] = {0};
    int         json_out = 0;
    const char* query    = NULL;

    for (int i = 2; i < argc; i++) {
        if      (strncmp(argv[i], "--type=",  7) == 0) type = parseType(argv[i] + 7);
        else if (strncmp(argv[i], "--data=",  7) == 0) data_path = argv[i] + 7;
        else if (strncmp(argv[i], "--index=", 8) == 0)
            strncpy(idx_path, argv[i] + 8, sizeof(idx_path) - 1);
        else if (strcmp(argv[i], "--json")    == 0)    json_out = 1;
        else if (argv[i][0] != '-')                    query = argv[i];
    }

    if (idx_path[0] == '\0')
        snprintf(idx_path, sizeof(idx_path), "data/index_%s.txt", typeName(type));

    if (strcmp(mode, "index") == 0) {
        runIndex(type, data_path, idx_path);
    } else if (strcmp(mode, "search") == 0) {
        if (!query) { fprintf(stderr, "No query provided\n"); return 1; }
        runSearch(type, idx_path, query, json_out);
    } else {
        fprintf(stderr, "Unknown mode: %s\n", mode);
        usage(argv[0]);
        return 1;
    }
    return 0;
}
