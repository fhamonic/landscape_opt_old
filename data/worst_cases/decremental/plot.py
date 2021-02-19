import matplotlib.pyplot as plt
import csv
import numpy as np

def readCSV(file_name, delimiter=' '):
    file = csv.DictReader(open(file_name), delimiter=delimiter)
    return list([row for row in file])

def substract(a, b):
    return [x-y for x,y in zip(a,b)]
def divide(a, b):
    return [x/y if y > 0 else 1 for x,y in zip(a,b)]

rows = readCSV('data/worst_cases/decremental/data.log')


bogo_datas = np.array([float(row['total_eca']) for row in rows if row['solver'] == "bogo_seed=99"])
ref_datas = np.array([bogo_datas[0] for row in rows if row['solver'] == "bogo_seed=99"])
mip_gain_datas = substract(np.array([float(row['total_eca']) for row in rows if row['solver'] == "pl_eca_3_fortest=0_log=1_relaxed=0_timeout=3600"]), ref_datas)


def get_datas(name, linestyle, marker_size, value):
    return ((name, (linestyle, marker_size)), (
        np.array([float(row['budget']) for row in rows if row['solver'] == value]),
        divide(substract(np.array([float(row['total_eca']) for row in rows if row['solver'] == value]), ref_datas), mip_gain_datas)))

datas = [
    get_datas("incremental greedy", "^-", 11, "glutton_eca_inc_log=0_parallel=1"),
    get_datas("", " ", 8, "bogo_seed=99"),
    get_datas("decremental greedy", "v-", 11, "glutton_eca_dec_log=0_parallel=1"),
    get_datas("optimum (MIP)", "s-", 8, "pl_eca_3_fortest=0_log=1_relaxed=0_timeout=3600"),
    # get_datas("randomized rounding", "*-", "randomized_rounding_draws=1000_log=0_parallel=1")
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

ymin = min([min(ydatas) for (_,(_,ydatas)) in datas])
ymax = max([max(ydatas) for (_,(_,ydatas)) in datas])
yrange = ymax - ymin

y_border_percent = 7.5
y_bottom = ymin - y_border_percent * yrange / 100
y_top = ymax + y_border_percent * yrange / 100

plt.ylim(y_bottom, y_top)


# plt.title("quebec-{}-{}-ECA value vs available budget.pdf".format(orig, median))
plt.ylabel('opt ratio', rotation=90, fontweight ='bold')
plt.xlabel("budget", fontweight ='bold')

for ((label,(linestyle,marker_size)),(xdatas,ydatas)) in datas:
    plt.plot(xdatas, ydatas, linestyle, markersize=marker_size, label=label)
    
legend = plt.legend(loc='lower right', shadow=True, fontsize='medium')


plt.savefig("data/worst_cases/decremental/dec worst case-opt ratio vs budget.pdf", dpi=500)