import matplotlib.pyplot as plt
import numpy as np
import random
import math

time = 10000
nb_tests = 10000
nb_directions = 4
round_resolution = 5

datas = {}

for _ in range(nb_tests):
    x = 0
    y = 0

    for i in range(time):
        angle = float(random.randrange(0, nb_directions)) / nb_directions * 2 * math.pi
        x += math.cos(angle)
        y += math.sin(angle)
        
    d = round( math.sqrt(x**2 + y**2) / round_resolution ) * round_resolution
    if d not in datas.keys():
        datas[d] = 1
    else:
        datas[d] += 1


# for key in sorted(datas.keys()):
#     print("{} {}".format(key, datas[key]))



absciss = np.array(sorted(datas.keys()))
ordinate = [
    ("repartition (rounded aroud {})".format(round_resolution), np.array([datas[key] for key in sorted(datas.keys())]))
]


plt.xlim(min(absciss) , max(absciss))

ymin = min([min(datas) for (_,datas) in ordinate])
ymax = max([max(datas) for (_,datas) in ordinate])
yrange = ymax - ymin

y_border_percent = 7.5
y_bottom = ymin - y_border_percent * yrange / 100
y_top = ymax + y_border_percent * yrange / 100

plt.ylim(y_bottom, y_top)


plt.title("Repartition of distance from origin of {} individus in a 2D random walk".format(nb_tests))
plt.ylabel('#individus')
plt.xlabel("distance from origin")

for (label,ydatas) in ordinate:
    plt.plot(absciss, ydatas, label=label)
    
legend = plt.legend(loc='upper right', shadow=True, fontsize='medium')

plt.show()