## Populate Agents ##

from AgentToolkit import *
from EnvironmentToolkit import *

##UTownTest Scenario

#f = open("test_agents.xml", "w")
#f.write("<Agents>\n")
#f.close()

#Your code here
#populate_rect(100, 200, 80, 50, 80, [[175.48280200000227, 224.30858400000216], [226.5928019999992, 114.6585840000007], [211.73280200000227, 76.83858400000099], [148.46280200000183, 75.93858399999954]], 3500, False, "test_agents.xml")
#populate_rect(155, 140, 40, 40, 40, [[175.48280200000227, 224.30858400000216], [226.5928019999992, 114.6585840000007], [211.73280200000227, 76.83858400000099], [148.46280200000183, 75.93858399999954]], 1500, False, "test_agents.xml")
#populate_rect(20, 240, 20, 20, 20, [[175.48280200000227, 224.30858400000216], [226.5928019999992, 114.6585840000007], [211.73280200000227, 76.83858400000099], [148.46280200000183, 75.93858399999954]], 1000, False, "test_agents.xml")
#populate_rect(30, 300, 20, 20, 20, [[175.48280200000227, 224.30858400000216], [226.5928019999992, 114.6585840000007], [211.73280200000227, 76.83858400000099], [148.46280200000183, 75.93858399999954]], 1000, False, "test_agents.xml")

#f = open("test_agents.xml", "a")
#f.write("</Agents>")
#f.close()

#create_xml_from_geojson(r'C:\Users\imjon\Documents\School\FYP\Environment\UTown\UTown_Green.geojson', r'testObstacles.xml', 0)
#create_png_from_geojson(r'C:\Users\imjon\Documents\School\FYP\Environment\UTown\UTown_Green.geojson', r'testObstacles.png', 0)
#get_endpoints_coordinates([[31985.54,21335.65], [31875.89,21386.76], [31838.07,21371.90], [31837.17,21308.63]], 21160.167198, 31761.231416, 0)
#[175.48280200000227, 224.30858400000216, 226.5928019999992, 114.6585840000007, 211.73280200000227, 76.83858400000099, 148.46280200000183, 75.93858399999954]


##UTownConcert Scenario

#create_xml_from_geojson(r"C:\Users\imjon\Documents\School\FYP\QGIS\utown\utown_obstacles.geojson", r"utownObstacles.xml", 50)
#create_png_from_geojson(r"C:\Users\imjon\Documents\School\FYP\QGIS\utown\utown_obstacles.geojson", r"utownObstacles.png", 50)
get_endpoints_coordinates([[31750.26,21382.82], [31735.25,21406.12], [31825.10,21504.62]], 21090.672486, 31710.897441)
#create_mask_png_from_geojson(r"C:\Users\imjon\Documents\School\FYP\QGIS\utown\utown_obstacles.geojson", 50,
                             #r"C:\Users\imjon\Documents\School\FYP\QGIS\utown_dense\utown_dense.geojson",
                             #r"UTownConcert_dense.png")
#create_mask_png_from_geojson(r"C:\Users\imjon\Documents\School\FYP\QGIS\utown\utown_obstacles.geojson", 50,
                             #r"C:\Users\imjon\Documents\School\FYP\QGIS\utown_exclude\utown_exclude.geojson",
                             #r"UTownConcert_exclude.png")


#filename = "UTownConcert_RandomizedTestPopulation30000.xml"
#num_agents = 30000
#goals = [[292.1475140000002, 39.362558999997422], [315.4475139999995, 24.35255899999902], [402.5875139999989, 115.88255899999785]]
#skew = 5

#open_agent_xml(filename)
#populate_world_from_png(r"C:\Users\imjon\Documents\School\FYP\CrowdSim\visibility-based-marching\images\custom\UTownConcert.png",
                        #r"C:\Users\imjon\Documents\School\FYP\CrowdSim\visibility-based-marching\images\custom\UTownConcert_dense.png",
                        #r"C:\Users\imjon\Documents\School\FYP\CrowdSim\visibility-based-marching\images\custom\UTownConcert_exclude.png",
                        #filename, num_agents, goals, skew)
#close_agent_xml(filename)

#Legacy Population Code
'''
filename = "UTownConcert70000.xml"
f = open(filename, "w")
f.write("<Agents>\n")
f.close()

#10000
populate_rect(21312, 31965, 45, 45, 60, [[0,0]], 3500, True, [21091, 31711], -41, filename)

populate_rect(21270, 31916, 20, 10, 15, [[0,0]], 500, True, [21091, 31711], 48, filename)
populate_rect(21255, 31933, 55, 20, 30, [[0,0]], 1750, True, [21091, 31711], 48, filename)
populate_rect(21234, 31937, 50, 18, 20, [[0,0]], 1000, True, [21091, 31711], 48, filename)

populate_rect(21295, 31894, 20, 10, 15, [[0,0]], 500, True, [21091, 31711], 48, filename)
populate_rect(21304, 31887, 55, 20, 30, [[0,0]], 1750, True, [21091, 31711], 49, filename)
populate_rect(21314, 31867, 45, 16, 20, [[0,0]], 1000, True, [21091, 31711], 49, filename)

#20000
populate_rect(21410, 32040, 75, 75, 100, [[0,0]], 10000, True, [21091, 31711], 0, filename)

#30000
populate_rect(21490, 32040, 75, 75, 100, [[0,0]], 10000, True, [21091, 31711], 0, filename)

#40000
populate_rect(21450, 31960, 75, 75, 100, [[0,0]], 10000, True, [21091, 31711], 0, filename)

#50000
populate_rect(21101, 31881, 75, 75, 100, [[0,0]], 10000, True, [21091, 31711], 0, filename)

#60000
populate_rect(21101, 31791, 75, 75, 100, [[0,0]], 10000, True, [21091, 31711], 0, filename)

#70000
populate_rect(21181, 31791, 75, 75, 100, [[0,0]], 10000, True, [21091, 31711], 0, filename)

f = open(filename, "a")
f.write("</Agents>")
f.close()
'''
