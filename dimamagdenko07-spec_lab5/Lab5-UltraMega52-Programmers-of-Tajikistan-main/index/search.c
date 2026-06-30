// Тут все на ваше усмотрение, просто переиспользуйте код из предыдущих лабораторных, если он вам подходит. Главное, чтобы интерфейс был таким же, как в avl и btree, чтобы не менять код в main.c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "search.h"

Vector* intersectPostings(Vector** lists, int n) {
    if (!lists || n <= 0) {
        printf("Err 1 in %s:\nSome data is empty\n", __func__);
        return NULL;
    }

    for (int i = 0; i < n; i++) {
        if (!lists[i] || lists[i]->size == 0) {
            printf("Err 2 in %s:\nSome data is empty\n", __func__);
            return NULL;
        }
    }

    Vector *result = createVector(sizeof(SearchResult));

    if (!result) {
        printf("Err in %s:\nMemory was not allocated\n", __func__);
        return NULL;
    }

    for (size_t i = 0; i < lists[0]->size; i++) {
        SearchResult* elem0 = (SearchResult*)getVectorItem(lists[0], i);
        if (!elem0) {
            continue;
        }

        int doc_id_to_find =elem0->doc_id;
        int found_in_all = 1;

        SearchResult* best_match = elem0;

        for (int j = 1; j < n; j++) {
            int found_in_curr = 0;
            for (size_t k = 0; k < lists[j]->size; k++) {
                SearchResult* elem_j = (SearchResult*)getVectorItem(lists[j], k);
                if (elem_j && elem_j->doc_id == doc_id_to_find) {
                    found_in_curr = 1;

                    if (strcmp(best_match->title, "default_title") == 0 && 
                    strcmp(elem_j->title, "default_title") != 0) {
                        best_match = elem_j;
                    }
                    break;
                }
            }
            if (!found_in_curr) {
                found_in_all = 0;
                break;
            }
        }

        if (found_in_all) {
            SearchResult res_item;
            res_item.doc_id = doc_id_to_find;
            strncpy(res_item.title, best_match->title, MAX_TITLE_LEN - 1);
            res_item.title[MAX_TITLE_LEN - 1] = '\0';
            res_item.score = n;
            appendVectorItem(result, &res_item);
        }
    }

    if (result->size == 0) {
        printf("Err in %s:\nData was not found\n", __func__);
        vectorFree(result);
        return NULL;
    }

    return result;
}

SearchResults* search(Index* idx, const char* query) {

    if (!idx || !query) {
        printf("Err in %s:\nSome data is empty\n", __func__);
        return NULL;
    }

    clock_t start_time = clock();

    SearchResults* sr = (SearchResults*)malloc(sizeof(SearchResults));

    if (!sr) {
        printf("Err in %s:\nMemory was not allocated\n", __func__);
        return NULL;
    }

    sr->results = NULL;
    sr->total = 0;
    sr->time_ms = 0.0;

    char* query_copy = strdup(query);
    if (!query_copy) {
        printf("Err in %s:\nSomething went wrong\n", __func__);
        free(sr);
        return NULL;
    }

    Vector *collected_lists[128];
    int list_count = 0;
    
    char* token = strtok(query_copy, " \t\n\r,.");

    while(token && list_count < 128) {
        Vector* term_postings = lookupTerm(idx, token);

        if (!term_postings || term_postings->size == 0) {
            free(query_copy);
            sr->results = createVector(sizeof(SearchResult));
            clock_t end_time = clock();

            sr->time_ms = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;
            return sr;
        }

        collected_lists[list_count++] = term_postings;
        token = strtok(NULL, " \t\n\r,.");
    }

    free(query_copy);

    if (list_count == 0) {
        sr->results = createVector(sizeof(SearchResult));
        return sr;
    }

    sr->results = intersectPostings(collected_lists, list_count);
    if (sr->results) {
        sr->total = (int)sr->results->size;
    } else {
        sr->results = createVector(sizeof(SearchResult));
        sr->total = 0;
    }

    clock_t end_time = clock();
    sr->time_ms = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;

    return sr;
}

void printResultsText(const SearchResults* sr) {
    if (!sr) {
        printf("No results to print\n");
        return;
    } 

    printf("Search completed in %.2f ms.\n%d documents were found:\n", sr->time_ms, sr->total);

    if (sr->total == 0 || !sr->results) {
        printf("No match with your query\n");
        return;
    }

    for (int i = 0; i < sr->total; i++) {
        SearchResult* res = (SearchResult*)getVectorItem(sr->results, i);
        if (res) {
            printf("%d. ID: %d \"%s\" Score: %d\n", i + 1, res->doc_id, res->title, res->score);
        }
    }
}

void printResultsJSON(const SearchResults* sr) { 
    if (!sr) {
        printf("{}\n");
        return;
    }

    printf("{\n");
    printf("  \"time_ms\": %.2f,\n", sr->time_ms);
    printf("  \"total\": %d,\n", sr->total);
    printf("  \"results\": [\n");

    if (sr->results && sr->total > 0) {
        for (int i = 0; i < sr->total; i++) {
            SearchResult* res = (SearchResult*)getVectorItem(sr->results, i);
            if (res) {
                printf("    {\n");
                printf("      \"doc_id\": %d,\n", res->doc_id);
                printf("      \"title\": \"%s\",\n", res->title);
                printf("      \"score\": %d\n", res->score);
                if (i == sr->total - 1) {
                    printf("    }\n");
                } else {
                    printf("    },\n");
                }
            }
        }
    }
    printf("  ]\n");
    printf("}\n");
}

void freeSearchResults(SearchResults* sr) {
    if (!sr) {
        return;
    }

    if (sr->results) {
        vectorFree(sr->results);
    }

    free(sr);
}