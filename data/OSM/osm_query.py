# coding: utf8
from enum import Enum
import overpy
import json

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
        'scrub' : WayType.BROUSSAILLE,
        'grassland' : WayType, 
        'heath' : 'lande',
        'meadow' : 'pre'
    }
}



green_tags_table = {
    'leisure': {
        'garden' : WayType.JARDIN,
        'park' : WayType.PARC
    },
    'landuse': {
        'grass' : WayType.PELOUSE,
        'forest' : WayType.FORET,
        'trees' : WayType.FORET,
        'allotments' : WayType.JARDIN
    },
    'natural': {
        'tree_row' : WayType.ALIGNEMENT_ARBRES,
        'wood' : WayType.FORET,
        'scrub' : WayType.BROUSSAILLE,
        'grassland' : WayType, 
        'heath' : 'lande',
        'meadow' : 'pre'
    }
}


city = "Marseille"

query = "[out:json];\n"
query += "area[name = '{}']->.a;\n".format(city)
query += "(\n"
for tag, values in green_tags_table.items():
    for value in values:
        #query += "\tnwr['{}'='{}']({{{{bbox}}}});\n".format(tag, value)
        query += "\tnwr(area.a)['{}'='{}'];\n".format(tag, value)
query += ");\n"
query += "(._;>;);\n"
query += "out body geom;"


print(query)

api = overpy.Overpass()
result = api.query(query)

# print(result)

for way in result.ways:
    if "name" in way.tags:
        print(way.tags["name"].encode('utf-8'))
    else:
        continue
    if "geometry" in way.attributes:
        print(way.attributes["geometry"])
    # print(way.attributes)