#include <tools/heatmapPNGWriter.h>
#include <tools/HelperFunctions.h>
#include <3rd-party/lodepng/lodepng.h>
#include <iostream>
#include <string>

bool heatmapPNGWriter::SetOutputDirectory(const std::string& dirname)
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

bool heatmapPNGWriter::writePNG(std::unique_ptr<int[]>& heatmapCount, std::vector<unsigned char>& obstacles, int width, int height, int pngCount)
{
	std::vector<unsigned char> image(width * height * 4);
	int idx, idx_flipped;
	Color color;

	for (int y = height - 1; y >= 0; --y)
	{
		for (int x = 0; x < width; ++x)
		{
			idx = y * width + x;
			idx_flipped = 4 * ((height - 1 - y) * width + x);

			if (obstacles[idx_flipped + 0] == 0 && obstacles[idx_flipped + 1] == 0 && obstacles[idx_flipped + 2] == 0)
				color = Color(0, 0, 0);
			else
				color = getColorByDensity(heatmapCount[idx]);

			
			image[idx_flipped + 0] = (color.r);
			image[idx_flipped + 1] = (color.g);
			image[idx_flipped + 2] = (color.b);
			image[idx_flipped + 3] = (color.a);
		}
	}
	unsigned error = lodepng::encode(this->dirname_ + std::to_string(pngCount) + ".png", image, width, height);
	image.clear();
	
	if (error)
	{
		std::cout << lodepng_error_text(error) << std::endl;
			return false;
	}
	else
		return true;
}

Color heatmapPNGWriter::getColorByDensity(int density)
{
	short r, g, b;

	if (density == 0) // white
	{
		r = 255;
		g = 255;
		b = 255;
	}
	else if (density <= 1) // blue
	{
		r = 0;
		g = 0;
		b = 255;
	}
	else if (density <= 2) // lighter blue
	{
		r = 0;
		g = 128;
		b = 255;
	}
	else if (density <= 3) // light blue
	{
		r = 0;
		g = 255;
		b = 255;
	}
	else if (density <= 4) // light green
	{
		r = 0;
		g = 255;
		b = 128;
	}
	else if (density <= 5) // green
	{
		r = 0;
		g = 255;
		b = 0;
	}
	else if (density <= 6) // yellow
	{
		r = 255;
		g = 255;
		b = 0;
	}
	else if (density <= 7) // orange
	{
		r = 255;
		g = 128;
		b = 0;
	}
	else if (density <= 8) // red
	{
		r = 255;
		g = 0;
		b = 0;
	}
	else if (density <= 9) // dark red
	{
		r = 128;
		g = 0;
		b = 0;
	}
	else // brown
	{
		r = 64;
		g = 0;
		b = 0;
	}

	return Color(r, g, b);
}


