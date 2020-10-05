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



x_min = min([float(row['xcoord']) for row in sommets_rows])
x_max = max([float(row['xcoord']) for row in sommets_rows])
y_min = min([float(row['ycoord']) for row in sommets_rows])
y_max = max([float(row['ycoord']) for row in sommets_rows])

xl_bound = x_min
xu_bound = x_min + (x_max-x_min) / 8
yl_bound = y_min
yu_bound = y_min + (y_max-y_min) / 8

def predicate(row):
    return xl_bound <= float(row['xcoord']) <= xu_bound and yl_bound <= float(row['ycoord']) <= yu_bound


nb_nodes = 0
ids = []
kept_sommets_rows = []
for row in sommets_rows:
    if not predicate(row):
        ids.append(-1)
        continue
    ids.append(nb_nodes)
    nb_nodes = nb_nodes + 1
    kept_sommets_rows.append(row)


split_node = dict()
nb_new_nodes = 0
nb_new_arcs = 0
problem = open(output + name + ".problem", "w")


patches_file.write("id, weight, x, y\n")
for row in kept_sommets_rows:
    id = ids[int(row['count'])-1]
    if float(row['menace']) > 0:
        patches_file.write("{}, {}, {}, {}\n".format(id, row['count2050'], row['xcoord'], row['ycoord']))
        if float(row['menace']) == 100:
            split_node[id] = (nb_new_nodes, nb_new_arcs)
            nb_new_nodes = nb_new_nodes + 1
            nb_new_arcs = nb_new_arcs + 1
        else:
            quality_gain = float(row['area']) - float(row['count2050'])
            problem.write("{} 1\n\tn {} {}\n".format(row['area'], id, quality_gain))
    else:
        patches_file.write("{}, {}, {}, {}\n".format(id, row['area'], row['xcoord'], row['ycoord']))

links_file.write("source_id, target_id, probability\n")
for row in arretes_rows:
    id_from = ids[int(row['from'])-1]
    id_to = ids[int(row['to'])-1]

    if id_from == -1 or id_to == -1: continue

    if id_to in split_node: links_file.write("{}, {}, {}\n".format(id_from, nb_nodes + split_node[id_to][0], row['probdistGAP']))
    else: links_file.write("{}, {}, {}\n".format(id_from, id_to, row['probdistGAP']))
    
    if id_from in split_node: links_file.write("{}, {}, {}\n".format(id_to, nb_nodes + split_node[id_from][0], row['probdistGAP']))
    else: links_file.write("{}, {}, {}\n".format(id_to, id_from, row['probdistGAP']))

for row in kept_sommets_rows:
    id = ids[int(row['count'])-1]
    if id not in split_node: continue
    (in_id, arc_id) = split_node[id]

    in_id = nb_nodes + in_id
        
    patches_file.write("{}, 0, {}, {}\n".format(in_id, float(row['xcoord'])+0.01, row['ycoord']))
    links_file.write("{}, {}, 0\n".format(in_id, id))

    quality_gain = float(row['area']) - float(row['count2050'])
    problem.write("{} 2\n\tn {} {}\n\ta {} {} 1\n".format(row['area'], id, quality_gain, in_id, id))

