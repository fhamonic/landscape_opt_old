import matplotlib.pyplot as plt
import csv
import numpy as np

def readCSV(file_name, delimiter=' '):
    file = csv.DictReader(open(file_name), delimiter=delimiter)
    return list([row for row in file])

#rows = readCSV('/home/plaiseek/Projects/landscape_opt_cpp/data/scripts/quebec_noise_exp_seed=456.log')
rows = readCSV('/home/plaiseek/Projects/landscape_opt_cpp/output/data.log')

pow = 1
median = 700
noise = 0


absciss = np.array([float(row['budget']) for row in rows if float(row['pow']) == pow and float(row['median']) == median and float(row['noise']) == noise and  row['solver'] == 'bogo_draws=0_seed=0'])

def datas(name, value):
    return (name , np.array([float(row['total_eca']) for row in rows if float(row['pow']) == pow and float(row['median']) == median and float(row['noise']) == noise and  row['solver'] == value]))

ordinate = [
    datas("random_solution" , "bogo_draws=0_seed=0"),
    datas("naive_incremental", "naive_eca_inc_log=0_parallel=1"),
    datas("naive_decremental", "naive_eca_dec_log=0_parallel=1"),
    datas("glutton_incremental", "glutton_eca_inc_log=0_parallel=1"),
    datas("glutton_decremental" , "glutton_eca_dec_log=0_parallel=1"),
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


plt.title("ECA vs budget with median distance={}m, noise={}".format(median, noise))
plt.ylabel('ECA')
plt.xlabel("budget")

for (label,ydatas) in ordinate:
    plt.plot(absciss, ydatas, label=label)
    
legend = plt.legend(loc='upper left', shadow=True, fontsize='medium')



plt.savefig("ECA vs budget with median={}_noise={}.pdf".format(median, noise), dpi=500) 
plt.show()

