import matplotlib.pyplot as plt
import csv
import numpy as np

def readCSV(file_name, delimiter=' '):
    file = csv.DictReader(open(file_name), delimiter=delimiter)
    return list([row for row in file])

rows = readCSV('/home/plaiseek/Projects/landscape_opt_cpp/data/marseille_analysis_100/data.log')

thresold = 3000.0
alpha = 2000.0
length_gain = 30.0
area_gain = 0.5

absciss = np.array([float(row['budget']) for row in rows if float(row['thresold']) == thresold and float(row['alpha']) == alpha and float(row['length_gain']) == length_gain and float(row['area_gain']) == area_gain and row['solver'] == "bogo_draws=0_log=0"])
ordinate = [
    ("bogo" , np.array([float(row['total_eca']) for row in rows if float(row['thresold']) == thresold and float(row['alpha']) == alpha and float(row['length_gain']) == length_gain and float(row['area_gain']) == area_gain and row['solver'] == "bogo_draws=0_log=0"])),
    ("naive_dec" , np.array([float(row['total_eca']) for row in rows if float(row['thresold']) == thresold and float(row['alpha']) == alpha and float(row['length_gain']) == length_gain and float(row['area_gain']) == area_gain and row['solver'] == "naive_eca_dec_log=0_parallel=1"])),
    ("glutton_dec" , np.array([float(row['total_eca']) for row in rows if float(row['thresold']) == thresold and float(row['alpha']) == alpha and float(row['length_gain']) == length_gain and float(row['area_gain']) == area_gain and row['solver'] == "glutton_eca_dec_log=0_parallel=1"])),
    ("randomized_rounding" , np.array([float(row['total_eca']) for row in rows if float(row['thresold']) == thresold and float(row['alpha']) == alpha and float(row['length_gain']) == length_gain and float(row['area_gain']) == area_gain and row['solver'] == "randomized_rounding_draws=10000_log=0_parallel=1"])),
]


# print(absciss)
# print(ordinate)

plt.ylim(min(absciss) , max(absciss))

ymin = min([min(datas) for (_,datas) in ordinate])
ymax = max([max(datas) for (_,datas) in ordinate])
yrange = ymax - ymin

y_border_percent = 7.5
y_bottom = ymin - y_border_percent * yrange / 100
y_top = ymax + y_border_percent * yrange / 100

plt.xlim(y_bottom, y_top)


plt.title("ECA gain vs alpha with thresold={} and length_gain={}".format(thresold, length_gain))
plt.ylabel('ECA value')
plt.xlabel("alpha)")

for (label,ydatas) in ordinate:
    plt.plot(ydatas, absciss, label=label)
    
legend = plt.legend(loc='lower right', shadow=True, fontsize='medium')
    
plt.show()
