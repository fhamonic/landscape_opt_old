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



from enum import Enum

class WayType(Enum):
    ERROR = -1
    NONE = 0
    TERRAIN_SPORT = 1
    CIMETIERE = 2
    JARDIN = 3
    PELOUSE = 4
    PARC = 5
    PISCINE = 6
    BASSIN = 7
    BROUSSAILLE = 8
    FORET = 9
    ALIGNEMENT_ARBRES = 10
    ROUTE_PRINCIPALE = 11
    ROUTE_SECONDAIRE = 12
    ROUTE_TERTIAIRE = 12
    AUTOROUTE = 13
    ROUTE_AUTRE = 14

class NodeType(Enum):
    ERROR = -1
    NONE = 0
    ARBRE = 1



way_tags_table = {
    'leisure': {
        'pitch' : WayType.TERRAIN_SPORT,
        'garden' : WayType.JARDIN,
        'park' : WayType.PARC
    },
    'landuse': {
        'cemetery' : WayType.CIMETIERE,
        'grass' : WayType.PELOUSE,
        'swimming_pool' : WayType.PISCINE,
        'basin' : WayType.BASSIN,
        'forest' : WayType.FORET,
        'trees' : WayType.FORET
    },
    'highway': {
        'motorway' : WayType.AUTOROUTE,
        'motorway_link' : WayType.AUTOROUTE,
        'trunk' : WayType.AUTOROUTE,
        'trunk_link' : WayType.AUTOROUTE,
        'primary' : WayType.ROUTE_PRINCIPALE,
        'primary_link' : WayType.ROUTE_PRINCIPALE,
        'secondary' : WayType.ROUTE_SECONDAIRE,
        'secondary_link' : WayType.ROUTE_SECONDAIRE,
        'tertiary' : WayType.ROUTE_TERTIAIRE,
        'tertiary_link' : WayType.ROUTE_TERTIAIRE
    },
    'natural': {
        'tree_row' : WayType.ALIGNEMENT_ARBRES,
        'wood' : WayType.FORET,
        'scrub' : WayType.BROUSSAILLE
    }
}
def identify_way(feature):
    if 'properties' not in feature:
        return WayType.ERROR
    p = feature['properties']
    for tag, correspondances in way_tags_table:
        if tag in p:
            if p[tag] in correspondances:
                return correspondances[p[tag]]
    return WayType.NONE



way_tags_table = {
    'natural': {
        'tree' : NodeType.ARBRE
    }
}
def identify_node(feature):
    if 'properties' not in feature:
        return WayType.ERROR
    p = feature['properties']
    for tag, correspondances in way_tags_table:
        if tag in p:
            if p[tag] in correspondances:
                return correspondances[p[tag]]
    return NodeType.NONE