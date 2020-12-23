import math

# a = 1
# b = 1
# gamma = 120


a = math.sqrt(2)
b = 1
gamma = 135

c = math.sqrt(pow(a,2) + pow(b,2) - 2*a*b * math.cos(math.radians(gamma)))
print(c)
print("ratio : {}".format((a+b) / c))