import matplotlib.pyplot as plt
import csv
import numpy as np

def readCSV(file_name, delimiter=' '):
    file = csv.DictReader(open(file_name), delimiter=delimiter)
    return list([row for row in file])

def add(a, b):
    return [x+y for x,y in zip(a,b)]
def substract(a, b):
    return [x-y for x,y in zip(a,b)]
def divide(a, b):
    return [x/y if y > 0 else 1 for x,y in zip(a,b)]
def normalize(a, b):
    return [x/b for x in a]

# rows = readCSV('/home/plaiseek/Projects/landscape_opt_cpp/output/marseille_cecile.log')
# rows = readCSV('/home/plaiseek/Projects/landscape_opt_cpp/output/quebec_weights.log')
rows = readCSV('/home/plaiseek/Projects/landscape_opt_cpp/output/data.log')

pow = 1
median = 900


origs = [row['orig'] for row in rows if float(row['pow']) == pow and float(row['median']) == median and row['solver'] == "bogo_seed=99" and float(row['budget']) == 0]

def data(orig, solver):
    return np.array([float(row['total_eca']) for row in rows if float(row['pow']) == pow and float(row['median']) == median and row['orig'] == orig and row['solver'] == solver])
def datas(solver):
    global origs
    sum = [0 for _ in data(origs[0], "bogo_seed=99")]
    r = [0 for _ in data(origs[0], "bogo_seed=99")]
    for orig in origs:
        c = data(orig, solver)
        r = data(orig, "bogo_seed=99")
        m = data(orig, "pl_eca_3_fortest=0_log=1_relaxed=0_timeout=3600")
        sum = add(sum, divide(substract(c, r), substract(m, r)))
    return normalize(sum, len(origs))

def get_datas(name, linestyle, value):
    global origs
    return ((name, linestyle), (
        np.array([float(row['budget_percent']) for row in rows if float(row['pow']) == pow and float(row['median']) == median and row['orig'] == origs[0] and  row['solver'] == value]),
        datas(value)))

datas = [
    get_datas("incremental glutton", "^-", "glutton_eca_inc_log=0_parallel=1"),
    get_datas("", " ", "pl_eca_3_fortest=0_log=1_relaxed=0_timeout=3600"),
    get_datas("decremental glutton", "v-", "glutton_eca_dec_log=0_parallel=1"),
    get_datas("MIP", "s-", "pl_eca_3_fortest=0_log=1_relaxed=0_timeout=3600"),
    # get_datas("randomized rounding", "*-", "randomized_rounding_draws=1000_log=0_parallel=1")
]


fig_size = plt.rcParams["figure.figsize"]
fig_size[0] = 10
fig_size[1] = 5
plt.rcParams["figure.figsize"] = fig_size

plt.subplots_adjust(left=0.095, right=0.95, top=0.92, bottom=0.13)

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
plt.ylabel('average optimum ratio', rotation=90, fontweight ='bold')
plt.xlabel("budget in %", fontweight ='bold')

for ((label,linestyle),(xdatas,ydatas)) in datas:
    plt.plot(xdatas, ydatas, linestyle, label=label)
    
legend = plt.legend(loc='lower right', shadow=True, fontsize='medium')


plt.savefig("output/quebec-avergare opt ratio vs budget.pdf", dpi=500)
plt.show()

