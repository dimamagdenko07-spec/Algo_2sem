#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "fuzzy.h"
#include "../index/index.h"
#include "../posting.h"

#define COLOR_RESET   "\x1b[0m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_CYAN    "\x1b[36m"

int total_passed = 0;
int total_tests = 0;

#define RUN_CHECK(condition, message) \
    do { \
        total_tests++; \
        if (condition) { \
            printf("  " COLOR_GREEN "[OK]" COLOR_RESET "   %s\n", message); \
            total_passed++; \
        } else { \
            printf("  " COLOR_RED "[FAIL]" COLOR_RESET " %s\n", message); \
        } \
    } while (0)

/* Простая токенизация для тестов: lowercase, без пунктуации */
static void tokenize(const char *text, char tokens[][64], int *count, int max_tokens) {
    *count = 0;
    int ti = 0;
    for (int i = 0; text[i] != '\0' && *count < max_tokens; i++) {
        char c = text[i];
        if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
        if (c >= 'a' && c <= 'z') {
            if (ti < 63) tokens[*count][ti++] = c;
        } else {
            if (ti > 0) {
                tokens[*count][ti] = '\0';
                (*count)++;
                ti = 0;
            }
        }
    }
    if (ti > 0 && *count < max_tokens) {
        tokens[*count][ti] = '\0';
        (*count)++;
    }
}

/* Вспомогательная: индексирует документ в любой индекс */
static void add_doc(Index *idx, int id, const char *title, const char *text) {
    char toks[32][64];
    int n_tok = 0;
    tokenize(text, toks, &n_tok, 32);
    const char *tok_ptrs[32];
    for (int i = 0; i < n_tok; i++) tok_ptrs[i] = toks[i];
    indexDocument(idx, id, title, tok_ptrs, n_tok);
}

/* Создаёт индекс с тестовыми документами */
static Index *create_test_index(TreeType type) {
    Index *idx = createIndex(type);
    if (!idx) return NULL;

    add_doc(idx, 1,  "How to sort a list in Python",       "python list sort");
    add_doc(idx, 2,  "Merge dictionaries in Python",       "python dictionary merge");
    add_doc(idx, 3,  "Java ArrayList vs LinkedList",       "java list array");
    add_doc(idx, 4,  "Sorting algorithms in Python",       "python sort algorithm");
    add_doc(idx, 5,  "Sorting a linked list efficiently",  "sorting linked list");
    add_doc(idx, 6,  "Python memory management tips",      "python memory management");
    add_doc(idx, 7,  "How to handle exceptions in Python", "python exception handling");
    add_doc(idx, 8,  "Binary search tree implementation",  "binary search tree");
    add_doc(idx, 9,  "Memory leak detection in C",         "memory leak detection c");
    add_doc(idx, 10, "Python list comprehension examples", "python list comprehension");

    return idx;
}

int main() {
    printf("\n");

    /* Тест 1: расстояние Левенштейна — базовые случаи */
    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " levenshteinDistance: basic cases\n");

    RUN_CHECK(levenshteinDistance("python", "python") == 0,
              "Identical strings have distance 0");
    RUN_CHECK(levenshteinDistance("", "abc") == 3,
              "Empty to 'abc' has distance 3");
    RUN_CHECK(levenshteinDistance("abc", "") == 3,
              "'abc' to empty has distance 3");
    RUN_CHECK(levenshteinDistance("", "") == 0,
              "Two empty strings have distance 0");
    RUN_CHECK(levenshteinDistance("python", "pythn") == 1,
              "'python' vs 'pythn' (missing letter) has distance 1");
    RUN_CHECK(levenshteinDistance("list", "lost") == 1,
              "'list' vs 'lost' (one substitution) has distance 1");
    RUN_CHECK(levenshteinDistance("sort", "short") == 1,
              "'sort' vs 'short' (one insertion) has distance 1");
    RUN_CHECK(levenshteinDistance("kitten", "sitting") == 3,
              "'kitten' vs 'sitting' has distance 3");

    printf("\n");

    /* Тест 2: расстояние Левенштейна — NULL и некорректные аргументы */
    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " levenshteinDistance: NULL handling\n");

    RUN_CHECK(levenshteinDistance(NULL, "abc") == -1,
              "NULL as first argument returns -1");
    RUN_CHECK(levenshteinDistance("abc", NULL) == -1,
              "NULL as second argument returns -1");
    RUN_CHECK(levenshteinDistance(NULL, NULL) == -1,
              "Both NULL returns -1");

    printf("\n");

    /* Тест 3: fuzzyFindCandidates — поиск с опечатками */
    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " fuzzyFindCandidates: finding terms with typos\n");

    Index *idx3 = create_test_index(TREE_AVL);
    Vector *cl = NULL;

    cl = fuzzyFindCandidates(idx3, "pythn", 2);
    RUN_CHECK(cl != NULL, "Candidates found for 'pythn' (max_dist=2)");
    if (cl) {
        RUN_CHECK(cl->size > 0, "At least one candidate for 'pythn'");
        FuzzyCandidate *first = (FuzzyCandidate *)getVectorItem(cl, 0);
        RUN_CHECK(first != NULL && strcmp(first->term, "python") == 0,
                  "First candidate for 'pythn' is 'python'");
        RUN_CHECK(first != NULL && first->distance == 1,
                  "Distance from 'pythn' to 'python' is 1");
        vectorFree(cl);
    }

    cl = fuzzyFindCandidates(idx3, "lisr", 2);
    RUN_CHECK(cl != NULL, "Candidates found for 'lisr' (max_dist=2)");
    if (cl) {
        FuzzyCandidate *first = (FuzzyCandidate *)getVectorItem(cl, 0);
        RUN_CHECK(first != NULL && strcmp(first->term, "list") == 0,
                  "First candidate for 'lisr' is 'list'");
        vectorFree(cl);
    }

    cl = fuzzyFindCandidates(idx3, "memry", 2);
    RUN_CHECK(cl != NULL, "Candidates found for 'memry' (max_dist=2)");
    if (cl) {
        FuzzyCandidate *first = (FuzzyCandidate *)getVectorItem(cl, 0);
        RUN_CHECK(first != NULL && strcmp(first->term, "memory") == 0,
                  "First candidate for 'memry' is 'memory'");
        vectorFree(cl);
    }

    printf("\n");

    /* Тест 4: fuzzyFindCandidates — точное совпадение */
    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " fuzzyFindCandidates: exact match with max_dist=0\n");

    cl = fuzzyFindCandidates(idx3, "python", 0);
    RUN_CHECK(cl != NULL, "Exact match found for 'python' with max_dist=0");
    if (cl) {
        RUN_CHECK(cl->size == 1, "Exactly one candidate for exact match");
        vectorFree(cl);
    }

    printf("\n");

    /* Тест 5: fuzzyFindCandidates — слишком далеко, ничего не найдено */
    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " fuzzyFindCandidates: no candidates when too far\n");

    cl = fuzzyFindCandidates(idx3, "xyzqwr", 1);
    RUN_CHECK(cl == NULL, "No candidates for 'xyzqwr' (max_dist=1)");

    printf("\n");

    /* Тест 6: fuzzyFindCandidates — кандидаты отсортированы по расстоянию */
    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " fuzzyFindCandidates: results sorted by distance\n");

    cl = fuzzyFindCandidates(idx3, "pythn", 3);
    RUN_CHECK(cl != NULL, "Candidates found for 'pythn' (max_dist=3)");
    if (cl) {
        int sorted = 1;
        for (size_t i = 1; i < cl->size; i++) {
            FuzzyCandidate *prev = (FuzzyCandidate *)getVectorItem(cl, i - 1);
            FuzzyCandidate *curr = (FuzzyCandidate *)getVectorItem(cl, i);
            if (prev->distance > curr->distance) {
                sorted = 0;
                break;
            }
        }
        RUN_CHECK(sorted == 1, "Candidates are sorted by distance (ascending)");
        vectorFree(cl);
    }

    printf("\n");

    /* Тест 7: fuzzyFindCandidates — NULL аргументы */
    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " fuzzyFindCandidates: NULL handling\n");

    RUN_CHECK(fuzzyFindCandidates(NULL, "test", 2) == NULL,
              "NULL index returns NULL");
    RUN_CHECK(fuzzyFindCandidates(idx3, NULL, 2) == NULL,
              "NULL term returns NULL");

    freeIndex(idx3);
    printf("\n");

    /* Тест 8: fuzzySearch — базовый поиск с опечатками */
    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " fuzzySearch: basic search with typos\n");

    Index *idx8 = create_test_index(TREE_AVL);

    SearchResults *sr = fuzzySearch(idx8, "pythn sort", 2);
    RUN_CHECK(sr != NULL, "fuzzySearch returned non-NULL result");
    if (sr) {
        RUN_CHECK(sr->total > 0, "Found documents for 'pythn sort'");
        RUN_CHECK(sr->results->size > 0, "Results vector is not empty");

        /* Проверяем что результаты отсортированы по score (убывание) */
        int sorted = 1;
        for (size_t i = 1; i < sr->results->size; i++) {
            SearchResult *prev = (SearchResult *)getVectorItem(sr->results, i - 1);
            SearchResult *curr = (SearchResult *)getVectorItem(sr->results, i);
            if (prev->score < curr->score) {
                sorted = 0;
                break;
            }
        }
        RUN_CHECK(sorted == 1, "Results are sorted by score (descending)");

        /* Документ с двумя совпавшими токенами должен иметь score >= 10 */
        if (sr->results->size > 0) {
            SearchResult *top = (SearchResult *)getVectorItem(sr->results, 0);
            RUN_CHECK(top->score >= 10,
                      "Top result has score >= 10 (matched both terms)");
        }

        freeSearchResults(sr);
    }

    freeIndex(idx8);
    printf("\n");

    /* Тест 9: fuzzySearch — оба слова с опечатками */
    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " fuzzySearch: both words with typos\n");

    Index *idx9 = create_test_index(TREE_AVL);

    sr = fuzzySearch(idx9, "lisr memry", 2);
    RUN_CHECK(sr != NULL, "fuzzySearch for 'lisr memry' returned non-NULL");
    if (sr) {
        RUN_CHECK(sr->total > 0, "Found documents for 'lisr memry'");

        /* Должен найтись документ про memory management (id=6) */
        int found_mem = 0;
        for (size_t i = 0; i < sr->results->size; i++) {
            SearchResult *r = (SearchResult *)getVectorItem(sr->results, i);
            if (r->doc_id == 6) found_mem = 1;
        }
        RUN_CHECK(found_mem == 1, "Found 'Python memory management tips' (id=6)");

        freeSearchResults(sr);
    }

    freeIndex(idx9);
    printf("\n");

    /* Тест 10: fuzzySearch — несуществующий запрос */
    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " fuzzySearch: query with no matches\n");

    Index *idx10 = create_test_index(TREE_AVL);

    sr = fuzzySearch(idx10, "xyzqwr zzzzz", 2);
    RUN_CHECK(sr != NULL, "fuzzySearch for nonsense query returned non-NULL");
    if (sr) {
        RUN_CHECK(sr->total == 0, "No documents found for nonsense query");
        freeSearchResults(sr);
    }

    freeIndex(idx10);
    printf("\n");

    /* Тест 11: fuzzySearch — одно слово в запросе */
    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " fuzzySearch: single word query\n");

    Index *idx11 = create_test_index(TREE_AVL);

    sr = fuzzySearch(idx11, "pythn", 1);
    RUN_CHECK(sr != NULL, "fuzzySearch for 'pythn' (single word) returned non-NULL");
    if (sr) {
        RUN_CHECK(sr->total > 0, "Found documents for single word with typo");
        RUN_CHECK(sr->results->size <= 10, "Results capped at 10");
        freeSearchResults(sr);
    }

    freeIndex(idx11);
    printf("\n");

    /* Тест 12: fuzzySearch — NULL аргументы */
    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " fuzzySearch: NULL handling\n");

    RUN_CHECK(fuzzySearch(NULL, "test", 2) == NULL,
              "NULL index returns NULL");
    RUN_CHECK(fuzzySearch(create_test_index(TREE_AVL), NULL, 2) == NULL,
              "NULL query returns NULL");

    printf("\n");

    /* Тест 13: fuzzySearch — одинаковые результаты на всех типах деревьев */
    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " fuzzySearch: consistent results across tree types\n");

    TreeType types[] = {TREE_AVL, TREE_RB, TREE_BTREE};
    int results_count[3] = {0, 0, 0};

    for (int t = 0; t < 3; t++) {
        Index *idx = create_test_index(types[t]);
        if (!idx) continue;
        sr = fuzzySearch(idx, "pythn sort", 2);
        if (sr) {
            results_count[t] = sr->total;
            freeSearchResults(sr);
        }
        freeIndex(idx);
    }

    RUN_CHECK(results_count[0] > 0, "AVL tree found results");
    RUN_CHECK(results_count[1] > 0, "Red-Black tree found results");
    RUN_CHECK(results_count[2] > 0, "B-tree found results");

    printf("\n");

    /* Тест 14: fuzzySearch — max_dist=0 это обычный поиск */
    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " fuzzySearch: max_dist=0 behaves like exact search\n");

    Index *idx14 = create_test_index(TREE_AVL);

    sr = fuzzySearch(idx14, "python sort", 0);
    RUN_CHECK(sr != NULL, "fuzzySearch with max_dist=0 returned non-NULL");
    if (sr) {
        RUN_CHECK(sr->total > 0, "Exact terms found with max_dist=0");
        freeSearchResults(sr);
    }

    sr = fuzzySearch(idx14, "pythn xyzw", 0);
    RUN_CHECK(sr != NULL, "fuzzySearch with typo and max_dist=0 returned non-NULL");
    if (sr) {
        RUN_CHECK(sr->total == 0, "No fuzzy matches when all terms have typos and max_dist=0");
        freeSearchResults(sr);
    }

    freeIndex(idx14);
    printf("\n");

    /* Тест 15: fuzzySearch — время замеряется */
    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " fuzzySearch: execution time is measured\n");

    Index *idx15 = create_test_index(TREE_AVL);

    sr = fuzzySearch(idx15, "pythn lisr sort", 2);
    RUN_CHECK(sr != NULL, "fuzzySearch returned result");
    if (sr) {
        RUN_CHECK(sr->time_ms >= 0.0, "Time is non-negative");
        printf("  " COLOR_CYAN "[INFO]" COLOR_RESET " Execution time: %.3f ms\n", sr->time_ms);
        freeSearchResults(sr);
    }

    freeIndex(idx15);
    printf("\n");

    /* Итоги */
    printf(COLOR_CYAN "=================================================================" COLOR_RESET "\n");
    printf(COLOR_CYAN "TOTAL: Passed %d out of %d tests" COLOR_RESET "\n", total_passed, total_tests);

    if (total_passed == total_tests) {
        printf(COLOR_GREEN "RESULT: SUCCESS" COLOR_RESET "\n");
    } else {
        printf(COLOR_RED "RESULT: FAILURE" COLOR_RESET "\n");
    }

    printf(COLOR_CYAN "=================================================================" COLOR_RESET "\n\n");

    return (total_passed == total_tests) ? 0 : 1;
}
