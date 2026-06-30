import numpy as np
import random
from main import YourIndex
from tree import Record
from embedding import Embeddings


def add_trash(text):
    symbols = "!@#$%^&*()_+"
    prefix = "".join(random.choices(symbols, k=3))
    suffix = "".join(random.choices(symbols, k=3))
    return f"{prefix} {text} {suffix}"

def run_task_test():
    emb_model = Embeddings()
    index = YourIndex()


    task = [
        "Орбитальный спутник меняет курс",
        "Расчёт баллистической траектории ракеты",
        "МКС выполняет манёвр уклонения",
        "Двигатели космического корабля",
        "Гравитационный манёвр у Юпитера",
        "Стыковка модулей на околоземной орбите",
        "Запуск автоматической межпланетной станции",
        "Подготовка космонавтов к выходу в открытый космос",
        "Система жизнеобеспечения лунной базы",
        "Исследование радиационных поясов Земли"
    ]


    task = task + [
        "Коррекция траектории полета к Марсу",
        "Телеметрия со спутника связи передается на Землю",
        "Солнечные батареи раскрылись в штатном режиме",
        "Аппарат вошел в плотные слои атмосферы",
        "Космический мусор угрожает безопасности станции",
        "Изучение черных дыр с помощью рентгеновского телескопа",
        "Посадочный модуль совершил мягкое приземление",
        "Космодром Байконур готовит ракету к старту",
        "Спектральный анализ далеких звездных систем",
        "Поиск экзопланет в созвездии Ориона"
    ]
    ids = [42, 17, 88, 5, 61] + [random.randint(100, 999) for _ in range(15)]

    print(f"Индекс построен на N={len(task)} текстах.\n")


    for i in range(len(task)):
        original_text = task[i]
        noisy_text = add_trash(original_text)
        
        record = Record(id=ids[i], text=noisy_text)
        vector = emb_model.embed(noisy_text)
        
        index.insert(vector=vector, item_id=str(ids[i]), payload=record)

    query_text = "космический аппарат и его траектория"
    query_vector = emb_model.embed(query_text)
    
    print(f"Запрос: \"{query_text}\"\n")
    print("Результат:")
    
    results = index.search(query=query_vector, k=5)


    for i, res in enumerate(results, 1):
        clean_text = res.payload.text
        print(f"{i}. id={res.payload.id:<3} distance={res.distance:.3f}   \"{clean_text}\"")

if __name__ == "__main__":
    run_task_test()