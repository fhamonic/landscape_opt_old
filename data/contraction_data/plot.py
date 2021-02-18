import matplotlib.pyplot as plt
import csv
import numpy as np

def readCSV(file_name, delimiter=' '):
    file = csv.DictReader(open(file_name), delimiter=delimiter)
    return list([row for row in file])

rows = readCSV('/home/plaiseek/Projects/landscape_opt_cpp/data/contraction_data/size_500.csv', " ")

x_datas = np.array([int(row['arcs']) for row in rows])

datas = [
    (("constraints", (".-",8)), (x_datas, np.array([100-float(row["constrs"]) for row in rows]))),
    (("variables", (".-",8)), (x_datas, np.array([100-float(row["vars"]) for row in rows]))),
    (("model size", (".-",8)), (x_datas, np.array([100 - (float(row["constrs"])*float(row["vars"])/100) for row in rows])))
]


fig_size = plt.rcParams["figure.figsize"]
fig_size[0] = 10
fig_size[1] = 5
plt.rcParams["figure.figsize"] = fig_size

plt.subplots_adjust(left=0.085, right=0.95, top=0.92, bottom=0.13)

plt.rcParams.update({'font.size': 16})

# print(absciss)
# print(ordinate)

xmin = min([min(xdatas) for (_,(xdatas,_)) in datas])
xmax = max([max(xdatas) for (_,(xdatas,_)) in datas])

plt.xlim(xmin , xmax)

ymin = 0
ymax = 100
yrange = ymax - ymin

y_border_percent = 7.5
y_bottom = ymin - y_border_percent * yrange / 100
y_top = ymax + y_border_percent * yrange / 100

plt.ylim(y_bottom, y_top)


# plt.title("quebec-{}-{}-ECA value vs available budget.pdf".format(orig, median))
plt.ylabel('opt ratio', rotation=90, fontweight ='bold')
plt.xlabel("percentage of restored arcs", fontweight ='bold')

for ((label,(linestyle,marker_size)),(xdatas,ydatas)) in datas:
    plt.plot(xdatas, ydatas, linestyle, markersize=marker_size, label=label)
    
legend = plt.legend(loc='upper right', shadow=True, fontsize='medium')


plt.savefig("data/contraction_data/preprocessing_benefits.pdf", dpi=500)