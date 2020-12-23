import matplotlib.pyplot as plt
import csv
import numpy as np

def readCSV(file_name, delimiter=' '):
    file = csv.DictReader(open(file_name), delimiter=delimiter)
    return list([row for row in file])

# rows = readCSV('/home/plaiseek/Projects/landscape_opt_cpp/data/scripts/quebec_noise_exp_seed=456.log')
#rows = readCSV('/home/plaiseek/Projects/landscape_opt_cpp/output/marseille_100_seed=456.log')

rows = readCSV('/home/plaiseek/Projects/landscape_opt_cpp/output/data.log')

pow = 1
median = 900
noise = 0


absciss = np.array([float(row['nb_friches']) for row in rows if float(row['pow']) == pow and float(row['median']) == median and  row['solver'] == 'pl_eca_3_fortest=0_log=1_relaxed=0'])

def datas(name, value):
    return (name , np.array([float(row['time'])/1000 for row in rows if float(row['pow']) == pow and float(row['median']) == median and row['solver'] == value]))

ordinate = [
    # datas("average_random_solution" , "bogo_seed=99"),
    # datas("naive_incremental", "naive_eca_inc_log=0_parallel=1"),
    # datas("naive_decremental", "naive_eca_dec_log=0_parallel=1"),
    datas("glutton_incremental", "glutton_eca_inc_log=0_parallel=1"),
    datas("glutton_decremental" , "glutton_eca_dec_log=0_parallel=1"),
    # datas("MIP" , "pl_eca_2_fortest=0_log=1_relaxed=0"),
    datas("preprocessed MIP" , "pl_eca_3_fortest=0_log=1_relaxed=0"),
]


fig_size = plt.rcParams["figure.figsize"]
fig_size[0] = 10
fig_size[1] = 6
plt.rcParams["figure.figsize"] = fig_size

# print(absciss)
# print(ordinate)

plt.xlim(min(absciss) , max(absciss))

ymin = min([min(datas) for (_,datas) in ordinate])
ymax = max([max(datas) for (_,datas) in ordinate])
yrange = ymax - ymin

y_border_percent = 7.5
y_bottom = ymin - y_border_percent * yrange / 100
y_top = ymax + y_border_percent * yrange / 100

plt.ylim(y_bottom, y_top)


plt.title("average time in seconds vs number of wastelands")
plt.ylabel('time (s)')
plt.xlabel("#wasteland")

for (label,ydatas) in ordinate:
    plt.plot(absciss, ydatas, label=label)
    
legend = plt.legend(loc='upper left', shadow=True, fontsize='medium')



plt.savefig("time vs number of wasteland.pdf".format(median, noise), dpi=500) 
plt.show()

