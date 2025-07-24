## Downtown Core ##

from EnvironmentToolkit import *
from AgentToolkit import *

#create_xml_from_geojson(r"C:\Users\imjon\Documents\School\FYP\QGIS\downtown\downtown_obstacles(2).geojson", r"Downtown.xml", 0)
#create_png_from_geojson(r"C:\Users\imjon\Documents\School\FYP\QGIS\downtown\downtown_obstacles(2).geojson", r"Downtown.png", 0)
#create_mask_png_from_geojson(r"C:\Users\imjon\Documents\School\FYP\QGIS\downtown\downtown_obstacles(2).geojson", 0,
                             #r"C:\Users\imjon\Documents\School\FYP\QGIS\downtown_dense\downtown_dense.geojson",
                             #r"Downtown_dense.png")
#create_mask_png_from_geojson(r"C:\Users\imjon\Documents\School\FYP\QGIS\downtown\downtown_obstacles(2).geojson", 0,
                             #r"C:\Users\imjon\Documents\School\FYP\QGIS\downtown_exclude\downtown_exclude.geojson",
                             #r"Downtown_exclude.png")

#write agent.xml
filename = "Downtown_500000.xml"
num_agents = 500000
goals = get_endpoints_coordinates([[28707.23,30417.62], [28905.94,29947.29], [29094.57,30162.91], [29407.30,29697.54], [29716.30,30047.19], [30606.40,30055.10], [30709.40,30396.06], [30714.36,30994.53], [29308.02,30844.06]], 29646.187489, 28684.779641)
skew = 5

open_agent_xml(filename)
populate_world_from_png(r"C:\Users\imjon\Documents\School\FYP\CrowdSim\visibility-based-marching\images\custom\Downtown.png",
                        r"C:\Users\imjon\Documents\School\FYP\CrowdSim\visibility-based-marching\images\custom\Downtown_dense.png",
                        r"C:\Users\imjon\Documents\School\FYP\CrowdSim\visibility-based-marching\images\custom\Downtown_exclude.png",
                        filename, num_agents, goals, skew)
close_agent_xml(filename)
