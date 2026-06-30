# Пишите сами друзья, можно даже вайбкодить!
from __future__ import annotations
import numpy as np
import streamlit as st
from streamlit_echarts import st_echarts
from typing import TypedDict
#ЗАПУСК streamlit run st_demo.py

from tree import Node, Record, SearchResult, Tree
from main import YourIndex
from embedding import Embeddings

# --- Вспомогательные классы для визуализации ---
class _EChartsNode(TypedDict):
    name: str
    children: list[_EChartsNode]

def binary_tree_to_echarts(node: Node | None, path: str = "") -> _EChartsNode | None:
    """Превращает твое бинарное дерево в формат для отрисовки графиков."""
    if node is None:
        return None

    # Формируем текст для узла
    label = f"ID: {node.info.id}\n{node.info.text[:20]}..."
    
    children = []
    # Добавляем левую ветку
    left_child = binary_tree_to_echarts(node.left, path + "l")
    if left_child: children.append(left_child)
    
    # Добавляем правую ветку
    right_child = binary_tree_to_echarts(node.right, path + "r")
    if right_child: children.append(right_child)

    return _EChartsNode(name=label, children=children)

def render_tree_chart(root: Node | None):
    """Отрисовка дерева через ECharts."""
    if root is None:
        st.info("Дерево пустое")
        return

    data = binary_tree_to_echarts(root)
    
    options = {
        "tooltip": {"trigger": "item", "triggerOn": "mousemove"},
        "series": [{
            "type": "tree",
            "data": [data],
            "top": "5%", "left": "10%", "bottom": "5%", "right": "20%",
            "symbolSize": 10,
            "orient": "LR",
            "label": {
                "position": "left", "verticalAlign": "middle", "align": "right", "fontSize": 11,
                "backgroundColor": "#eee", "borderColor": "#aaa", "borderWidth": 1, "borderRadius": 4, "padding": [4, 6]
            },
            "leaves": {"label": {"position": "right", "align": "left"}},
            "expandAndCollapse": True,
            "initialTreeDepth": -1
        }]
    }
    st_echarts(options=options, height="600px")

# --- Логика Session State ---
if "records" not in st.session_state:
    st.session_state.records = [] # Список объектов Record

def rebuild_index():
    """Пересобирает YourIndex на основе списка records."""
    new_index = YourIndex()
    emb = Embeddings()
    for rec in st.session_state.records:
        vec = emb.embed(rec.text)
        new_index.insert(vector=vec, item_id=str(rec.id), payload=rec)
    st.session_state.my_index = new_index

# --- Основной интерфейс ---
def main():
    st.set_page_config(page_title="Semantic Tree Demo", layout="wide")
    st.title("🌳 Semantic Binary Tree Index")

    emb_model = Embeddings()

    # Инициализация индекса при первом запуске
    if "my_index" not in st.session_state:
        rebuild_index()

    # --- Sidebar ---
    with st.sidebar:
        st.header("Управление")
        
        with st.form("add_form", clear_on_submit=True):
            st.write("**Добавить запись**")
            text_input = st.text_area("Текст")
            submitted = st.form_submit_button("Добавить в дерево")
            if submitted and text_input:
                new_id = np.random.randint(1, 9999)
                st.session_state.records.append(Record(id=new_id, text=text_input))
                rebuild_index()
                st.success(f"Запись {new_id} добавлена")
                st.rerun()

        with st.form("del_form"):
            st.write("**Удалить запись**")
            id_to_del = st.number_input("ID", step=1)
            if st.form_submit_button("Удалить", type="primary"):
                st.session_state.records = [r for r in st.session_state.records if r.id != id_to_del]
                rebuild_index()
                st.warning(f"ID {id_to_del} удален")
                st.rerun()

        st.write(f"Всего узлов: {len(st.session_state.records)}")

    # --- Поиск и Визуализация ---
    query = st.text_input("Введите запрос для поиска", "космический корабль")
    k = st.slider("Сколько найти (K)", 1, 10, 3)

    col_left, col_right = st.columns([1, 2])

    with col_left:
        st.subheader("Результаты поиска")
        if query and st.session_state.records:
            q_vec = emb_model.embed(query)
            results = st.session_state.my_index.search(q_vec, k=k)
            for res in results:
                st.success(f"**ID: {res.id}** (score: {res.score:.3f})\n\n{res.payload.text}")
                st.caption(f"Путь: {' → '.join(res.path)}")
        else:
            st.write("Добавьте данные и введите запрос.")

    with col_right:
        st.subheader("Визуализация структуры")
        render_tree_chart(st.session_state.my_index.index.root)

if __name__ == "__main__":
    main()