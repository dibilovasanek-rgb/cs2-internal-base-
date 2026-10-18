#include "fileIO.h"
#include <sstream>

bool DoesPathExist(std::string path)
{
	if (std::filesystem::exists(path))
		return true;
	else
		return false;
}

void CreateFolder(std::string path)
{
	if (!DoesPathExist(path))
		std::filesystem::create_directory(path);
}

std::vector<std::filesystem::path> GetFileList(std::string path)
{
	std::vector<std::filesystem::path> ret;

	for (const auto& entry : std::filesystem::directory_iterator(path))
		ret.push_back(entry.path());

	return ret;
}

std::ofstream OpenFile(std::string name, std::ios_base::openmode mode)
{
	std::ofstream file;
	file.open(name, mode);
	return file;
}

void CloseFile(std::ofstream& file)
{
	if (file)
		file.close();
}

bool WriteToFile(std::string buffer, std::ofstream& file)	
{
	if (!file.is_open())
		return false;
	file << buffer;
	return true;
}

std::string ReadFile(std::ofstream& file)
{
	if (!file.is_open())
		return "";

	file.seekp(0, std::ios_base::beg);

	std::stringstream bffr;
	bffr << file.rdbuf();
	return bffr.str();
}
