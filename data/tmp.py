
import math

center_x = 0
center_y = 0
radius = 3
nb_vertices = 5
angle = 2 * math.pi / nb_vertices

"""
print("id,weight,x,y")
for i in range(nb_vertices):
    x = center_x + radius * math.sin(i * angle)
    y = center_y + radius * math.cos(i * angle)
    print("{},{},{},{}".format(i, 1, x, y))
""
print("id,source_id,target_id,length")
cpt = 0
for i in range(nb_vertices):
    for j in range(i+1,nb_vertices):
        print("{},{},{},{}".format(cpt, i, j, 1))
        cpt += 1
"""
print("edge_id,cost,degraded_length")
degraded_length = 1000
cpt = 0
for i in range(nb_vertices):
    for j in range(i+1,nb_vertices):
        print("{},{},{}".format(cpt, 1, degraded_length))
        cpt += 1
#"""