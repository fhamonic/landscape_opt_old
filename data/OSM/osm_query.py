
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
        'scrub' : WayType.BROUSSAILLE
    }
}


city = "Marseille"

query = "[out:json];\n"
query += "area[name = '{}'];\n".format(city)
query += "(\n"
for tag, values in way_tags_table.items():
    for value in values:
        query += "\tway['{}'='{}'](area);\n".format(tag, value)
query += ");\n"
query += "out;"


print(query)
"""
api = overpy.Overpass()
result = api.query(query)

print(result)"""
