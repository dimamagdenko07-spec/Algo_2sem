from __future__ import annotations
import numpy as np
from dataclasses import dataclass

@dataclass(frozen=True)
class Record:
    '''Структура, хранящая id и текст'''
    id: int
    text: str

@dataclass(frozen=True)
class SearchResult:
    '''Структура, описывающая узел по отношению к запросу'''
    id: str
    distance: float
    score: float
    path: tuple[str, ...]
    payload: Record | None

@dataclass
class Node:
    '''Узел дерева'''
    info: Record | None
    emb: np.ndarray
    parent: Node | None = None
    left: Node | None = None
    right: Node | None = None

def cos_sim(vec1: np.ndarray, vec2: np.ndarray):
    '''Вычисление косинусного сходства'''
    norm = np.linalg.norm(vec1) * np.linalg.norm(vec2)
    return (vec1 @ vec2) / norm if norm != 0 else 0.0

def search_k_recursion(node: Node, vec: np.ndarray, k: int, curr_path: str, results: list):
    if node is None:
        return
    sim = cos_sim(vec, node.emb)
    results.append({
        "id": str(node.info.id) if node.info else "None",
        "sim": sim,
        "path": curr_path,
        "info": node.info
    })
    results.sort(key=lambda x: x["sim"], reverse=True)
    if len(results) > k:
        results.pop()

    left, right = node.left, node.right
    if left is None and right is None:
        return

    sim_l = cos_sim(vec, left.emb) if left else -float('inf')
    sim_r = cos_sim(vec, right.emb) if right else -float('inf')

    if sim_l > sim_r:
        search_k_recursion(left, vec, k, curr_path + 'l', results)
        if len(results) < k or sim_r > results[-1]["sim"]:
            search_k_recursion(right, vec, k, curr_path + 'r', results)
    else:
        search_k_recursion(right, vec, k, curr_path + 'r', results)
        if len(results) < k or sim_l > results[-1]["sim"]:
            search_k_recursion(left, vec, k, curr_path + 'l', results)

class Tree:
    def __init__(self):
        self.root = None
        self.size = 0

    def insert_vector(self, vector: np.ndarray, payload: Record | None):
        self.size += 1
        if self.root is None:
            self.root = Node(info=payload, emb=vector)
            return
        curr = self.root
        while curr.left is not None and curr.right is not None:
            if cos_sim(vector, curr.left.emb) < cos_sim(vector, curr.right.emb):
                curr = curr.right
            else:
                curr = curr.left
        new_node = Node(info=payload, emb=vector, parent=curr)
        if curr.left is None:
            curr.left = new_node
        else:
            curr.right = new_node

    def search_vector(self, query: np.ndarray, k: int) -> list[SearchResult]:
        if self.root is None:
            return []
        results_list = []
        search_k_recursion(self.root, query, k, '', results_list)
        list = []
        for res in results_list:
            list.append(SearchResult(
                id=res["id"],
                distance=float(1.0 - res["sim"]),
                score=float(res["sim"]),
                path=tuple(res["path"]),
                payload=res["info"]
            ))
        return list
        