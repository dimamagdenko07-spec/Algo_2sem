/*
 * fuzzy.c - нечёткий поиск по инвертированному индексу
 *
 * Идея: для каждого слова запроса ищем похожие термины в индексе
 * через расстояние Левенштейна, потом объединяем posting list'ы
 * и ранжируем результаты.
 *
 * Оптимизации (чтобы не было совсем тормозно на реальных данных):
 * 1) Отсев по разнице длин - если |len(a)-len(b)| > max_dist,
 *    то и расстояние Левенштейна точно больше, можно не считать
 * 2) Ранний выход в DP - если минимальное значение в строке
 *    уже превысило max_dist, дальше считать нет смысла
 * 3) Два массива вместо полной таблицы - экономия памяти
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "fuzzy.h"

/* Максимум слов в одном запросе */
#define MAX_QUERY_TOKENS 64

/* Вспомогательная функция: минимум из двух чисел */
static int min2(int a, int b) {
    return a < b ? a : b;
}

/* Вспомогательная функция: минимум из трёх чисел */
static int min3(int a, int b, int c) {
    return min2(min2(a, b), c);
}

/*
 * bounded_levenshtein - расстояние Левенштейна с ограничением сверху.
 *
 * Считает обычное расстояние Левенштейна между s1 и s2,
 * но если результат точно больше max_d - сразу возвращает max_d + 1.
 * Это нужно чтобы не тратить время на заведомо неподходящие слова.
 *
 * Идея раннего выхода: после заполнения каждой строки DP-таблицы
 * смотрим на минимум в строке. Если он > max_d, то и финальный
 * результат будет > max_d, потому что значения в таблице
 * монотонно не убывают при движении вправо-вниз.
 */
static int bounded_levenshtein(const char *s1, const char *s2, int max_d) {
    if (!s1 || !s2) return max_d + 1;

    int len1 = (int)strlen(s1);
    int len2 = (int)strlen(s2);

    /* Быстрая проверка: если разница длин больше max_d,
     * то расстояние точно больше max_d (каждый лишний символ
     * - это минимум одна вставка/удаление) */
    int diff = len1 - len2;
    if (diff < 0) diff = -diff;
    if (diff > max_d) return max_d + 1;

    /* Делаем так чтобы s1 была короче - меньше итераций во внешнем цикле */
    if (len1 > len2) {
        const char *tmp_s = s1;
        s1 = s2;
        s2 = tmp_s;
        int tmp_l = len1;
        len1 = len2;
        len2 = tmp_l;
    }

    /* Два массива вместо двумерной таблицы - экономим память.
     * prev_row - предыдущая строка DP-таблицы
     * curr_row - текущая строка, которую мы заполняем */
    int *prev_row = (int *)malloc((len2 + 1) * sizeof(int));
    int *curr_row = (int *)malloc((len2 + 1) * sizeof(int));
    if (!prev_row || !curr_row) {
        free(prev_row);
        free(curr_row);
        return max_d + 1; /* при ошибке памяти просто говорим "не подходит" */
    }

    /* Первая строка: превращение пустой строки в s2[0..j-1] */
    for (int j = 0; j <= len2; j++) {
        prev_row[j] = j;
    }

    for (int i = 1; i <= len1; i++) {
        curr_row[0] = i; /* превращение s1[0..i-1] в пустую строку */
        int row_min = curr_row[0]; /* минимум в текущей строке */

        for (int j = 1; j <= len2; j++) {
            if (s1[i - 1] == s2[j - 1]) {
                /* Символы совпали - стоимость 0, берём диагональ */
                curr_row[j] = prev_row[j - 1];
            } else {
                /* Символы не совпали - берём минимум из трёх вариантов:
                 * удаление:    prev_row[j]   + 1
                 * вставка:     curr_row[j-1] + 1
                 * замена:      prev_row[j-1] + 1 */
                curr_row[j] = 1 + min3(
                    prev_row[j],
                    curr_row[j - 1],
                    prev_row[j - 1]
                );
            }
            /* Обновляем минимум в строке */
            if (curr_row[j] < row_min) {
                row_min = curr_row[j];
            }
        }

        /* Если минимум в строке уже больше порога - дальше считать нет смысла */
        if (row_min > max_d) {
            free(prev_row);
            free(curr_row);
            return max_d + 1;
        }

        /* Меняем строки местами: текущая становится предыдущей */
        int *tmp = prev_row;
        prev_row = curr_row;
        curr_row = tmp;
    }

    int result = prev_row[len2];
    free(prev_row);
    free(curr_row);
    return result;
}

/* Публичная функция: расстояние Левенштейна без ограничения.
 * Просто вызываем bounded_levenshtein с большим порогом. */
int levenshteinDistance(const char *s1, const char *s2) {
    if (!s1 || !s2) return -1;
    return bounded_levenshtein(s1, s2, 9999);
}

/*
 * Структура для передачи данных в callback при обходе дерева.
 * traverseIndex вызывает функцию visit для каждого ключа,
 * а нам нужно передать: что ищем, порог и куда складывать результаты.
 */
typedef struct {
    const char *term;         /* слово из запроса */
    int         max_distance; /* порог расстояния */
    Vector     *candidates;   /* сюда складываем FuzzyCandidate */
} FuzzyTraverseCtx;

/*
 * callback_fuzzy_visit - вызывается для каждого термина в дереве.
 *
 * Проверяет, находится ли термин на расстоянии <= max_distance
 * от искомого слова. Если да - добавляет в список кандидатов.
 */
static void callback_fuzzy_visit(const char *key, Vector *postings, void *ctx) {
    FuzzyTraverseCtx *fc = (FuzzyTraverseCtx *)ctx;

    /* Отсев по разнице длин - быстрая проверка O(1) */
    int key_len = (int)strlen(key);
    int term_len = (int)strlen(fc->term);
    int diff = key_len - term_len;
    if (diff < 0) diff = -diff;
    if (diff > fc->max_distance) return; /* точно далеко, пропускаем */

    /* Считаем расстояние с ограничением (может быстро отсечься) */
    int dist = bounded_levenshtein(key, fc->term, fc->max_distance);
    if (dist > fc->max_distance) return; /* далеко, пропускаем */

    /* Подходит! Создаём кандидата и добавляем в список */
    FuzzyCandidate cand;
    strncpy(cand.term, key, 255);
    cand.term[255] = '\0';
    cand.distance = dist;
    cand.postings = postings; /* это указатель из дерева, НЕ освобождаем! */

    appendVectorItem(fc->candidates, &cand);
}

/*
 * Сравнение для qsort - сортируем кандидатов по расстоянию (ближе = лучше).
 * Если расстояние одинаковое - по алфавиту.
 */
static int cmp_candidates(const void *a, const void *b) {
    const FuzzyCandidate *ca = (const FuzzyCandidate *)a;
    const FuzzyCandidate *cb = (const FuzzyCandidate *)b;
    if (ca->distance != cb->distance) {
        return ca->distance - cb->distance;
    }
    return strcmp(ca->term, cb->term);
}

/*
 * fuzzyFindCandidates - находит все термины в индексе, похожие на term.
 *
 * Алгоритм:
 * 1) Обходим всё дерево через traverseIndex
 * 2) Для каждого термина считаем расстояние Левенштейна
 * 3) Если distance <= max_distance - добавляем в результат
 * 4) Сортируем результат по расстоянию
 */
Vector *fuzzyFindCandidates(Index *idx, const char *term, int max_distance) {
    if (!idx || !term) {
        printf("fuzzyFindCandidates: idx or term is NULL\n");
        return NULL;
    }

    /* Создаём пустой вектор кандидатов */
    Vector *candidates = createVector(sizeof(FuzzyCandidate));
    if (!candidates) return NULL;

    /* Заполняем контекст для callback */
    FuzzyTraverseCtx ctx;
    ctx.term = term;
    ctx.max_distance = max_distance;
    ctx.candidates = candidates;

    /* Обходим всё дерево - для каждого ключа вызовется callback */
    traverseIndex(idx, callback_fuzzy_visit, &ctx);

    /* Если ничего не нашли - возвращаем NULL */
    if (candidates->size == 0) {
        vectorFree(candidates);
        return NULL;
    }

    /* Сортируем по расстоянию - ближайшие первые */
    qsort(candidates->data, candidates->size, candidates->elem_size,
          cmp_candidates);

    return candidates;
}

/*
 * Структура для агрегации результатов по документам.
 *
 * Для каждого документа мы храним:
 * - сколько токенов запроса в нём совпали (matched_terms)
 * - суммарное расстояние всех совпадений (total_distance)
 * - для каждого токена - нашли ли совпадение и лучшее расстояние
 *
 * Это нужно чтобы:
 * 1) Не дублировать документ если он совпал с одним токеном через
 *    несколько разных кандидатов
 * 2) Правильно считать score = matched_terms * 10 - avg_distance
 */
typedef struct {
    int  doc_id;
    char title[MAX_TITLE_LEN];
    int  matched_terms;                              /* сколько токенов совпало */
    int  total_distance;                             /* сумма лучших расстояний */
    int  token_matched[MAX_QUERY_TOKENS];            /* 1 если i-й токен совпал */
    int  token_best_dist[MAX_QUERY_TOKENS];          /* лучшее расстояние для i-го токена */
} AggDoc;

/*
 * Сравнение для сортировки документов по score (по убыванию).
 * score = matched_terms * 10 - avg_distance
 * Документы с большим score должны идти первыми.
 */
static int cmp_agg_docs(const void *a, const void *b) {
    const AggDoc *da = (const AggDoc *)a;
    const AggDoc *db = (const AggDoc *)b;

    double score_a = 0.0, score_b = 0.0;
    if (da->matched_terms > 0)
        score_a = da->matched_terms * 10.0 - (double)da->total_distance / da->matched_terms;
    if (db->matched_terms > 0)
        score_b = db->matched_terms * 10.0 - (double)db->total_distance / db->matched_terms;

    if (score_b > score_a) return  1;
    if (score_b < score_a) return -1;
    return da->doc_id - db->doc_id;
}

/*
 * fuzzySearch - нечёткий поиск по запросу с ранжированием.
 *
 * Алгоритм:
 * 1) Разбиваем запрос на отдельные слова-токены
 * 2) Для каждого токена находим похожие термины (fuzzyFindCandidates)
 * 3) Обходим posting list'ы всех кандидатов
 * 4) Агрегируем результаты по документам:
 *    - Считаем сколько токенов запроса "накрылись" (хотя бы один кандидат)
 *    - Для каждого токена берём лучшее (минимальное) расстояние
 * 5) Ранжируем по формуле: score = matched_terms * 10 - avg_distance
 * 6) Возвращаем топ-10
 */
SearchResults *fuzzySearch(Index *idx, const char *query, int max_distance) {
    if (!idx || !query) {
        printf("fuzzySearch: idx or query is NULL\n");
        return NULL;
    }

    clock_t start = clock();

    /* Создаём структуру результата */
    SearchResults *sr = (SearchResults *)calloc(1, sizeof(SearchResults));
    if (!sr) return NULL;
    sr->results = createVector(sizeof(SearchResult));
    if (!sr->results) {
        free(sr);
        return NULL;
    }

    /* Копируем запрос потому что strtok портит строку */
    char *query_copy = strdup(query);
    if (!query_copy) {
        freeSearchResults(sr);
        return NULL;
    }

    /* Разбиваем запрос на токены */
    char *tokens[MAX_QUERY_TOKENS];
    int n_tokens = 0;

    char *tok = strtok(query_copy, " \t\n\r,.");
    while (tok != NULL && n_tokens < MAX_QUERY_TOKENS) {
        tokens[n_tokens] = tok;
        n_tokens++;
        tok = strtok(NULL, " \t\n\r,.");
    }

    if (n_tokens == 0) {
        free(query_copy);
        return sr; /* пустой запрос - пустой результат */
    }

    /* Вектор для агрегации документов */
    Vector *agg = createVector(sizeof(AggDoc));
    if (!agg) {
        free(query_copy);
        freeSearchResults(sr);
        return NULL;
    }

    /* Основной цикл: для каждого токена запроса */
    for (int t = 0; t < n_tokens; t++) {
        /* Ищем похожие термины в индексе */
        Vector *candidates = fuzzyFindCandidates(idx, tokens[t], max_distance);
        if (!candidates) {
            continue; /* нет кандидатов - пропускаем, но не выходим */
        }

        /* Обходим всех кандидатов и их posting list'ы */
        for (size_t ci = 0; ci < candidates->size; ci++) {
            FuzzyCandidate *cand = (FuzzyCandidate *)getVectorItem(candidates, ci);
            if (!cand || !cand->postings) continue;

            for (size_t pi = 0; pi < cand->postings->size; pi++) {
                PostingEntry *pe = (PostingEntry *)getVectorItem(cand->postings, pi);
                if (!pe) continue;

                /* Ищем - уже есть этот документ в агрегации? */
                int found_idx = -1;
                for (size_t ai = 0; ai < agg->size; ai++) {
                    AggDoc *ad = (AggDoc *)getVectorItem(agg, ai);
                    if (ad->doc_id == pe->doc_id) {
                        found_idx = (int)ai;
                        break;
                    }
                }

                if (found_idx >= 0) {
                    /* Документ уже есть - обновляем информацию */
                    AggDoc *ad = (AggDoc *)getVectorItem(agg, found_idx);

                    if (!ad->token_matched[t]) {
                        /* Первый кандидат для этого токена запроса */
                        ad->token_matched[t] = 1;
                        ad->token_best_dist[t] = cand->distance;
                        ad->matched_terms++;
                        ad->total_distance += cand->distance;
                    } else if (cand->distance < ad->token_best_dist[t]) {
                        /* Нашли кандидата поближе для этого токена */
                        ad->total_distance -= ad->token_best_dist[t];
                        ad->total_distance += cand->distance;
                        ad->token_best_dist[t] = cand->distance;
                    }

                    /* Обновляем title если было default_title */
                    if (strcmp(ad->title, "default_title") == 0 &&
                        strcmp(pe->title, "default_title") != 0) {
                        strncpy(ad->title, pe->title, MAX_TITLE_LEN - 1);
                        ad->title[MAX_TITLE_LEN - 1] = '\0';
                    }
                } else {
                    /* Новый документ - добавляем */
                    AggDoc new_doc;
                    memset(&new_doc, 0, sizeof(AggDoc));
                    new_doc.doc_id = pe->doc_id;
                    strncpy(new_doc.title, pe->title, MAX_TITLE_LEN - 1);
                    new_doc.title[MAX_TITLE_LEN - 1] = '\0';
                    new_doc.token_matched[t] = 1;
                    new_doc.token_best_dist[t] = cand->distance;
                    new_doc.matched_terms = 1;
                    new_doc.total_distance = cand->distance;
                    appendVectorItem(agg, &new_doc);
                }
            }
        }

        /* Освобождаем список кандидатов для этого токена */
        vectorFree(candidates);
    }

    free(query_copy);

    /* Сортируем документы по score (по убыванию) */
    if (agg->size > 0) {
        qsort(agg->data, agg->size, agg->elem_size, cmp_agg_docs);
    }

    /* Берём топ-10 */
    int top_n = (int)agg->size;
    if (top_n > 10) top_n = 10;

    for (int i = 0; i < top_n; i++) {
        AggDoc *ad = (AggDoc *)getVectorItem(agg, (size_t)i);

        SearchResult res;
        res.doc_id = ad->doc_id;
        strncpy(res.title, ad->title, MAX_TITLE_LEN - 1);
        res.title[MAX_TITLE_LEN - 1] = '\0';

        /* score = matched_terms * 10 - среднее_расстояние */
        double avg_dist = 0.0;
        if (ad->matched_terms > 0) {
            avg_dist = (double)ad->total_distance / ad->matched_terms;
        }
        res.score = (int)(ad->matched_terms * 10 - avg_dist);

        appendVectorItem(sr->results, &res);
    }

    /* total - сколько всего документов нашли (до top-10) */
    sr->total = (int)agg->size;

    vectorFree(agg);

    clock_t end = clock();
    sr->time_ms = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;

    return sr;
}