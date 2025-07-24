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

#include <core/crowdSimulator.h>

#include <tools/HelperFunctions.h>
#include <tools/TrajectoryCSVWriter.h>
#include <tools/heatmapPNGWriter.h>
#include <3rd-party/lodepng/lodepng.h>
#include <core/worldInfinite.h>
#include <core/worldToric.h>
#include <core/costFunctionFactory.h>
#include <core/sph.h>
#include <memory>
#include <clocale>
#include <filesystem>

#include <omp.h>
#include <stdexcept>

#include <fstream>

CrowdSimulator::CrowdSimulator()
{
	CostFunctionFactory::RegisterAllCostFunctions();
	writer_ = nullptr;
	pngWriter_ = nullptr;
	end_time_ = MaxFloat;
}

void CrowdSimulator::StartCSVOutput(const std::string &dirname, bool byAgent, bool flushImmediately)
{
	// create the CSV writer if it did not exist yet
	if (writer_ == nullptr)
		writer_ = new TrajectoryCSVWriter(byAgent, flushImmediately);

	// try to set the directory
	if (!writer_->SetOutputDirectory(dirname))
	{
		std::cerr << "Error: Could not set CSV output directory to " << dirname << "." << std::endl
			<< "The program will be unable to write CSV output." << std::endl;
		delete writer_;
		writer_ = nullptr;
	}
}

void CrowdSimulator::StopCSVOutput()
{
	if (writer_ != nullptr)
	{
		delete writer_;
		writer_ = nullptr;
	}
}

void CrowdSimulator::StartPNGOutput(const std::string& dirname)
{
	// create the PNG writer if it did not exist yet
	if (pngWriter_ == nullptr)
		pngWriter_ = new heatmapPNGWriter();

	// try to set the directory
	if (!pngWriter_->SetOutputDirectory(dirname))
	{
		std::cerr << "Error: Could not set PNG output directory to " << dirname << "." << std::endl
			<< "The program will be unable to write PNG output." << std::endl;
		delete pngWriter_;
		pngWriter_ = nullptr;
	}
}

void CrowdSimulator::StopPNGOutput()
{
	if (pngWriter_ != nullptr)
	{
		delete pngWriter_;
		pngWriter_ = nullptr;
	}
}

CrowdSimulator::~CrowdSimulator()
{
	// delete the CSV writer
	if (writer_ != nullptr)
		delete writer_;

	// delete the PNG writer
	if (pngWriter_ != nullptr)
		delete pngWriter_;

	// delete all policies
	for (auto& policy : policies_)
		delete policy.second;
	policies_.clear();

	// clear all cost-function creation functions
	CostFunctionFactory::ClearRegistry();
}

void CrowdSimulator::RunSimulationSteps(int nrSteps)
{
	for (int i = 0; i < nrSteps; ++i)
	{
		world_->DoStep();

		write_time_ += world_->GetFineDeltaTime();
		if (write_time_ >= write_interval_)
			write_time_ = 0.0f;

		png_write_time_ += world_->GetFineDeltaTime();
		if (png_write_time_ >= png_write_interval_)
		{
			png_write_time_ = 0.0f;
		}

		if (writer_ != nullptr && write_time_ == 0.0f)
		{
			double t = world_->GetCurrentTime();
			const auto& agents = world_->GetAgents();

			AgentTrajectoryPoints data;
			//#pragma omp parallel for
			//for (int j = 0; j < agents.size(); j++)
			//{
			//	Agent* agent = agents[j];
			//	data[agent->getID()] = TrajectoryPoint(t, agent->getPosition(), agent->getViewingDirection());
			//}

			//for (const Agent* agent : agents)
			//	data[agent->getID()] = TrajectoryPoint(t, agent->getPosition(), agent->getViewingDirection(), agent->getColor());

			for (const Agent* agent : agents)
			{
				float x = agent->getPosition().x + world_->GetOffset()->x;
				float y = agent->getPosition().y + world_->GetOffset()->y;
				data[agent->getID()] = TrajectoryPoint(t, Vector2D(x, y), agent->getViewingDirection(), agent->getColor());
			}

			if (writer_->GetByAgent())
			{
				writer_->AppendAgentData(data);
			}
			else
			{
				writer_->FlushByTimeStep(data, flushCount_);
				flushCount_ += 1;
			}
		}

		if (pngWriter_ != nullptr && obstaclesSuccess_ && png_write_time_ == 0.0f)
		{
			double t = world_->GetCurrentTime();
			const auto& agents = world_->GetAgents();
		
			densityArray_ = std::unique_ptr<int[]>(new int[world_->GetWidth() * world_->GetHeight()]());

			for (const Agent* agent : agents)
				densityArray_[std::floor(agent->getPosition().y) * world_->GetWidth() + std::floor(agent->getPosition().x)] += 1;

			if (pngWriter_->writePNG(densityArray_, obstaclesArray_, world_->GetWidth(), world_->GetHeight(), pngCount_))
			{
				pngCount_ += 1;
			}
		}
	}
}

void CrowdSimulator::RunSimulationUntilEnd(bool showProgressBar, bool measureTime)
{
	if (end_time_ == MaxFloat || end_time_ <= 0)
	{
		std::cerr << "Error: The end time of the simulation is not correctly specified." << std::endl
			<< "The simulation cannot be run because it is unclear when it should end." << std::endl;
		return;
	}

	const int nrIterations_ = (int)ceilf(end_time_ / world_->GetFineDeltaTime());

	// get the current system time; useful for time measurements later on
	const auto& startTime = HelperFunctions::GetCurrentTime();

	// In preparation for writing to files
	double t = world_->GetCurrentTime();
	const auto& agents = world_->GetAgents();

	// Write initial agent positions
	if (writer_ != nullptr)
	{
		AgentTrajectoryPoints data;

		for (const Agent* agent : agents)
		{
			float x = agent->getPosition().x + world_->GetOffset()->x;
			float y = agent->getPosition().y + world_->GetOffset()->y;
			data[agent->getID()] = TrajectoryPoint(t, Vector2D(x, y), agent->getViewingDirection(), agent->getColor());
		}

		if (writer_->GetByAgent())
		{
			writer_->AppendAgentData(data);
		}
		else
		{
			writer_->FlushByTimeStep(data, flushCount_);
			flushCount_ += 1;
		}
	}

	// Write initial heatmap
	if (pngWriter_ != nullptr && obstaclesSuccess_)
	{
		densityArray_ = std::unique_ptr<int[]>(new int[world_->GetWidth() * world_->GetHeight()]());

		for (const Agent* agent : agents)
			densityArray_[std::floor(agent->getPosition().y) * world_->GetWidth() + std::floor(agent->getPosition().x)] += 1;

		if (pngWriter_->writePNG(densityArray_, obstaclesArray_, world_->GetWidth(), world_->GetHeight(), pngCount_))
		{
			pngCount_ += 1;
		}
	}

	if (showProgressBar)
	{
		// do the simulation in K blocks, and show a progress bar with K blocks
		const int nrProgressBarBlocks = 20;
		int nrIterationsPerBlock = nrIterations_ / nrProgressBarBlocks;
		int nrIterationsDone = 0;

		// - print an empty progress bar first, to indicate how long the real bar will be
		std::string progressBar = "Simulation progress: |";
		for (int i = 0; i < nrProgressBarBlocks; ++i)
			progressBar += "-";
		progressBar += "|100%";
		std::cout << std::endl << progressBar << std::endl;

		// - on a new line, start writing a new progress bar
		std::cout << "                     [";
		for (int i = 0; i < nrProgressBarBlocks; ++i)
		{
			// run a block of iterations
			int nrIterationsToDo = (i+1 == nrProgressBarBlocks ? nrIterations_ - nrIterationsDone : nrIterationsPerBlock);
			RunSimulationSteps(nrIterationsToDo);

			// augment the progress bar
			std::cout << "#" << std::flush;
			nrIterationsDone += nrIterationsToDo;
		}
		std::cout << "]" << std::endl << std::endl;
	}
	else
	{
		// do all steps at once without any printing
		RunSimulationSteps(nrIterations_);
	}

	if (measureTime)
	{
		// report running times
		const auto& endTime = HelperFunctions::GetCurrentTime();
		const auto& timeSpent = HelperFunctions::GetIntervalMilliseconds(startTime, endTime);

		std::cout << "Time simulated: " << world_->GetCurrentTime() << " seconds." << std::endl;
		std::cout << "Computation time used: " << timeSpent / 1000 << " seconds." << std::endl;
	}

	if (writer_ != nullptr)
		writer_->Flush();
}

#pragma region [Loading a configuration file]

std::string getFolder(const std::string& filename)
{
	std::filesystem::path path(filename);
	path.remove_filename();
	return path.string();
}

bool CrowdSimulator::FromConfigFile_loadWorld(const tinyxml2::XMLElement* worldElement)
{
	// load the world type
	WorldBase::Type worldType = WorldBase::Type::UNKNOWN_WORLD_TYPE;
	const char* type = worldElement->Attribute("type");
	if (type != nullptr)
		worldType = WorldBase::StringToWorldType(type);

	if (worldType == WorldBase::Type::UNKNOWN_WORLD_TYPE)
	{
		std::cerr << "Warning: No valid world type specified in the XML file. Selecting default type (Infinite)." << std::endl;
		worldType = WorldBase::Type::INFINITE_WORLD;
	}

	if (worldType == WorldBase::Type::INFINITE_WORLD)
	{
		world_ = std::make_unique<WorldInfinite>();
		std::cout << "Created Infinite world." << std::endl;
	}

	// for toric worlds, load the width and height
	else if (worldType == WorldBase::Type::TORIC_WORLD)
	{
		float width = -1, height = -1;
		worldElement->QueryFloatAttribute("width", &width);
		worldElement->QueryFloatAttribute("height", &height);

		if (width > 0 && height > 0)
		{
			world_ = std::make_unique<WorldToric>(width, height);
			std::cout << "Created Toric world, width " << width << " and height " << height << "." << std::endl;
		}
		else
		{
			std::cerr << "Error: No valid size specified for the toric world in the XML file." << std::endl
				<< "Make sure to specify a non-negative width and height." << std::endl;
			return false;
		}
	}

	WorldBase::Integration_Mode intMode = WorldBase::Integration_Mode::UNKNOWN;
	const char* mode = worldElement->Attribute("integration_mode");
	if (mode != nullptr)
		intMode = WorldBase::StringToIntegrationMode(mode);

	if (intMode == WorldBase::Integration_Mode::UNKNOWN)
	{
		std::cerr << "Warning: No valid integration mode specified in the XML file. Selecting default type (Semi-Implicit Euler)." << std::endl;
		intMode = WorldBase::Integration_Mode::EULER;
	}
	if (intMode == WorldBase::Integration_Mode::EULER)
	{
		std::cout << "World Integration Mode set to Semi-Implicit Euler." << std::endl;
	}
	else if (intMode == WorldBase::Integration_Mode::RK4)
	{
		std::cout << "World Integration Mode set to Rugge-Kutta 4." << std::endl;
	}
	else if (intMode == WorldBase::Integration_Mode::VERLET2)
	{
		std::cout << "World Integration Mode set to Velocity-Verlet." << std::endl;
	}
	else if (intMode == WorldBase::Integration_Mode::LEAPFROG2)
	{
		std::cout << "World Integration Mode set to Leapfrog." << std::endl;
	}
	world_->SetMode(intMode);

	const char* goalRadius = worldElement->Attribute("goal_radius");
	std::string goalRadius_str;
	float goalRadius_float = 1.0;

	if (goalRadius != nullptr)
	{
		goalRadius_str = (std::string) goalRadius;
		try
		{
			goalRadius_float = stof(goalRadius_str);
		}
		catch (std::logic_error& e)
		{
			std::cerr << "Error initializing goal radius. Default value of 1.0 will be initialized." << std::endl;
		}

		if (goalRadius_float <= 0.0f)
		{
			goalRadius_float = 1.0;
			std::cerr << "Error initializing goal radius. Ensure a goal radius larger than 0. Default value of 1.0 will be initialized." << std::endl;
		}
	}
	
	world_->SetGoalRadius(goalRadius_float);

	const char* isNearestNav = worldElement->Attribute("nearest_nav");
	std::string isNearestNav_str;
	if (isNearestNav != nullptr)
		isNearestNav_str = (std::string)isNearestNav;
	if (isNearestNav_str == "true")
	{
		world_->SetIsActiveNearestNav(true);
		std::cout << "Nearest navigation initialized. Will only work if more than one map is specified." << std::endl;
	}

	const char* isDynamicNav = worldElement->Attribute("dynamic_nav");
	std::string isDynamicNav_str;
	if (isDynamicNav != nullptr)
		isDynamicNav_str = (std::string) isDynamicNav;
	if (isDynamicNav_str == "true")
	{
		world_->SetIsActiveDynamicNav(true);
		std::cout << "Dynamic navigation initialized. Will only work if more than one map is specified and nearest navigation is also active." << std::endl;
	}

	return true;
}

bool CrowdSimulator::FromConfigFile_loadSPH(const tinyxml2::XMLElement* SPHElement)
{
	// load SPH settings if specified
	const char* maxDensity = SPHElement->Attribute("max_density");
	std::string maxDensity_str(maxDensity);
	float maxDensity_float = 0.0;

	try 
	{
		 maxDensity_float = stof(maxDensity_str);
	}
	catch (std::logic_error& e)
	{
		std::cerr << "Error initializing SPH max density. Ensure a float value between 0.0 and 100.0 is entered." << std::endl;
		return false;
	}
	
	if (maxDensity_float < 0.0f || maxDensity_float > 100.0f)
	{
		std::cerr << "Error initializing SPH max density. Ensure a float value between 0.0 and 100.0 is entered." << std::endl;
		return false;
	}

	if (maxDensity_float == 0.0f)
	{
		std::cout << "Initialized simulation without SPH." << std::endl;
		return true;
	}

	if (maxDensity_float != 0.0f)
	{
		world_->GetSPH()->setMaxRestDensity(maxDensity_float);
		world_->SetIsActiveSPH(true);
		std::cout << "Successfully initialized SPH with SPH max density of " << maxDensity_str << "." << std::endl;
		const char* useDensityBlending = SPHElement->Attribute("density_blending");
		std::string useDensityBlending_str(useDensityBlending);
		if (useDensityBlending_str == "true")
		{
			world_->SetIsActiveDensityBlending(true);
			std::cout << "Initialized SPH with density-based blending active." << std::endl;
			return true;
		}
		else if (useDensityBlending_str == "false")
		{
			world_->SetIsActiveDensityBlending(false);
			std::cout << "Initialized SPH without density-based blending active." << std::endl;
			return true;
		}
		else
		{
			world_->SetIsActiveDensityBlending(false);
			std::cerr << "\'density_blending\' parameter expects either true or false. Initializing SPH without density-based blending." << std::endl;
			return false;
		}
	}
}

bool CrowdSimulator::FromConfigFile_loadPoliciesBlock_ExternallyOrNot(const tinyxml2::XMLElement* xmlBlock, const std::string& fileFolder)
{
	// - if this block refers to another file, read it
	const char* externalFilename = xmlBlock->Attribute("file");
	if (externalFilename != nullptr)
	{
		tinyxml2::XMLDocument externalDoc;
		externalDoc.LoadFile((fileFolder + externalFilename).data());
		if (externalDoc.ErrorID() != 0)
		{
			std::cerr << "Could not load or parse Policies XML file at " << (fileFolder + externalFilename) << "." << std::endl
				<< "Please check this file location, or place your Policies in the main XML file itself." << std::endl;
			return false;
		}

		return FromConfigFile_loadPoliciesBlock(externalDoc.FirstChildElement("Policies"));
	}

	// - otherwise, read the agents straight from the file itself
	return FromConfigFile_loadPoliciesBlock(xmlBlock);
}

bool CrowdSimulator::FromConfigFile_loadAgentsBlock_ExternallyOrNot(const tinyxml2::XMLElement* xmlBlock, const std::string& fileFolder)
{
	// - if this block refers to another file, read it
	const char* externalFilename = xmlBlock->Attribute("file");
	if (externalFilename != nullptr)
	{
		tinyxml2::XMLDocument externalDoc;
		externalDoc.LoadFile((fileFolder + externalFilename).data());
		if (externalDoc.ErrorID() != 0)
		{
			std::cerr << "Could not load or parse Agents XML file at " << (fileFolder + externalFilename) << "." << std::endl
				<< "Please check this file location, or place your Agents in the main XML file itself." << std::endl;
			return false;
		}

		return FromConfigFile_loadAgentsBlock(externalDoc.FirstChildElement("Agents"));
	}

	// - otherwise, read the agents straight from the file itself
	return FromConfigFile_loadAgentsBlock(xmlBlock);
}

bool CrowdSimulator::FromConfigFile_loadObstaclesBlock_ExternallyOrNot(const tinyxml2::XMLElement* xmlBlock, const std::string& fileFolder)
{
	// - if this block refers to another file, read it
	const char* externalFilename = xmlBlock->Attribute("file");
	if (externalFilename != nullptr)
	{
		tinyxml2::XMLDocument externalDoc;
		externalDoc.LoadFile((fileFolder + externalFilename).data());
		if (externalDoc.ErrorID() != 0)
		{
			std::cerr << "Could not load or parse Obstacles XML file at " << (fileFolder + externalFilename) << "." << std::endl
				<< "Please check this file location, or place your Obstacles in the main XML file itself." << std::endl;
			return false;
		}

		return FromConfigFile_loadObstaclesBlock(externalDoc.FirstChildElement("Obstacles"));
	}

	// - otherwise, read the agents straight from the file itself
	return FromConfigFile_loadObstaclesBlock(xmlBlock);
}

bool CrowdSimulator::FromConfigFile_loadObstaclesPNG(const tinyxml2::XMLElement* xmlBlock, const std::string& fileFolder)
{
	// - if this block refers to another file, read it
	const char* externalFilename = xmlBlock->Attribute("file");
	if (externalFilename != nullptr)
	{
		// Decode the PNG file
		unsigned width, height;
		unsigned error = lodepng::decode(obstaclesArray_, width, height, fileFolder + externalFilename);
		if (error)
		{
			std::cerr << lodepng_error_text(error) << std::endl;
			std::cout << "Heatmaps will not be displayed." << std::endl;
			obstaclesSuccess_ = false;
			return false;
		}
		else
		{
			std::cout << "Successfully loaded obstacles PNG file. Heatmaps can be displayed." << std::endl;
			obstaclesSuccess_ = true;
			return true;
		}
	}
	else
	{
		std::cout << "Failed to load obstacles PNG file. Heatmaps will not be displayed." << std::endl;
		return false;
	}
	
}

bool CrowdSimulator::FromConfigFile_loadMapBlock(const tinyxml2::XMLElement* xmlBlock, const std::string& fileFolder, const int num_threads)
{
	const char* goal_x = xmlBlock->Attribute("goal_x");
	std::string goal_x_str(goal_x);
	const char* goal_y = xmlBlock->Attribute("goal_y");
	std::string goal_y_str(goal_y);

	int width = world_->GetWidth();
	int height = world_->GetHeight();
	vectorMap* m = new vectorMap(width, height, num_threads);
	m->setGoal(Vector2D(stof(goal_x_str), stof(goal_y_str)));

	// - if this block refers to another file, read it
	const char* externalFilename = xmlBlock->Attribute("vector");
	if (externalFilename != nullptr)
	{
		std::ifstream f(fileFolder + externalFilename);
		if (!f.is_open())
		{
			std::cerr << "Could not load or parse Map vector txt file at " << (fileFolder + externalFilename) << "." << std::endl
				<< "Please check this file location." << std::endl;
			return false;
		}
		
		std::string s;
		float x_val = 0;
		float y_val = 0;
		std::string temp = "";
		int i;
		for (int j = 0; j < height; ++j)
		{
			i = 0;
			getline(f, s);
			for (char c : s)
			{
				if (c == ' ')
				{
					if (i % 2 == 0)
					{
						x_val = stof(temp);
					}
					else
					{
						y_val = stof(temp);
						m->setVector((int)std::floor(i / 2), j, Vector2D(x_val - i / 2, y_val - j - 0.5f).getnormalized());
					}
					temp = "";
					++i;
				}
				else
				{
					temp += c;
				}
			}
		}
		f.close();

		const char* externalFilename = xmlBlock->Attribute("distance");
		if (externalFilename != nullptr)
		{
			std::ifstream f(fileFolder + externalFilename);
			if (!f.is_open())
			{
				std::cerr << "Could not load or parse Map distance txt file at " << (fileFolder + externalFilename) << "." << std::endl
					<< "Please check this file location." << std::endl;
				return false;
			}

			std::string s;
			float dist = 0;
			std::string temp = "";
			int i;
			for (int j = 0; j < height; ++j)
			{
				i = 0;
				getline(f, s);
				for (char c : s)
				{
					if (c == ' ')
					{
						dist = stof(temp);
						m->setDistance(i, j, dist);
						temp = "";
						++i;
					}
					else
					{
						temp += c;
					}
				}
			}
			f.close();
		}
		world_->AddMap(m);
		world_->SetIsActiveGlobalNav(true);
		std::cout << "Loaded " << externalFilename << " as map." << std::endl;
		return true;
	}
	return false;
}

bool CrowdSimulator::FromConfigFile_loadPoliciesBlock(const tinyxml2::XMLElement* xmlBlock)
{
	// load the elements one by one
	const tinyxml2::XMLElement* element = xmlBlock->FirstChildElement();
	while (element != nullptr)
	{
		// load a single element
		if (!FromConfigFile_loadSinglePolicy(element))
			return false;

		// go to the next element
		element = element->NextSiblingElement("Policy");
	}

	return true;
}

bool CrowdSimulator::FromConfigFile_loadAgentsBlock(const tinyxml2::XMLElement* xmlBlock)
{
	// load the elements one by one
	const tinyxml2::XMLElement* element = xmlBlock->FirstChildElement();
	while (element != nullptr)
	{
		// load a single element
		if (!FromConfigFile_loadSingleAgent(element))
			return false;

		// go to the next element
		element = element->NextSiblingElement("Agent");
	}

	return true;
}

bool CrowdSimulator::FromConfigFile_loadObstaclesBlock(const tinyxml2::XMLElement* xmlBlock)
{
	// load the elements one by one
	float offset_x = 0.0f;
	float offset_y = 0.0f;
	const tinyxml2::XMLElement* element = xmlBlock->FirstChildElement("Offset");
	if (element != nullptr)
	{
		element->QueryFloatAttribute("x", &offset_x);
		element->QueryFloatAttribute("y", &offset_y);
		world_->SetOffset(Vector2D(offset_x, offset_y));
	}
	int width = 0;
	int height = 0;
	element = xmlBlock->FirstChildElement("Dimension");
	if (element != nullptr)
	{
		element->QueryIntAttribute("width", &width);
		element->QueryIntAttribute("height", &height);
		world_->SetWidth(width);
		world_->SetHeight(height);
	}
	
	element = xmlBlock->FirstChildElement("Obstacle");
	while (element != nullptr)
	{
		// load a single element
		if (!FromConfigFile_loadSingleObstacle(element))
			return false;

		// go to the next element
		element = element->NextSiblingElement("Obstacle");
	}

	return true;
}

bool CrowdSimulator::FromConfigFile_loadSinglePolicy(const tinyxml2::XMLElement* policyElement)
{
	// --- Read mandatory parameters

	// Unique policy ID
	int policyID;
	policyElement->QueryIntAttribute("id", &policyID);

	auto name = policyElement->Attribute("name");

	// Optimization method
	auto methodName = policyElement->Attribute("OptimizationMethod");
	Policy::OptimizationMethod method;
	if (!Policy::OptimizationMethodFromString(methodName == nullptr ? "" : methodName, method))
	{
		std::cerr << "Error in Policy " << policyID << ": Optimization method invalid." << std::endl;
		return false;
	}

	// Sampling parameters
	SamplingParameters params;
	if (method == Policy::OptimizationMethod::SAMPLING)
	{
		policyElement->QueryAttribute("SamplingAngle", &params.angle);
		policyElement->QueryAttribute("SpeedSamples", &params.speedSamples);
		policyElement->QueryAttribute("AngleSamples", &params.angleSamples);
		policyElement->QueryAttribute("RandomSamples", &params.randomSamples);
		policyElement->QueryBoolAttribute("IncludeBaseAsSample", &params.includeBaseAsSample);

		// type of sampling (= random or regular)
		const char * res = policyElement->Attribute("SamplingType");
		if (res != nullptr && !SamplingParameters::TypeFromString(res, params.type))
			std::cerr << "Policy " << policyID << ": sampling type invalid, using default (regular)." << std::endl;

		// base velocity (= origin of the cone or circle being sampled)
		res = policyElement->Attribute("SamplingBase");
		if (res != nullptr && !SamplingParameters::BaseFromString(res, params.base))
			std::cerr << "Policy " << policyID << ": sampling base invalid, using default (zero)." << std::endl;

		// base direction (= central direction of the cone or circle)
		res = policyElement->Attribute("SamplingBaseDirection");
		if (res != nullptr && !SamplingParameters::BaseDirectionFromString(res, params.baseDirection))
			std::cerr << "Policy " << policyID << ": sampling base direction invalid, using default (current velocity)." << std::endl;

		// radius (= radius of the cone or circle)
		res = policyElement->Attribute("SamplingRadius");
		if (res != nullptr && !SamplingParameters::RadiusFromString(res, params.radius))
			std::cerr << "Policy " << policyID << ": sampling radius type invalid, using default (preferred speed)." << std::endl;
	}

	// --- Create the policy

	Policy* pl = new Policy(name, method, params);

	// --- Read optional parameters

	// Relaxation time
	float relaxationTime = 0;
	if (policyElement->QueryFloatAttribute("RelaxationTime", &relaxationTime) == tinyxml2::XMLError::XML_SUCCESS)
		pl->setRelaxationTime(relaxationTime);

	// Force scale
	//float contactForceScale = 0;
	//if (policyElement->QueryFloatAttribute("ContactForceScale", &contactForceScale) == tinyxml2::XMLError::XML_SUCCESS)
	//	pl->setContactForceScale(contactForceScale);

	// --- Read and create cost functions

	// read all cost functions that belong to the policy; instantiate them one by one
	auto* funcElement = policyElement->FirstChildElement("costfunction");
	while (funcElement != nullptr)
	{
		const auto& costFunctionName = funcElement->Attribute("name");
		CostFunction* costFunction = CostFunctionFactory::CreateCostFunction(costFunctionName);
		if (costFunction != nullptr)
			pl->AddCostFunction(costFunction, CostFunctionParameters(funcElement));

		funcElement = funcElement->NextSiblingElement();
	}

	if (pl->GetNumberOfCostFunctions() == 0)
	{
		std::cerr << "Error: Policy " << policyID << "needs at least one cost function element" << std::endl;
		delete pl;
		return false;
	}

	// --- Save the policy in the simulator

	if (!AddPolicy(policyID, pl))
	{
		std::cerr << "Error: Failed to add Policy " << policyID << " because its ID is already taken" << std::endl;
		delete pl;
		return false;
	}

	return true;
}

bool CrowdSimulator::FromConfigFile_loadSingleAgent(const tinyxml2::XMLElement* agentElement)
{
	// optional ID
	int agentID = -1;
	agentElement->QueryIntAttribute("id", &agentID);

	// optional agent parameters (if they are not provided, we use the default ones)
	Agent::Settings settings;
	agentElement->QueryFloatAttribute("rad", &settings.radius_);
	agentElement->QueryFloatAttribute("pref_speed", &settings.preferred_speed_);
	agentElement->QueryFloatAttribute("max_speed", &settings.max_speed_);
	agentElement->QueryFloatAttribute("max_acceleration", &settings.max_acceleration_);
	//agentElement->QueryFloatAttribute("mass", &settings.mass_);
	settings.mass_ = pow(settings.radius_ / 0.24, 2); // Set mass based on radius with a mean radius of 0.24
	agentElement->QueryBoolAttribute("remove_at_goal", &settings.remove_at_goal_);

	// optional color
	auto* colorElement = agentElement->FirstChildElement("color");
	if (colorElement)
	{
		 int r = -1, g = -1, b = -1;
		 colorElement->QueryIntAttribute("r", &r);
		 colorElement->QueryIntAttribute("g", &g);
		 colorElement->QueryIntAttribute("b", &b);
		 settings.color_ = Color((unsigned short)r, (unsigned short)g, (unsigned short)b);
	}

	// position
	float x, y;
	auto* positionElement = agentElement->FirstChildElement("pos");
	if (!positionElement)
	{
		std::cerr << "Error: Agent " << agentID << " needs a position element." << std::endl;
		return false;
	}
	positionElement->QueryFloatAttribute("x", &x);
	positionElement->QueryFloatAttribute("y", &y);
	Vector2D position(x, y);

	// policy
	auto* policyElement = agentElement->FirstChildElement("Policy");
	if (!policyElement)
	{
		std::cerr << "Error: Agent " << agentID << "needs a policy element." << std::endl;
		return false;
	}

	int agentPolicyID;
	policyElement->QueryIntAttribute("id", &agentPolicyID);
	auto policy = GetPolicy(agentPolicyID);
	if (policy == nullptr)
	{
		std::cerr << "Error: The policy with id " << agentPolicyID << " doesn't exist." << std::endl;
		return false;
	}

	settings.policy_ = policy;

	// optional start time
	float startTime = 0;
	agentElement->QueryFloatAttribute("start_time", &startTime);

	// --- Add the agent to the world.

	Agent* agent = world_->AddAgent(position, settings, (agentID >= 0 ? (size_t)agentID : std::numeric_limits<size_t>::max()), startTime);

	// optional goal
	Vector2D goal = position;
	auto* goalElement = agentElement->FirstChildElement("goal");
	if (!goalElement)
	{
		std::cerr << "Warning: Agent " << agentID << " has no goal element." << std::endl;
		std::cerr << "This agent will not move." << std::endl;
	}
	else
	{
		goalElement->QueryFloatAttribute("x", &x);
		goalElement->QueryFloatAttribute("y", &y);
		goal = Vector2D(x, y);
	}

	agent->setGoal(goal);
	return true;
}

bool CrowdSimulator::FromConfigFile_loadSingleObstacle(const tinyxml2::XMLElement* obstacleElement)
{
	std::vector<Vector2D> points;

	// load coordinates
	auto* pointElement = obstacleElement->FirstChildElement("Point");
	while (pointElement != nullptr)
	{
		float x, y;
		pointElement->QueryFloatAttribute("x", &x);
		pointElement->QueryFloatAttribute("y", &y);
		pointElement = pointElement->NextSiblingElement("Point");
		points.push_back(Vector2D(x-world_->GetOffset()->x, y-world_->GetOffset()->y));
		//points.push_back(Vector2D(x, y));
	}

	// add obstacle to world
	world_->AddObstacle(points);
	return true;
}

CrowdSimulator* CrowdSimulator::FromConfigFile(const std::string& filename, int num_threads)
{
	std::setlocale(LC_NUMERIC, "en_US.UTF-8");

	// Parse the XML into the property tree.
	// If the path cannot be resolved, an exception is thrown.
	tinyxml2::XMLDocument doc;
	doc.LoadFile(filename.data());
	if (doc.ErrorID() != 0)
	{
		std::cerr << "Error: Could not load or parse XML file at " << filename << std::endl;
		return nullptr;
	}

	const std::string& fileFolder = getFolder(filename);

	// --- Check if this is a "main" config file that refers to another config file.

	tinyxml2::XMLElement* simConfigPathElement = doc.FirstChildElement("configPath");
	if (simConfigPathElement != nullptr)
	{
		// Location of the config file should be relative to the location of the *main* config file.
		// Check if the main config file lies in a subfolder.
		return CrowdSimulator::FromConfigFile(fileFolder + simConfigPathElement->Attribute("path"), num_threads);
	}

	// --- Otherwise, we assume that this is a "regular" config file that contains the simulation itself.

	tinyxml2::XMLElement* simulationElement = doc.FirstChildElement("Simulation");
	if (simulationElement == nullptr)
	{
		std::cerr << "Error: No main Simulation element in the XML file" << std::endl;
		return nullptr;
	}

	CrowdSimulator* crowdsimulator = new CrowdSimulator();
	crowdsimulator->scenarioFilename_ = filename;

	//
	// --- Read the world parameters
	//

	tinyxml2::XMLElement* worldElement = simulationElement->FirstChildElement("World");
	if (worldElement == nullptr)
	{
		std::cerr << "Error: No World element in the XML file" << std::endl;
		delete crowdsimulator;
		return nullptr;
	}
	if (!crowdsimulator->FromConfigFile_loadWorld(worldElement))
	{
		std::cerr << "Error: Failed to load the world from the XML file" << std::endl;
		delete crowdsimulator;
		return nullptr;
	}

	//
	// --- Read the SPH parameters
	//

	tinyxml2::XMLElement* SPHElement = simulationElement->FirstChildElement("SPH");
	if (SPHElement == nullptr)
	{
		std::cerr << "Error: No SPH element in the XML file. Initializing simulation without SPH." << std::endl;
		//delete crowdsimulator;
		//return nullptr;
	}
	else if (!crowdsimulator->FromConfigFile_loadSPH(SPHElement))
	{
		std::cerr << "Error in loading SPH. Initializing simulation without SPH." << std::endl;
		//delete crowdsimulator;
		//return nullptr;
	}


	//
	// --- Read the simulation parameters
	//

	// the length of a simulation step
	float delta_time = -1;
	simulationElement->QueryFloatAttribute("delta_time", &delta_time);
	if (delta_time <= 0)
	{
		std::cerr << "Error: No valid value for delta_time found in the XML file." << std::endl
			<< "This attribute of Simulation is mandatory and should be positive." << std::endl;
		delete crowdsimulator;
		return nullptr;
	}
	crowdsimulator->GetWorld()->SetCoarseDeltaTime(delta_time);
	crowdsimulator->GetWorld()->SetFineDeltaTime(delta_time); // temporary setting for unified delta_time

	float write_interval = -1;
	simulationElement->QueryFloatAttribute("write_interval", &write_interval);
	if (write_interval <= 0)
	{
		std::cerr << "Error: No valid value for write_interval found in the XML file." << std::endl
			<< "This attribute of Simulation is mandatory and should be positive. Default value of 0.2 will be used." << std::endl;
	}
	crowdsimulator->write_interval_ = write_interval;

	// the total simulation time (optional)
	float end_time = -1;
	simulationElement->QueryFloatAttribute("end_time", &crowdsimulator->end_time_);

	//
	// --- Read policies
	//

	// read the block with all policies
	tinyxml2::XMLElement* policiesElement = simulationElement->FirstChildElement("Policies");
	if (policiesElement != nullptr && !crowdsimulator->FromConfigFile_loadPoliciesBlock_ExternallyOrNot(policiesElement, fileFolder))
	{
		std::cerr << "Error while loading policies. The simulation cannot be loaded." << std::endl;
		delete crowdsimulator;
		return nullptr;
	}

	// if there are no policies at this point, print an error, and stop loading
	if (!crowdsimulator->HasPolicies())
	{
		std::cerr
			<< "Error: Failed to load any policies for the simulation." << std::endl
			<< "A simulation needs a Policy block with at least one valid Policy element." << std::endl;
		delete crowdsimulator;
		return nullptr;
	}

	//
	// --- Read agents
	//

	// read the block with all agents
	tinyxml2::XMLElement* agentsElement = simulationElement->FirstChildElement("Agents");
	if (agentsElement != nullptr && !crowdsimulator->FromConfigFile_loadAgentsBlock_ExternallyOrNot(agentsElement, fileFolder))
	{
		std::cerr << "Error while loading agents. The simulation cannot be loaded." << std::endl;
		delete crowdsimulator;
		return nullptr;
	}

	// if there are no agents at this point, print a warning (but not an error, because an empty crowd is allowed)
	if (crowdsimulator->GetWorld()->GetAgents().empty())
	{
		std::cerr << "Warning: Failed to load any agents for the simulation." << std::endl
			<< "The simulation will start without agents." << std::endl;
	}

	// 
	// --- Read obstacles
	//

	// read the block with all obstacles
	tinyxml2::XMLElement* obstaclesElement = worldElement->FirstChildElement("Obstacles");
	if (obstaclesElement != nullptr && !crowdsimulator->FromConfigFile_loadObstaclesBlock_ExternallyOrNot(obstaclesElement, fileFolder))
	{
		std::cerr << "Error while loading obstacles. The simulation cannot be loaded." << std::endl;
		delete crowdsimulator;
		return nullptr;
	}

	//
	// --- Read obstacles png
	//

	tinyxml2::XMLElement* obstaclesPNGElement = worldElement->FirstChildElement("PNG");
	if (obstaclesPNGElement != nullptr && !crowdsimulator->FromConfigFile_loadObstaclesPNG(obstaclesPNGElement, fileFolder))
	{
		std::cerr << "Error while loading obstacles PNG. The simulation will not output heatmaps." << std::endl;
	}
	else if (obstaclesPNGElement == nullptr)
	{
		std::cout << "No Obstacles PNG file detected. The simulation will not output heatmaps." << std::endl;
	}

	// 
	// --- Read map
	//

	// read the block with map array
	tinyxml2::XMLElement* mapElement = simulationElement->FirstChildElement("Map");
	while (mapElement != nullptr)
	{
		if (!crowdsimulator->FromConfigFile_loadMapBlock(mapElement, fileFolder, num_threads))
		{
			std::cerr << "Error while loading map." << std::endl;
		}
		mapElement = mapElement->NextSiblingElement("Map");
	}
	
	if (!crowdsimulator->GetWorld()->GetIsActiveGlobalNav())
	{
		std::cout << "Warning: Initializing simulation without global navigation." << std::endl;
	}

	return crowdsimulator;
}

#pragma endregion

bool CrowdSimulator::AddPolicy(int id, Policy* policy)
{
	// if a policy with this ID already existed, don't add the policy
	if (policies_.find(id) != policies_.end())
		return false;

	// store the policy
	policies_[id] = policy;
	return true;
}

Policy* CrowdSimulator::GetPolicy(int id)
{
	auto findIt = policies_.find(id);
	if (findIt == policies_.end())
		return nullptr;
	return findIt->second;
}
