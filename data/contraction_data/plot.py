import matplotlib.pyplot as plt
import csv
import numpy as np
import statistics

def readCSV(file_name, delimiter=' '):
    file = csv.DictReader(open(file_name), delimiter=delimiter)
    return list([row for row in file])

rows = readCSV('/home/plaiseek/Projects/landscape_opt/data/contraction_data/data.log', " ")


def substract(a, b):
    return [x-y for x,y in zip(a,b)]
def divide(a, b):
    return [x/y if y > 0 else 1 for x,y in zip(a,b)]


x_datas = np.array(range(0,105,5))

def get_datas(name, linstyle, maker_size, t):
    return ((name, (linstyle,maker_size)), (x_datas,
        np.array([statistics.mean([100*(1-float(row["nb_{}_contract".format(t)])/float(row["nb_{}".format(t)])) for row in rows if int(row["percent_arcs"]) == percent]) for percent in x_datas]) ))

datas = [
    get_datas("constraints", "s-",8, "constraints"),
    get_datas("variables", "o-",8, "vars"),
    get_datas("non-zero entries", "P-",8, "entries")
]


fig_size = plt.rcParams["figure.figsize"]
fig_size[0] = 10
fig_size[1] = 5
plt.rcParams["figure.figsize"] = fig_size

plt.subplots_adjust(left=0.125, right=0.95, top=0.92, bottom=0.13)

plt.rcParams.update({'font.size': 16})

xmin = min(x_datas)
xmax = max(x_datas)

plt.xlim(xmin , xmax)

ymin = 0
ymax = 100
yrange = ymax - ymin

y_border_percent = 7.5
y_bottom = ymin - y_border_percent * yrange / 100
y_top = ymax + y_border_percent * yrange / 100

plt.ylim(y_bottom, y_top)


# plt.title("quebec-{}-{}-ECA value vs available budget.pdf".format(orig, median))
plt.ylabel('percentage of elements\nremoved by the preprocessing', rotation=90, fontweight ='bold')
plt.xlabel("percentage of restored arcs", fontweight ='bold')

for ((label,(linestyle,marker_size)),(xdatas,ydatas)) in datas:
    plt.plot(xdatas, ydatas, linestyle, markersize=marker_size, label=label)
    
legend = plt.legend(loc='upper right', shadow=True, fontsize='medium')


plt.savefig("data/contraction_data/preprocessing_benefits.pdf", dpi=500)