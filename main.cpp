#include <iostream>
#include <filesystem>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <unistd.h>

using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;

enum InstructionIndex {
	BUILD = 0,
	SRCS,
	FLAGS,
	LIBS,
	PKGS,
	NAME,
	INCLUDE,
	INSTRUCTION_COUNT
};

int main(){
	std::ifstream file(".howtobuild");
	if(!file.is_open()){
		std::cout << "Failed to find .howtobuild" << std::endl;
		return 1;
	}

	std::vector<std::string> lines;
	std::string temp;
	while(std::getline(file, temp)){
		lines.push_back(temp);
	}
	file.close();

	int filesIndex = -1;
	std::map<std::string, int> projectIndices;

	if(lines[0][0] != '['){
		std::cout << ".howtobuild meant to start with \'[\' but started with \'" << lines[0][0] << "\'" << std::endl;
		return 1;
	}
	else{
		for(int i = 0; i < lines.size(); i++){
			if(lines[i][0] == '['){
				std::string tag = lines[i].substr(1);
				tag.pop_back();
				if(tag == "Files"){
					filesIndex = i;
				}
				else{
					if(projectIndices.contains(tag)){
						std::cout << "Repeat tag \"" << tag << "\" found" << std::endl;
						continue;
					}
					projectIndices[tag] = i;
				}
			}
		}
	}

	// -- FILE HANDLING --

	std::string c;
	if(filesIndex != -1){
		std::vector<std::string> filepaths;

		for(int i = filesIndex+1; i < lines.size(); i++){
			if(lines[i][0] == '['){
				break;
			}
			filepaths.push_back(lines[i]);
			if(!std::filesystem::exists(lines[i])){
				if(lines[i].back() == '/'){
					std::filesystem::create_directory(lines[i]);
				}
				else{
					std::ofstream temp(lines[i]);
					temp << "[Generated by Remy!]";
					temp.close();
				}
			}
		}

		std::vector<std::filesystem::path> invalidPaths;

		for (const auto& dirEntry : recursive_directory_iterator(std::filesystem::current_path())){
			bool exists = false;
			std::filesystem::path relative = std::filesystem::relative(dirEntry.path());
			for(int i = 0; i < filepaths.size(); i++){
				if(relative.generic_string() == filepaths[i]
					|| relative.generic_string()[0] == '.'
					|| projectIndices.contains(relative.generic_string())){
					exists = true;
				}
			}
			
			if(!exists && !dirEntry.is_directory()){
				invalidPaths.push_back(dirEntry.path());
			}
		}

		int size = invalidPaths.size();
		if(size != 0){
			std::cout << "Found " << size << " paths not listed in .howtobuild:\n";
			for(int i = 0; i < invalidPaths.size(); i++){
				std::cout << invalidPaths[i].generic_string() << '\n';
			}
			std::cout << "Free to delete files?[y/N]: ";
			std::getline(std::cin, c);
			if(c == "y"){
				for(int i = 0; i < invalidPaths.size(); i++){
					std::filesystem::remove(invalidPaths[i]);
				}
				std::cout << "Deleted all " << size << " files" << std::endl; 
			}
			else{
				std::cout << "Free to add files to .howtobuild?[Y/n]: ";
				std::getline(std::cin, c);
				if(c != "n"){
					for(int i = 0; i < invalidPaths.size(); i++){
						std::filesystem::path relative = std::filesystem::relative(invalidPaths[i]);
						lines.insert(lines.begin()+filesIndex+1, relative.generic_string());
					}

					std::ofstream temp("./.howtobuild", std::ios::trunc);
					for(int i = 0; i < lines.size(); i++){
						temp << lines[i] << std::endl;
					}
					temp.close();
				}
			}
		}
	}

	// -- BUILD HANDLING --

	if(projectIndices.size() == 0){
		std::cout << "No project instructions found" << std::endl;
		return 1;
	}

	std::cout << "Found " << projectIndices.size() << " build targets:\n";

	std::vector<std::vector<std::string>> BUILD_INSTRUCTIONS;
	BUILD_INSTRUCTIONS.resize(projectIndices.size());
	int index = 0;
	for(auto const& [key, val] : projectIndices){
		std::cout << "[" << index << "] " << key << std::endl;
		BUILD_INSTRUCTIONS[index].resize(INSTRUCTION_COUNT);
		BUILD_INSTRUCTIONS[index][NAME] = key.c_str();
		for(int i = val+1; i < lines.size(); i++){
			if(lines[i][0] == '['){
				break;
			}
			else if(lines[i][0] == '$'){
				int end = lines[i].find(':');
				std::string ident = lines[i].substr(1, end-1);
				if(ident == "BUILD"){
					BUILD_INSTRUCTIONS[index][BUILD] = lines[i].substr(end+1);
				}
				else if(ident == "SRCS"){
					BUILD_INSTRUCTIONS[index][SRCS] = lines[i].substr(end+1);
				}
				else if(ident == "FLAGS"){
					BUILD_INSTRUCTIONS[index][FLAGS] = lines[i].substr(end+1);
				}
				else if(ident == "LIBS"){
					BUILD_INSTRUCTIONS[index][LIBS] = lines[i].substr(end+1);
				}
				else if(ident == "PKGS"){
					BUILD_INSTRUCTIONS[index][PKGS] = lines[i].substr(end+1);
				}
				else if(ident == "NAME"){
					if(lines[i][end+1] == ' '){
						BUILD_INSTRUCTIONS[index][NAME] = lines[i].substr(end+2);
					}
					else{
						BUILD_INSTRUCTIONS[index][NAME] = lines[i].substr(end+1);
					}
				}
			}
		}
		index++;
	}

	int selection = -1;
	c.clear();
	while(selection == -1){
		std::cout << "Which build target would you like?[0-" << projectIndices.size()-1 << "]: ";
		std::getline(std::cin, c);
		try {
			if(c.empty() || c == "q"){
				selection = -2;
			}
			std::stoi(c);
		} catch(...){
			continue;
		}
		if(std::stoi(c) >= 0 && std::stoi(c) < projectIndices.size()){
			selection = std::stoi(c);
		}
	}

	if(selection == -2){
		return 0;
	}

	std::string finalBuild = BUILD_INSTRUCTIONS[selection][BUILD] + ' ' + BUILD_INSTRUCTIONS[selection][SRCS] + " -o \"" + BUILD_INSTRUCTIONS[selection][NAME] + "\" " + BUILD_INSTRUCTIONS[selection][FLAGS] + ' ' + BUILD_INSTRUCTIONS[selection][INCLUDE] + ' ' + BUILD_INSTRUCTIONS[selection][LIBS];
	if(!BUILD_INSTRUCTIONS[selection][PKGS].empty()){
		finalBuild = finalBuild + " $(pkg-config " + BUILD_INSTRUCTIONS[selection][PKGS] + " --cflags --libs)";
	}

	if(c != "n"){
		std::cout << "Running \"" << finalBuild << "\"" << std::endl;
		std::system(finalBuild.c_str());
	}

	return 0;
}
