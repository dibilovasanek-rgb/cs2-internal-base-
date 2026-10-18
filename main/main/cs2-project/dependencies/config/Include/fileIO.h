#pragma once
#include <fstream>
#include <filesystem>

extern std::string ReadFile(std::ofstream& file);
extern bool WriteToFile(std::string buffer, std::ofstream& file);
extern void CloseFile(std::ofstream& file);
extern std::ofstream OpenFile(std::string name, std::ios_base::openmode mode = std::fstream::in | std::fstream::out);
extern std::vector<std::filesystem::path> GetFileList(std::string path);
extern void CreateFolder(std::string path);
extern bool DoesPathExist(std::string path);
