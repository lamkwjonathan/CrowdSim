# Crowd Simulation for Digital Twin Applications

This is a final year project focusing on the development of crowd simulation functionality for large-scale crowds. It heavily references the methods in [SPH crowds: Agent-based crowd simulation up to
extreme densities using fluid dynamics](https://inria.hal.science/hal-03270915/file/CAG2021-SPHCrowds%20-%20Author%20version.pdf) and [Exact Wavefront Propagation for Globally Optimal One-to-All Path Planning on 2D Cartesian Grids](https://arxiv.org/abs/2409.11545).

## References

The UMANS code is taken and adapted from: https://gitlab.inria.fr/OCSR/UMANS (Accompanying paper: [Generalized Microscopic Crowd Simulation using Costs in Velocity Space](https://project.inria.fr/crowdscience/generalized-microscopic-crowd-simulation-using-costs-in-velocity-space-i3d-2020/)).

The ChAOS code is taken and adapted from: https://gitlab.inria.fr/OCSR/chaos.

The visibility-based-marching code is taken and adapted from: https://github.com/IbrahimSquared/visibility-based-marching.

Code in the `Initial Commit` of the `UMANS` and `ChAOS` folders are almost entirely by the original authors and all credit goes to them. For full references and accreditation, visit the above `GitLab` links.

Code in the `visibility-based-marching` folder is almost entirely by the original authors. Only code for integration-sake has been added. For full references and accreditation, visit the above `GitHub` link.

My own additions can be found in the future commits.

## Getting Started

To be filled in.

## UMANS Details

Basic unoptimized SPH with SPH obstacle handling and density-based blending has been implemented for the UMANS system according to the method described in [SPH crowds: Agent-based crowd simulation up to
extreme densities using fluid dynamics](https://inria.hal.science/hal-03270915/file/CAG2021-SPHCrowds%20-%20Author%20version.pdf). 

Some parameters have been updated for faster simulation of large crowds: 

Simulation Time-Step = 0.05s

Local Collision Range = 3m

The Social Forces model of local collision is used exclusively for this simulation system. Other local collision algorithms remain untested.

RK4, Verlet and Leapfrog integration have also been implemented alongside the default Euler integration for testing purposes. However, only the Euler integration mode is recommended for best simulation behavior.
Legacy Coarse Time-Step and Fine Time-Step systems have been implemented for testing purposes, but currently only a unified Fine Time-Step is in use.

For integration with the VBM algorithm as described in [Exact Wavefront Propagation for Globally Optimal One-to-All Path Planning on 2D Cartesian Grids](https://arxiv.org/abs/2409.11545), UMANS now accepts `map` objects that contain a distance grid indicating distance from current grid square to goal, and a vector grid indicating direction from current grid square to goal (both using the computed shortest path). All `maps` should contain only one goal point each. If multiple goal points are required, multiple maps should be added to the simulation. At every time-step (more specifically when `agentID` % `numberOfMaps` == `timeStepCount` % `numberOfMaps` to ensure average O(1) additional time), distance-to-goal of all `maps` are compared and the vector of the `map` with the shortest distance is chosen.

Additional logic has also been implemented to allow dynamic goal-switching based on a map-level `distanceMultiplier` parameter (equals to 1/average speed of all agents moving towards that goal). At every time-step, distance-to-goal * `distanceMultiplier` (equivalent to time-to-goal) is compared and the vector of the `map` with the shortest time-to-goal is chosen.

## VBM Details

VBM is a 2D grid-based method that calculates shortest path direction vectors for every grid position in the entire grid in O(n) time and space using the principle of line-of-sight.

The VBM solver has been updated to take in `png` obstacle files and output a `visibilityBased.txt` file (distance grid) and a `visibilityBased_vectorFrom` file (vector grid) in a unified coordinate system acceptable by the UMANS system.

## Environment and Agent Toolkit Details

`EnvironmentToolkit.py` has been written with functions to convert `obstacles.geojson` files into `obstacles.png` and `obstacles.xml` formats readable by the VBM solver and the UMANS system respectively.

`AgentToolkit.py` has been written with a function to write agents (in rectangular blocks with randomized placement) to an `agents.xml` file readable by the UMANS system.

For more details on the functions, visit the respective python files in `CrowdSim/scripts`.

## Usage

![Simulation Pipeline](https://github.com/user-attachments/assets/83e2cfa1-203d-4a77-958b-cd84b08700df)

The UMANS system takes in `obstacles.xml`, `agents.xml`, `distance.txt` and `vector.txt` files linked together by a `simulation.xml` file and outputs either a 2D simulation or a folder containing `.csv` files that store agent coordinates. QGIS, Environment Toolkit, Agent Toolkit and the VBM solver can be used to generate the respective files. 

## QGIS

QGIS is an open-source geospatial software that allows users to add maps and draw polygons using selected coordinate reference systems.

To set-up:
- Download QGIS from https://qgis.org/ (while likely incoonsequential, the version used for this pipeline is 3.40.4).
- Open the software.
- In the browser menu found on the left-hand side of the application window, right-click on `XYZ Tiles` and choose `New Connection`.
- Name the new connection `Google Maps` and add https://mt1.google.com/vt/lyrs=r&x={x}&y={y}&z={z} as the URL.
- Repeat the above steps to open a second `New Connection`. Name it `Google Satellite Hybrid` and add https://mt1.google.com/vt/lyrs=y&x={x}&y={y}&z={z} as the URL.

To draw obstacles:
- Drag both the `Google Maps` and `Google Satellite Hybrid` XYZ tiles into the Layers menu located on the bottom left of the application. The maps should become visible in the editor on the right. These two maps are recommended for aiding in drawing of obstacles.
- Click on the icon on the bottom right of the application window labeled `EPSG:xxxx` and search for and switch the Coordinate Reference System to `SVY21 / Singapore TM`.
- From the options at the top of the application window, navigate to `Layer` -> `Create Layer` -> `New Shapefile Layer`.
- Give the file a name and select a location for the file. Select a geometry type of `Polygon` and ensure the Coordinate Reference System chosen is `SVY21 / Singapore TM`. Confirm the choices and close the window. A new layer should have been created.
- From the icons at the top of the application window, select the yellow pencil icon labeled `Toggle Editing` to turn on editing and allow drawing of polygons.
- Select the green polygon icon labeled `Add polygon` to add polygon points by clicking on locations on the map. Navigate the map by holding down the mouse scroll button (scroll to zoom). When all polygon points are drawn, right-click to finalize the polygon. When prompted, provide a reference ID number for the polygon.
- Polygons can be edited by clicking on the hammer and screwdriver icon labeled `Vertex Tools` immediately to the right of the `Add polygon` icon.
- Repeat until satisfactory.
- Click the `Toggle Editing` icon to finish editing and save the layer.

To export as `.geojson`:
- Right-click on the shapefile layer containing all the polygons, navigate to `Export` -> `Save Features as`.
- Select `GeoJSON` as the file type and ensure the Coordinate Reference System chosen is `SVY21 / Singapore TM`.
- Choose an appropriate file name and location to save the file.

To re-import old layers for further editing:
- From the options at the top of the application window, navigate to `Layer` -> `Add Layer` -> `Add Vector Layer`.
- Select `File` as the source type and pick the appropriate `.geojson` file to be added as a layer.

### VBM Solver

The VBM Solver is controlled by the `settings.config` file found in the `CrowdSim\visibility-based-marching\config` folder. In particular, `imagePath` has should be set to the path of the `obstacle.png` file, and `initialFrontline` should be set to `{x, y}` where `x` and `y` represent the x and y coordinates of the goal point to be solved for.

Refer to `CrowdSim\visibility-based-marching\config\settings.config` for a full example.

The VBM Solver outputs to `CrowdSim\visibility-based-marching\build\output`.

### obstacles.xml

```
<?xml version="1.0" encoding="utf-8"?>
<Obstacles>
  <Offset x="21090.672486" y="31710.897441"/>
  <Dimension width="488" height="477"/>
  <Obstacle>
    <Point x="21335.874815" y="31868.248084"/>
    <Point x="21348.483449" y="31874.957129"/>
    <Point x="21363.568577" y="31874.509482"/>
    <Point x="21365.369315" y="31854.605047"/>
    <Point x="21346.456476" y="31849.238019"/>
    <Point x="21335.874815" y="31868.248084"/>
  </Obstacle>
  <Obstacle>
    ...
  </Obstacle>
</Obstacles>
```

`Offset` represents the bottom-left corner of the environment bounding box, usually with `x` equaling the left-most obstacle x-coordinate and `y` equaling the bottom-most obstacle y-coordinate (with some optional buffer)

`Dimension` represents the dimension of the environment bounding box.

`Obstacle` represents a multi-point polygon with the final point being the same as the starting point. Multiple obstacle polygons can be included in one `obstacles.xml` file.

All `obstacles.xml` files should be included in the `CrowdSim\UMANS\examples\obstacles` folder. Refer to `CrowdSim\UMANS\examples\obstacles\UTownConcert.xml` for a full example.

### agents.xml

```
<?xml version="1.0" encoding="utf-8"?>
<Agents>
  <Agent rad="0.2539194824565282" pref_speed="1.4" max_speed="1.8" remove_at_goal="true">
    <pos x="220.9829194659796" y="253.9913546862203"/>
    <goal x="0" y="0"/>
    <Policy id="0"/>
  </Agent>
  <Agent rad="0.23451579913311257" pref_speed="1.4" max_speed="1.8" remove_at_goal="true">
    <pos x="221.59053299515838" y="253.36828977743073"/>
    <goal x="0" y="0"/>
    <Policy id="0"/>
  </Agent>
</Agents>
```

`Agent` represents an individual agent. Multiple agents can be included in the same `agents.xml` file.

`rad` represents the radius of the agent in meters (usually centered and distributed around 0.24m)

`pref_speed` represents the preferred speed of the agent in m/s (usually chosen to be 1.4m/s)

`max_speed` represents the maximum allowable speed of the agent in m/s (usually chosen to be 1.8m/s)

`remove_at_goal` determines whether the agent gets removed from the simulation environment upon reaching its goal point. Accepts either `true` or `false`.

`pos` represents the `x` and `y` starting coordinates of the agent.

`goal` represents the `x` and `y` coordinates of the initial assigned goal point (usually arbitrarily set to `x` = 0 and `y` = 0).

`Policy id` represents the index of the chosen local collision algorithm selected (if only one local collision algorithm was specified in the `simulation.xml` file, set to 0).

All `agents.xml` files should be included in the `CrowdSim\UMANS\examples\agents` folder. Refer to `CrowdSim\UMANS\examples\agents\UTownConcert10000.xml` for a full example.

### simulation.xml

```
<?xml version="1.0" encoding="utf-8"?>
<Simulation delta_time="0.05" write_interval="1.0" end_time="600">
	<World type="Infinite" integration_mode="Euler" goal_radius="5.0" dynamic_nav="false">
		<Obstacles file="../obstacles/UTownConcert.xml"/>
	</World>
	<SPH max_density="6.0" density_blending="true"/>
	<Policies file="../policies/SocialForces.xml" />
	<Agents file="../agents/UTownConcert10000.xml" />
	<Map goal_x="292.1475140000002" goal_y="39.362558999997422" vector="../maps/UTownConcert_exit1_vector.txt" distance="../maps/UTownConcert_exit1_distance.txt"/>
	<Map goal_x="315.4475139999995" goal_y="24.35255899999902" vector="../maps/UTownConcert_exit2_vector2.txt" distance="../maps/UTownConcert_exit2_distance2.txt"/>
	<Map goal_x="402.5875139999989" goal_y="115.88255899999785" vector="../maps/UTownConcert_exit3_vector4.txt" distance="../maps/UTownConcert_exit3_distance4.txt"/>
</Simulation>
```

`Simulation` represents the header enclosing all simulation parameters.

`delta_time` represents the simulation time-step at which the simulation operates in seconds (recommended at 0.05s for best results).

`write_interval` represents the time interval in which agent positional data is written to the `.csv` files when the option to write-to-output is selected.

`end_time` represents the simulation end time. If no timing is provided, defaults to infinity.

`World` represents the header enclosing all world parameters.

`type` represents the type of world initialized. Accepts either `Infinite` or `Toric`. `Infinite` worlds are worlds that extend infinitely in all directions. `Toric` worlds have fixed boundaries that wrap around when reached. The simulation system is tested only for `Infinite` worlds.

`integration_mode` represents the integration mode of the simulation. Accepts `Euler`, `RK4`, `Verlet`, `Leapfrog`. The simulation system is only tested for `Euler` integration mode.

`goal_radius` represents the radius around the goal point in which an agent is counted to have arrived at the goal point as a multiple of agent size. 

`dynamic_nav` determines whether dynamic goal selection is active. Accepts `true` or `false`. Dynamic navigation only works if more than one `map` is provided.

`Obstacles file` represents the file location of the `obstacles.xml` file used for the simulation.

`SPH` represents the header enclosing SPH-related parameters.

`max_density` represents the maximum allowable SPH-density in people/m^2. Setting 0.0 results in SPH being deactivated.

`density_blending` determines whether the density blending framework is to be used. Accepts `true` or `false`. It enhances density control and is recommended by the SPH Crowds authors to be set to active. 

`Policies file` represents the file location of the `policy.xml` file to be used for the simulation. Changing this file changes the chosen local collision algorithm. Only the Social Forces model has been tested with the simulation system.

`Agents file` represents the file location of the `agents.xml` file to be used for the simulation. 

`Map` represents the header enclosing a single map containing paths to a single goal point in this environment. Multiple maps can be initialized in the same `simulation.xml` file.

`goal_x` and `goal_y` represent the `x` and `y` coordinates of the goal point of the respective map. 

`vector` represents the file location of the `vector.txt` file to be used for this map object. All `vector.txt` files should be included in the `CrowdSim\UMANS\examples\maps` folder.

`distance` represents the file location of the `distance.txt` file to be used for this map object. All `distance.txt` files should be included in the `CrowdSim\UMANS\examples\maps` folder.

All `simulation.xml` files should be included in the `CrowdSim\UMANS\examples\custom` folder. Refer to `CrowdSim\UMANS\examples\custom\UTownConcert.xml` for a full example.

### Examples

The following working examples have been provided:

**Room Scenario** for basic testing of SPH density functionality : `CrowdSim\UMANS\examples\custom\RoomEvacuationTest.xml`

**Room Wall Scenario** for basic testing of VBM Global Path Planning functionality : `CrowdSim\UMANS\examples\custom\RoomWallTest.xml`

**Room Two Exit Scenario** for basic testing of Map-level speed value Dynamic Exit Selection functionality : `CrowdSim\UMANS\examples\custom\RoomTwoExit.xml`

**Corridor Two Exit Scenario** for further testing of Map-level speed value Dynamic Exit Selection functionality : `CrowdSim\UMANS\examples\custom\CorridorTwoExit.xml`

**NUS UTown Concert Evacuation Scenario** for integration testing and limit-testing of large-scale crowds : `CrowdSim\UMANS\examples\custom\UTownConcert.xml`
