import csv

def readCSV(file_name, delimiter=' '):
    file = csv.DictReader(open(file_name), delimiter=delimiter)
    return list([row for row in file])

def createGraphFiles(output, name):
    patches_file_name = output + name + ".patches" 
    links_file_name = output + name + ".links"
    index = open(output + name + ".index", "w")
    index.write("patches_file,links_file\n{},{}".format(patches_file_name, links_file_name))
    index.close()
    return (open(patches_file_name, "w") , open(links_file_name, "w"))


sommets_rows = readCSV('data/quebec_leam_v3/raw/sommets_leam_v3.txt', ' ')
arretes_rows = readCSV('data/quebec_leam_v3/raw/aretes_leam_v3.txt', ' ')

output = 'data/quebec_leam_v3/'
name = 'quebec'
(patches_file, links_file) = createGraphFiles(output, name)

patches_file.write("id, weight, x, y\n")
for row in sommets_rows:
    patches_file.write("{},{},{},{}\n".format(int(row['count'])-1, row['area'], row['xcoord'], row['ycoord']))

links_file.write("source_id, target_id, probability\n")
for row in arretes_rows:
    links_file.write("{},{},{}\n".format(int(row['from'])-1, int(row['to'])-1, row['probdistGAP']))
    links_file.write("{},{},{}\n".format(int(row['to'])-1, int(row['from'])-1, row['probdistGAP']))
