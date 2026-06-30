import numpy as np
import time
from dataclasses import dataclass
from tree import Tree, Record, SearchResult, cos_sim, search_k_recursion

def linear_search(
    vectors: np.ndarray,     # shape (N, dim)
    payloads: list[Record],
    query: np.ndarray,       # shape (dim,)
    k: int,
) -> list[SearchResult]:
    res = []
    sign = [False]*vectors.shape[0]
    for _ in range(k):
        max_id = 0
        cos_sim_max_id = cos_sim(query, vectors[max_id])
        for i in range(vectors.shape[0]):
            if sign[max_id] == True:
                if sign[i] == False:
                    max_id = i
                    cos_sim_max_id = cos_sim(query, vectors[max_id])
            if cos_sim(query, vectors[i]) > cos_sim_max_id and sign[i] == False:
                max_id = i
                cos_sim_max_id = cos_sim(query, vectors[max_id])
        if (max_id == 0 and sign[0] == True):
            break
        else:
            res.append(max_id)
            sign[max_id] = True
    search_res = []
    for i in range(len(res)):
        search_res.append(SearchResult(
            id=payloads[res[i]].id,
            distance=1-cos_sim(query, vectors[res[i]]),
            score=cos_sim(query, vectors[res[i]]),
            path = (),
            payload=payloads[res[i]]
        ))
    return search_res


def run_testing():
    dim = 384
    k = 10
    N_values = [100, 500, 1000, 5000, 10000]
    
    print(f"{'N':>7} | {'Linear (ms)':>12} | {'Tree (ms)':>12} | {'Speedup':>10}")
    print("-" * 50)

    for n in N_values:
        '''Генерация вектора'''
        vectors = np.random.randn(n, dim).astype(np.float32)
        vectors /= np.linalg.norm(vectors, axis=1)[:, np.newaxis]
        '''Информация'''
        payloads = [Record(id=i, text=f"text_{i}") for i in range(n)]
        query = np.random.randn(dim).astype(np.float32)
        query /= np.linalg.norm(query)

        '''Делаем дерево'''
        tree = Tree()
        for i in range(n):
            tree.insert_vector(vectors[i], payloads[i])

        '''Замеряем линеную функцию'''
        t0 = time.perf_counter()
        for _ in range(5):
            _ = linear_search(vectors, payloads, query, k)
        t1 = time.perf_counter()
        linear_time = ((t1 - t0) / 5) * 1000

        '''Замеряем дерево'''
        t0 = time.perf_counter()
        for _ in range(5):
            results_list = []
            search_k_recursion(tree.root, query, k, '', results_list)
        t1 = time.perf_counter()
        tree_time = ((t1 - t0) / 5) * 1000

        speedup = linear_time / tree_time if tree_time > 0 else 0
        
        print(f"{n:>7} | {linear_time:>12.2f} | {tree_time:>12.2f} | {speedup:>9.1f}x")

if __name__ == "__main__":
    run_testing()