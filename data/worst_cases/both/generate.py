import math

def regular_vertices(nb_vertices, radius):
    angle = 2*math.pi/nb_vertices
    return [(radius*math.cos(i*angle), radius*math.sin(i*angle)) for i in range(nb_vertices)]

budget=5

relative_path = "data/worst_cases/both/"
name = "both_worst_case"

index = open(relative_path + name + ".index", "w")
patches = open(relative_path+ name + ".patches", "w")
links = open(relative_path + name + ".links", "w")
problem = open(relative_path + name + ".problem", "w")

last = -1
count = 0
def addPath(weight,x,y):
    global last
    global count
    last = count
    count += 1
    patches.write("{},{},{},{}\n".format(last,weight,x,y))

def addLink(a,b):
    links.write("{},{},0\n".format(a,b))
    links.write("{},{},0\n".format(b,a))
    problem.write("1 2\n")
    problem.write("\ta {} {} 1\n".format(a,b))
    problem.write("\ta {} {} 1\n".format(b,a))

index.write("patches_file,links_file\n"+relative_path+ name + ".patches,"+ relative_path + name + ".links\n")

patches.write("id,weight,x,y\n")
links.write("from,to,probability\n")


epsilon = 0.0001
one_plus_epsilon = 1.0001

# #for display
# epsilon = 0.2
# one_plus_epsilon = 1.4


addPath(1, 0, 0)
for (i,(x,y)) in enumerate(regular_vertices(budget+1, 2*budget)):
    if i == 0: 
        addPath(one_plus_epsilon, x, y)
    else:
        addPath(1, x/budget, y/budget)

prec = 0
for i in range(2*budget-1):
    addPath(epsilon, i+1, 0)
    addLink(prec,last)
    prec=last
addLink(prec,1)

for (i,(x,y)) in enumerate(regular_vertices(budget+1, budget)):
    if i != 0: 
        addPath(0, x/budget, y/budget)
        addLink(0,last)
        addLink(last, i+1)

index.close()
patches.close()
links.close()
problem.close()
