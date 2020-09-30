import matplotlib.pyplot as plt
import csv
import numpy as np
import math

def readCSV(file_name, delimiter=' '):
    file = csv.DictReader(open(file_name), delimiter=delimiter)
    return list([row for row in file])

rows = readCSV('/home/plaiseek/Projects/landscape_opt_cpp/data/scripts/marseille2_alpha_analysis.csv')

thresold = 3000
alpha = 2000

absciss = np.array([int(row['length_gain']) for row in rows if int(row['thresold']) == thresold and int(row['alpha']) == alpha])
ordinate = [
    ("total_eca" , np.array([math.sqrt(float(row['total_eca'])) for row in rows if int(row['thresold']) == thresold and int(row['alpha']) == alpha])),
    ("total_eca_restored" , np.array([math.sqrt(float(row['total_eca_restored'])) for row in rows if int(row['thresold']) == thresold and int(row['alpha']) == alpha]))
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


plt.title("ECA gain vs length_gain with thresold={} and alpha={}".format(thresold, alpha))
plt.ylabel('ECA value')
plt.xlabel("alpha)")

for (label,ydatas) in ordinate:
    plt.plot(absciss, ydatas, label=label)
    
legend = plt.legend(loc='lower center', shadow=True, fontsize='medium')
    
plt.show()
