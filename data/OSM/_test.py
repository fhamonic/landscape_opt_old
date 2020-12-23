from shapely.geometry import Polygon

b = Polygon([[1, 1], [1, 3], [3, 3], [3, 1]])

b.contains(0)