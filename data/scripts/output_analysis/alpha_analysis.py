import matplotlib.pyplot as plt
import csv
import numpy as np

def readCSV(file_name, delimiter=' '):
    file = csv.DictReader(open(file_name), delimiter=delimiter)
    return list([row for row in file])

rows = readCSV('/home/plaiseek/Projects/landscape_opt_cpp/data/scripts/marseille2_alpha_analysis.csv')

thresold = 3000
length_gain = 60

absciss = np.array([int(row['alpha']) for row in rows if int(row['thresold']) == thresold and int(row['length_gain']) == length_gain])
ordinate = [
    ("total_eca" , np.array([float(row['total_eca']) for row in rows if int(row['thresold']) == thresold and int(row['length_gain']) == length_gain])),
    ("total_eca_restored" , np.array([float(row['total_eca_restored']) for row in rows if int(row['thresold']) == thresold and int(row['length_gain']) == length_gain]))
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


plt.title("ECA gain vs alpha with thresold={} and length_gain={}".format(thresold, length_gain))
plt.ylabel('ECA value')
plt.xlabel("alpha)")

for (label,ydatas) in ordinate:
    plt.plot(absciss, ydatas, label=label)
    
legend = plt.legend(loc='lower center', shadow=True, fontsize='medium')
    
plt.show()
