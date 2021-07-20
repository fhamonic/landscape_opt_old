import matplotlib.pyplot as plt
import csv
import numpy as np

def readCSV(file_name, delimiter=' '):
    file = csv.DictReader(open(file_name), delimiter=delimiter)
    return list([row for row in file])

rows = readCSV('/home/plaiseek/Projects/landscape_opt_cpp/data/scripts/marseille2_thresold_analysis.csv')

alpha = 2000
length_gain = 30

absciss = np.array([int(row['thresold']) for row in rows if int(row['alpha']) == alpha and int(row['length_gain']) == length_gain])
ordinate = [
    ("total_eca_restored", np.array([float(row['total_eca_restored']) for row in rows if int(row['alpha']) == alpha and int(row['length_gain']) == length_gain])),
    ("total_eca", np.array([float(row['total_eca']) for row in rows if int(row['alpha']) == alpha and int(row['length_gain']) == length_gain])),
    ("parcs_eca", np.array([float(row['parcs_eca']) for row in rows if int(row['alpha']) == alpha and int(row['length_gain']) == length_gain])),
    ("between_massifs_parcs_eca", np.array([float(row['between_massifs_parcs_eca']) for row in rows if int(row['alpha']) == alpha and int(row['length_gain']) == length_gain])),
    ("between_massifs_parcs_eca_restored", np.array([float(row['between_massifs_parcs_eca_restored']) for row in rows if int(row['alpha']) == alpha and int(row['length_gain']) == length_gain])),
    ("massifs_eca", np.array([float(row['massifs_eca']) for row in rows if int(row['alpha']) == alpha and int(row['length_gain']) == length_gain])),
    ("massifs_eca_restored", np.array([float(row['massifs_eca_restored']) for row in rows if int(row['alpha']) == alpha and int(row['length_gain']) == length_gain]))
]


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


plt.title("ECA value vs arcs thresold with alpha={}".format(alpha))
plt.ylabel('ECA value')
plt.xlabel("arcs thresold (m)")

for (label,ydatas) in ordinate:
    plt.plot(absciss, ydatas, label=label)
    
legend = plt.legend(loc='lower center', shadow=True, fontsize='medium')

plt.show()
