import json

#https://stackoverflow.com/a/4682656
def reproject(latitude, longitude):
    """Returns the x & y coordinates in meters using a sinusoidal projection"""
    from math import pi, cos, radians
    earth_radius = 6371009 # in meters
    lat_dist = pi * earth_radius / 180.0
    y = [lat * lat_dist for lat in latitude]
    x = [long * lat_dist * cos(radians(lat)) for lat, long in zip(latitude, longitude)]
    return x, y

def area_of_polygon(x, y):
    """Calculates the area of an arbitrary polygon given its verticies"""
    area = 0.0
    for i in range(-1, len(x)-1):
        area += x[i] * (y[i+1] - y[i-1])
    return abs(area) / 2.0

def get_projected_area(geo_points):
    (latitude, longitude) = zip(*geo_points)
    (x, y) = reproject(latitude, longitude)
    return area_of_polygon(x, y)



with open('/home/plaiseek/Projects/landscape_opt_cpp/data/OSM/marseille.json') as json_file:
    data = json.load(json_file)
    for p in data['features']:
        if p['geometry']['type'] != 'Polygon':
            continue
        
        if "name" in p['properties']:
            print('name: ' + p['properties']['name'])
        else:
            if "leisure" in p['properties']:
                print('leisure: ' + p['properties']['leisure'])
            elif "natural" in p['properties']:
                print('natural: ' + p['properties']['natural'])

            print('Id: ' + p['properties']['@id'])

        print('Aire(m2): ' + str(get_projected_area(p['geometry']['coordinates'][0])))