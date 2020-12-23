import matplotlib.pyplot as plt
import csv
import numpy as np

def readCSV(file_name, delimiter=' '):
    file = csv.DictReader(open(file_name), delimiter=delimiter)
    return list([row for row in file])

rows = readCSV('/home/plaiseek/Projects/landscape_opt_cpp/output/data.log')
# rows = readCSV('/home/plaiseek/Projects/landscape_opt_cpp/output/quebec_weights.log')
# rows = readCSV('/home/plaiseek/Projects/landscape_opt_cpp/output/marseille_cecile.log')
# rows = readCSV('/home/plaiseek/Projects/landscape_opt_cpp/output/marseille_100_seed=456.log')

pow = 1
median = 900
noise = 0


def get_datas(name, value):
    return (name , (
        np.array([float(row['budget']) for row in rows if float(row['pow']) == pow and float(row['median']) == median and float(row['noise']) == noise and  row['solver'] == value]),
        np.array([float(row['total_eca']) for row in rows if float(row['pow']) == pow and float(row['median']) == median and float(row['noise']) == noise and  row['solver'] == value])))

datas = [
    # get_datas("random_solution" , "bogo_draws=0_seed=0"),
    get_datas("average_random_solution" , "bogo_seed=99"),
    # get_datas("naive_incremental", "naive_eca_inc_log=0_parallel=1"),
    # get_datas("naive_decremental", "naive_eca_dec_log=0_parallel=1"),
    get_datas("glutton_incremental", "glutton_eca_inc_log=0_parallel=1"),
    get_datas("glutton_decremental_incremental" , "glutton_eca_dec_log=0_parallel=1"),
    get_datas("preprocessed MIP" , "pl_eca_3_fortest=0_log=1_relaxed=0"),
]


fig_size = plt.rcParams["figure.figsize"]
fig_size[0] = 10
fig_size[1] = 6
plt.rcParams["figure.figsize"] = fig_size

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


plt.title("ECA value vs budget\n Quebec with median={}".format(median))
plt.ylabel('ECA value', rotation=0)
plt.xlabel("budget")

for (label,(xdatas,ydatas)) in datas:
    plt.plot(xdatas, ydatas, label=label)
    
legend = plt.legend(loc='lower right', shadow=True, fontsize='medium')


plt.savefig("ECA value vs budget-Quebec with median={}.pdf".format(median), dpi=500) 
plt.show()

