/* UMANS: Unified Microscopic Agent Navigation Simulator
** MIT License
** Copyright (C) 2018-2020  Inria Rennes Bretagne Atlantique - Rainbow - Julien Pettré
**
** Permission is hereby granted, free of charge, to any person obtaining
** a copy of this software and associated documentation files (the
** "Software"), to deal in the Software without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject
** to the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
** OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
** LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
** ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
** Contact: crowd_group@inria.fr
** Website: https://project.inria.fr/crowdscience/
** See the file AUTHORS.md for a list of all contributors.
*/

#ifndef LIB_CROWD_SIMULATOR_H
#define LIB_CROWD_SIMULATOR_H

#include <core/worldBase.h>
#include <core/agent.h>
#include <map>
#include <memory>

class TrajectoryCSVWriter;
class heatmapPNGWriter;

/// <summary>Wrapper object that manages the overall crowd simulation.</summary>
class CrowdSimulator
{
private:

  /// <summary>The world in which the simulation takes place.</summary>
  std::unique_ptr<WorldBase> world_;

  /// <summary>A pointer to an optional TrajectoryCSVWriter that can write the simulation output to CSV files.</summary>
  TrajectoryCSVWriter* writer_;

  /// <summary>The time interval (in seconds) in which agent positions are recorded for writing to csv. Defaults to 0.2 if unspecified.</summary>
  float write_interval_ = 0.2;

  /// <summary>The time (in seconds) that has passed since the previous agent position record.</summary>
  float write_time_ = 0.0f;

  /// <summary>The number of times the data points have been written to CSV./// </summary>
  int flushCount_ = 0;

  /// <summary>A pointer to an optional heatmapPNGWriter that can write the simulation heatmap output to PNG files.</summary>
  heatmapPNGWriter* pngWriter_;

  /// <summary>The time interval (in seconds) in which heatmaps are recorded for writing to PNG. Defaults to 30.0 if unspecified.</summary>
  float png_write_interval_ = 5.0;

  /// <summary>The time (in seconds) that has passed since the previous PNG record.</summary>
  float png_write_time_ = 0.0f;

  /// <summary>The counter keeping track of the number of PNG files written./// </summary>
  int pngCount_ = 0;

  /// <summary>The array keeping track of the number of agents per grid square./// </summary>
  std::unique_ptr<int[]> densityArray_;

  /// <summary>The array storing obstacles PNG data./// </summary>
  std::vector<unsigned char> obstaclesArray_;

  /// <summary>The boolean indicating whether obstaclesArray_ was successfully instantiated./// </summary>
  bool obstaclesSuccess_ = false;

  /// <summary>An optional time at which the simulation should end.
  /// Only used if this number is set in a configuration file.</summary>
  float end_time_;

  /// <summary>A list containing all navigation policies, ordered by ID.
  /// Each agent uses one of these policies for its navigation.</summary>
  std::map<int, Policy*> policies_;

  std::string scenarioFilename_;

public:

  /// <summary>Creates a new CrowdSimulator object by loading a given configuration file.</summary>
  /// <remarks>Note: The caller of this method is responsible for deleting the resulting CrowdSimulator object.</remarks>
  /// <param name="filename">The name of the configuration file to load.</param>
  /// <param name="num_threads">The number of threads initialized for this simulation.</param>
  /// <returns>A pointer to new CrowdSimulator object, or nullptr if the loading failed for any reason.</returns>
  static CrowdSimulator* FromConfigFile(const std::string& filename, int num_threads);
  
  /// <summary>Destroys this CrowdSimulator object.</summary>
  ~CrowdSimulator();

  const std::string& GetScenarioFilename() const { return scenarioFilename_; }

  /// <summary>Prepares this CrowdSimulator for writing simulation output (as CSV files) to the given directory.</summary>
  /// <param name="dirname">The name of the directory to use for output.</param>
  /// <param name="byAgent">Whether to save CSV input by agent or by timestep.</param>
  /// <param name="flushImmediately">Whether or not the CSV writer should write its output files as fast as possible. 
  /// If it is true, the output files will be updated after each simulation frame.
  /// If it is false, the data to write will be cached, and files will be written when the CrowdSimulator gets destroyed.</param>
  void StartCSVOutput(const std::string& dirname, bool byAgent, bool flushImmediately);

  void StopCSVOutput();

  /// <summary>Prepares this CrowdSimulator for writing simulation heatmap output (as PNG files) to the given directory.</summary>
  /// <param name="dirname">The name of the directory to use for output.</param>
  void StartPNGOutput(const std::string& dirname);

  void StopPNGOutput();

  /// <summary>Runs the given number of simulation steps.</summary>
  /// <param name="nrSteps">The number of simulation steps to run; should be at least 1, otherwise nothing happens.</param>
  void RunSimulationSteps(int nrSteps=1);

  /// <summary>Runs the crowd simulation for the number of iterations specified in the previously loaded config file.</summary>
  /// <remarks>If the config file does not specify a number of iterations, then this method will do nothing.</remarks>
  /// <param name="showProgressBar">Whether or not to print a progress bar in the console.</param>
  /// <param name="measureTime">Whether or not to measure the total computation time and report it in the console.</param>
  void RunSimulationUntilEnd(bool showProgressBar, bool measureTime);
  
  /// <summary>Returns a pointer to the world in which the simulation takes place.</summary>
  WorldBase* GetWorld() { return world_.get(); }

  /// <summary>Stores a navigation policy inside the simulation, under the given ID.</summary>
  /// <param name="id">A unique ID for the policy.</param>
  /// <param name="policy">A Policy object.</param>
  /// <returns>true if the policy was succesfully added; false otherwise, i.e. if the ID was already taken.</returns>
  bool AddPolicy(int id, Policy* policy);

  /// <summary>Finds and returns the navigation policy with the given ID, or returns nullptr if it does not exist.</summary>
  /// <param name="id">The ID of the policy to find.</param>
  /// <returns>A pointer to the Policy stored under the given ID, or nullptr if no such Policy exists.</returns>
  Policy* GetPolicy(int id);

  inline bool HasPolicies() const { return !policies_.empty(); }

  ///// <summary>Returns the csv output time interval (in seconds).</summary>
  ///// <returns>The csv output time interval (in seconds).</summary>
  //inline float GetWriteInterval() const { return write_interval_; }

  ///// <summary>Returns the time (in seconds) since the last agent position for csv output was stored.</summary>
  ///// <returns>The time since last agent position was stored.</summary>
  //inline float GetWriteTime() const { return write_time_; }

  ///// <summary>Sets the csv output time interval (in seconds).</summary>
  //inline void SetWriteInterval(float write_interval) { write_interval_ = write_interval; }

  ///// <summary>Sets the time (in seconds) since the last agent position for csv output was stored.</summary>
  //inline void SetWriteTime(float write_time) { write_time_ = write_time; }

private:
	CrowdSimulator();

	bool FromConfigFile_loadWorld(const tinyxml2::XMLElement* worldElement);
	bool FromConfigFile_loadSPH(const tinyxml2::XMLElement* SPHElement);

	bool FromConfigFile_loadPoliciesBlock_ExternallyOrNot(const tinyxml2::XMLElement* policiesBlock, const std::string& fileFolder);
	bool FromConfigFile_loadPoliciesBlock(const tinyxml2::XMLElement* policiesBlock);
	bool FromConfigFile_loadSinglePolicy(const tinyxml2::XMLElement* policyElement);

	bool FromConfigFile_loadAgentsBlock_ExternallyOrNot(const tinyxml2::XMLElement* agentsBlock, const std::string& fileFolder);
	bool FromConfigFile_loadAgentsBlock(const tinyxml2::XMLElement* agentsBlock);
	bool FromConfigFile_loadSingleAgent(const tinyxml2::XMLElement* agentElement);

	bool FromConfigFile_loadObstaclesBlock_ExternallyOrNot(const tinyxml2::XMLElement* obstaclesBlock, const std::string& fileFolder);
	bool FromConfigFile_loadObstaclesBlock(const tinyxml2::XMLElement* obstaclesBlock);
	bool FromConfigFile_loadSingleObstacle(const tinyxml2::XMLElement* obstacleElement);

	bool FromConfigFile_loadObstaclesPNG(const tinyxml2::XMLElement* obstaclesPNG, const std::string& fileFolder);
	bool FromConfigFile_loadMapBlock(const tinyxml2::XMLElement* mapBlock, const std::string& fileFolder, int num_threads);
};

#endif
