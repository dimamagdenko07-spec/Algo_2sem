import numpy as np
from tree import Tree, Record, SearchResult, cos_sim

class YourIndex:
    def __init__(self):
        self.index = Tree()

    def insert(
        self,
        vector: np.ndarray,
        item_id: str,
        payload: Record | None = None,
    ) -> None:
        self.index.insert_vector(vector, payload)

    def search(
        self,
        query: np.ndarray,
        k: int = 5,
    ) -> list[SearchResult]:
        return self.index.search_vector(query, k)

    def clear(self):
        self.root = None
        self.size = 0