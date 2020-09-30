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


split_node = dict()
last_node_id = len(sommets_rows)
last_arc_id = 2*len(arretes_rows)
problem = open(output + name + ".problem", "w")

patches_file.write("id, weight, x, y\n")
for row in sommets_rows:
    id = int(row['count'])-1
    if float(row['menace']) > 0:
        patches_file.write("{}, {}, {}, {}\n".format(id, row['count2050'], row['xcoord'], row['ycoord']))
        if float(row['menace']) == 100:
            split_node[id] = (last_node_id, last_arc_id)
            last_node_id = last_node_id + 1
            last_arc_id = last_arc_id + 1
        else:
            quality_gain = float(row['area']) - float(row['count2050'])
            problem.write("{} 1\n\tn {} {}\n".format(row['area'], id, quality_gain))
    else:
        patches_file.write("{}, {}, {}, {}\n".format(id, row['area'], row['xcoord'], row['ycoord']))

links_file.write("source_id, target_id, probability\n")
for row in arretes_rows:
    id_from = int(row['from'])-1
    id_to = int(row['to'])-1

    if id_to in split_node: links_file.write("{}, {}, {}\n".format(id_from, split_node[id_to][0], row['probdistGAP']))
    else: links_file.write("{}, {}, {}\n".format(id_from, id_to, row['probdistGAP']))
    
    if id_from in split_node: links_file.write("{}, {}, {}\n".format(id_to, split_node[id_from][0], row['probdistGAP']))
    else: links_file.write("{}, {}, {}\n".format(id_to, id_from, row['probdistGAP']))

for row in sommets_rows:
    id = int(row['count'])-1
    if id not in split_node: continue
    (in_id, arc_id) = split_node[id]
        
    patches_file.write("{}, 0, {}, {}\n".format(in_id, row['xcoord'], row['ycoord']))
    links_file.write("{}, {}, {}\n".format(in_id, id, 0))

    quality_gain = float(row['area']) - float(row['count2050'])
    problem.write("{} 2\n\tn {} {}\n\ta {} {} 1\n".format(row['area'], id, quality_gain, in_id, id))

