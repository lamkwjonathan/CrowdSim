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

#include <tools/TrajectoryCSVWriter.h>
#include <tools/HelperFunctions.h>
#include <omp.h>

#include <iostream>
#include <fstream>

using namespace std;

bool TrajectoryCSVWriter::SetOutputDirectory(const std::string &dirname)
{
	// if necessary, append a slash to the directory path
	if (dirname.back() == '/')
		this->dirname_ = dirname;
	else
		this->dirname_ = dirname + '/';

	// try to create the directory, and return if the directory exists
	HelperFunctions::CreateDirectoryIfNonExistent(this->dirname_);
	return HelperFunctions::DirectoryExists(this->dirname_);
}

bool TrajectoryCSVWriter::Flush()
{
	if (dirname_.empty())
		return false;

    mtx_.lock();
	AgentTrajectories pos_log_copy = pos_log_;
    pos_log_.clear();
    mtx_.unlock();

	//Speed ups are limited due to hardware (SSD) chokepoint 
	//Check if multiple flushes work
	#pragma omp parallel for 
	for (int i = 0; i < pos_log_copy.size(); i++)
	{
		std::fstream file_i;
		std::string file_name = dirname_ + "output_" + std::to_string(i) + ".csv";
		file_i.open(file_name, std::ios::app);
		for (const TrajectoryPoint& record : pos_log_copy.at(i))
		{
			if (record.time == 0.1f)
			{
				file_i << "time" << "," << "pos_x" << "," << "pos_y" << "," << "pos_z" << "," << "ori_x" << "," << "ori_y" << "," << "ori_z" << "," << "color_r" << "," << "color_g" << "," << "color_b" << "\n";
			}
			file_i << record.time << ","
				<< record.position.x << "," << record.position.y << "," << 0 << ","
				<< record.orientation.x << "," << record.orientation.y << "," << 0 << ","
				<< record.color.r << "," << record.color.g << "," << record.color.b << "\n";
		}
		file_i.close();
		pos_log_copy.at(i).clear();
	}

	//for (auto& data : pos_log_copy)
	//{
 //       std::fstream file_i;
 //       size_t id = data.first;
 //       std::string file_name = dirname_ + "output_" + std::to_string(id) + ".csv";
 //       file_i.open(file_name, std::ios::app);
	//	for (const TrajectoryPoint& record : data.second)
	//	{
	//		file_i << record.time << ","
	//			<< record.position.x << "," << record.position.y << ","
	//			<< record.orientation.x << "," << record.orientation.y << "\n";
	//	}
 //       file_i.close();
 //       data.second.clear();
 //   }
	
	return true;
}

bool TrajectoryCSVWriter::FlushByTimeStep(AgentTrajectoryPoints data, int seq)
{
	if (dirname_.empty())
		return false;

	std::fstream file_i;
	std::string file_name = dirname_ + std::to_string(seq) + ".csv";
	file_i.open(file_name, std::ios::app);
	file_i << "id" << "," << "pos_x" << "," << "pos_y" << "," << "pos_z" << "," << "ori_x" << "," << "ori_y" << "," << "ori_z" << "," << "color_r" << "," << "color_g" << "," << "color_b" << "\n";
	
	for (auto& item : data) // For every agent
	{
		TrajectoryPoint record = item.second;
		file_i << item.first << ","
			<< record.position.x << "," << record.position.y << "," << 0 << ","
			<< record.orientation.x << "," << record.orientation.y << "," << 0 << ","
			<< record.color.r << "," << record.color.g << "," << record.color.b << "\n";
	}

	file_i.close();
	return true;
}

void TrajectoryCSVWriter::AppendAgentData(const AgentTrajectoryPoints& data)
{
    mtx_.lock();
    for (const auto& d : data)
        pos_log_[d.first].push_back(d.second);
    mtx_.unlock();

	if (flushImmediately)
		Flush();
}

TrajectoryCSVWriter::~TrajectoryCSVWriter()
{
	Flush();
}