#ifndef LIB_PNGWRITER_H
#define LIB_PNGWRITER_H

#include <iostream>
#include <vector>
#include <tools/Color.h>

class heatmapPNGWriter
{
public:
	heatmapPNGWriter() {};

	/// <summary>Writes all heatmap output to PNG file, and then cleans the buffer.</summary>
	/// <param name="heatmapCount">The array containing counts of agents per grid square.</param>
	/// <param name="obstacles">The array containing the obstacles png data.</param>
	/// <param name="width">The world width.</param>
	/// <param name="height">The world height.</param>
	/// <param name="pngCount">The counter keeping track of number of PNGs written. For naming purposes.</param>
	/// <returns>true if the output was successfully written; 
	/// false otherwise, e.g. if the output folder was never specified via SetOutputDirectory().</returns>
	bool writePNG(std::unique_ptr<int[]>& heatmapCount, std::vector<unsigned char>& obstacles, int width, int height, int pngCount);

	/// <summary>Tries to set the directory to which PNG output files will be written.
	/// If this directory does not yet exist, the program will try to create it, but this operation might fail.</summary>
	/// <param name="dirname">The path to the desired output directory.</param>
	/// <returns>true if the directory was successfully set (and created if necessary); 
	/// false otherwise, i.e. if the folder did not exist and could not be created for some reason.</returns>
	bool SetOutputDirectory(const std::string& dirname);
	
protected:
	float time_ = 0.0f;
	std::string dirname_;
	Color getColorByDensity(int density);
	
};

#endif //LIB_PNGWRITER_H