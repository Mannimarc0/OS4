import glob
import os
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Patch

NUM_PAGES = 16


# РАЗБОР ОДНОГО ЖУРНАЛЬНОГО ФАЙЛА В СПИСОК СОБЫТИЙ
def parse(path):
    events = []
    with open(path) as f:
        for line in f:
            parts = line.split()
            if len(parts) < 2:
                continue
            t = int(parts[0])
            state = parts[1]
            page = int(parts[2]) if len(parts) > 2 else -1
            events.append((t, state, page))
    return events


reader_files = sorted(glob.glob("reader_*.log"))
writer_files = sorted(glob.glob("writer_*.log"))
files = reader_files + writer_files
if not files:
    print("No log files found. Run the script in the same folder as the .log files.")
    exit()

data = {os.path.basename(f).replace(".log", ""): parse(f) for f in files}

all_t = [e[0] for ev in data.values() for e in ev]
t0 = min(all_t) if all_t else 0
t_max = (max(all_t) - t0) if all_t else 1

# ПОРЯДОК ОТ ЧИТАТЕЛЕЙ ВНИЗУ К ПИСАТЕЛЯМ ВВЕРХУ И КРАСИВЫЕ ПОДПИСИ
order = []
labels = []
ri, wi = 1, 1
for f in reader_files:
    name = os.path.basename(f).replace(".log", "")
    order.append(name)
    labels.append("Reader_%d" % ri)
    ri += 1
for f in writer_files:
    name = os.path.basename(f).replace(".log", "")
    order.append(name)
    labels.append("Writer_%d" % wi)
    wi += 1

COLOR = {"WAIT": "#f4c20d", "READ": "#3c9d3c", "WRITE": "#e03c31"}

# ГРАФИК 1 ДИАГРАММА ГАНТА СМЕНЫ СОСТОЯНИЙ
plt.figure(figsize=(15, 8))
for idx, name in enumerate(order):
    ev = data[name]
    # СОБИРАЕМ ИНТЕРВАЛЫ СОСТОЯНИЙ ИЗ ПОСЛЕДОВАТЕЛЬНОСТИ СОБЫТИЙ
    for i in range(len(ev) - 1):
        t, state, pg = ev[i]
        nt = ev[i + 1][0]
        start = (t - t0) / 1000.0
        dur = (nt - t) / 1000.0
        if state == "BEGIN_WAITING":
            c = COLOR["WAIT"]
        elif state == "READ":
            c = COLOR["READ"]
        elif state == "WRITE":
            c = COLOR["WRITE"]
        else:
            continue  # BEGIN_RELEASE ДО СЛЕДУЮЩЕГО ОЖИДАНИЯ ПРОПУСКАЕМ
        plt.barh(idx, dur, left=start, height=0.6,
                 color=c, edgecolor="black", linewidth=0.5)

plt.yticks(range(len(order)), labels)
plt.xlabel("ВРЕМЯ ОТ НАЧАЛА РАБОТЫ, СЕК")
plt.ylabel("ПРОЦЕССЫ")
plt.title("ГРАФИК СМЕНЫ СОСТОЯНИЙ ПРОЦЕССОВ ЧИТАТЕЛЕЙ И ПИСАТЕЛЕЙ (ЗАДАНИЕ 4.1)")
legend = [Patch(facecolor=COLOR["WAIT"], edgecolor="black", label="ОЖИДАНИЕ"),
          Patch(facecolor=COLOR["READ"], edgecolor="black", label="ЧТЕНИЕ"),
          Patch(facecolor=COLOR["WRITE"], edgecolor="black", label="ЗАПИСЬ")]
plt.legend(handles=legend, loc="upper right")
plt.grid(True, axis="x", linestyle="--", alpha=0.5)
plt.tight_layout()
plt.savefig("state_changes.png", dpi=120)
print("Saved state_changes.png")

# ГРАФИК 2 ТЕПЛОВАЯ КАРТА ЗАНЯТОСТИ СТРАНИЦ ВО ВРЕМЕНИ
bins = 200
dt = (t_max / 1000.0) / bins if t_max > 0 else 1
heat = np.zeros((NUM_PAGES, bins))
for name, ev in data.items():
    start_t = None
    page = -1
    for (t, state, pg) in ev:
        if state in ("READ", "WRITE"):
            start_t = (t - t0) / 1000.0
            page = pg
        elif state == "BEGIN_RELEASE" and start_t is not None and 0 <= page < NUM_PAGES:
            end_t = (t - t0) / 1000.0
            b0 = max(0, int(start_t / dt))
            b1 = min(bins - 1, int(end_t / dt))
            for b in range(b0, b1 + 1):
                heat[page, b] += 1
            start_t = None

plt.figure(figsize=(15, 6))
plt.imshow(heat, aspect="auto", origin="lower", cmap="hot",
           extent=[0, t_max / 1000.0, -0.5, NUM_PAGES - 0.5])
plt.colorbar(label="АКТИВНЫХ ПРОЦЕССОВ")
plt.yticks(range(NUM_PAGES))
plt.xlabel("ВРЕМЯ, СЕК")
plt.ylabel("НОМЕР СТРАНИЦЫ")
plt.title("ЗАНЯТОСТЬ СТРАНИЦ ВО ВРЕМЕНИ (ЗАДАНИЕ 4.1)")
plt.tight_layout()
plt.savefig("page_heatmap.png", dpi=120)
print("Saved page_heatmap.png")

plt.show()