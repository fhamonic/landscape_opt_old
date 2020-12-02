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

# rows = readCSV('/home/plaiseek/Projects/landscape_opt_cpp/output/marseille_cecile.log')
# rows = readCSV('/home/plaiseek/Projects/landscape_opt_cpp/output/quebec_weights.log')
rows = readCSV('/home/plaiseek/Projects/landscape_opt_cpp/output/data.log')

pow = 1
median = 900
noise = 0
orig = "(304364.000000,5088601.000000)"
# orig = "(273689.000000,5051344.000000)"
# orig = "(332538.000000,5015636.000000)"


ref_datas = np.array([float(row['total_eca']) for row in rows if float(row['pow']) == pow and float(row['median']) == median and float(row['noise']) == noise and row['orig'] == orig and row['solver'] == "bogo_seed=99"])
# ref_datas = np.array([1725121950 for row in rows if float(row['pow']) == pow and float(row['median']) == median and float(row['noise']) == noise and row['orig'] == orig and row['solver'] == "bogo_seed=99"])
mip_gain_datas = substract(np.array([float(row['total_eca']) for row in rows if float(row['pow']) == pow and float(row['median']) == median and float(row['noise']) == noise and row['orig'] == orig and row['solver'] == "pl_eca_3_fortest=0_log=1_relaxed=0"]), ref_datas)

def get_datas(name, value):
    return (name , (
        np.array([float(row['budget_percent']) for row in rows if float(row['pow']) == pow and float(row['median']) == median and float(row['noise']) == noise and row['orig'] == orig and  row['solver'] == value]),
        divide(substract(np.array([float(row['total_eca']) for row in rows if float(row['pow']) == pow and float(row['median']) == median and float(row['noise']) == noise and row['orig'] == orig and row['solver'] == value]), ref_datas), mip_gain_datas)))

datas = [
    # get_datas("average_random_solution" , "bogo_seed=99"),
    # get_datas("naive_incremental", "naive_eca_inc_log=0_parallel=1"),
    # get_datas("naive_decremental", "naive_eca_dec_log=0_parallel=1"),
    get_datas("glutton_incremental", "glutton_eca_inc_log=0_parallel=1"),
    get_datas("glutton_decremental_incremental" , "glutton_eca_dec_log=0_parallel=1"),
    get_datas("preprocessed MIP" , "pl_eca_3_fortest=0_log=1_relaxed=0")
]


fig_size = plt.rcParams["figure.figsize"]
fig_size[0] = 12
fig_size[1] = 6
plt.rcParams["figure.figsize"] = fig_size

plt.rcParams.update({'font.size': 12})

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

plt.ylim(y_bottom-0.015, y_top)


# plt.title("quebec-{}-{}-ECA value vs available budget.pdf".format(orig, median))
plt.ylabel('ratio to MIP solution', rotation=90)
plt.xlabel("budget in %")

for (label,(xdatas,ydatas)) in datas:
    plt.plot(xdatas, ydatas, '.-', label=label)
    
legend = plt.legend(loc='lower right', shadow=True, fontsize='medium')


plt.savefig("quebec-{}-{}-ratio to MIP solution vs available budget.pdf".format(orig, median), dpi=500) 
plt.show()

