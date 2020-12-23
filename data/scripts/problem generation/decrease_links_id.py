import csv

def readCSV(file_name, delimiter=' '):
    file = csv.DictReader(open(file_name), delimiter=delimiter)
    return list([row for row in file])

rows = readCSV('/tmp/mozilla_plaiseek0/edges_marseille.txt', ' ')

print("source_id,target_id,length")
for row in rows:
    print("{},{},{}".format(int(row['source_id'])-1, int(row['target_id'])-1, row['length']))
    print("{},{},{}".format(int(row['target_id'])-1, int(row['source_id'])-1, row['length']))
