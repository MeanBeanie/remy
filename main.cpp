#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <filesystem>

namespace fs = std::filesystem;

enum INSTRUCTIONS {
	BUILD = 0,
	SRCS,
	FLAGS,
	NAME,
	INCLUDE,
	LIBS,
	PKGS,
	INSTRUCTION_COUNT
};

int main(){
	if(!fs::exists("./.howtobuild")){
		std::cout << "No .howtobuild in this directory" << std::endl;
		return 0;
	}
	std::ifstream file(".howtobuild");
	std::vector<std::string> FILE_LINES;
	std::string str;

	std::map<int, int> BUILD_TARGETS;
	std::string GLOBALS[INSTRUCTION_COUNT];

	int index = 0;
	while(std::getline(file, str)){
		if(str[0] == '['){
			std::string ident = str.substr(1);
			ident.pop_back();
			if(BUILD_TARGETS.contains(index)){
				std::cout << "Found duplicate of target " << ident << std::endl;
				return 1;
			}
			BUILD_TARGETS[index] = FILE_LINES.size();
			index++;
		}
		else if(BUILD_TARGETS.size() == 0 && str[0] == '$'){
			int end = str.find_first_of(':');
			std::string ident = str.substr(1, end-1);
			if(ident == "BUILD"){
				GLOBALS[BUILD] = str.substr(end+1);
			}
			else if(ident == "SRCS"){
				GLOBALS[SRCS] = str.substr(end+1);
			}
			else if(ident == "FLAGS"){
				GLOBALS[FLAGS] = str.substr(end+1);
			}
			else if(ident == "NAME"){
				GLOBALS[NAME] = str.substr(end+1);
			}
			else if(ident == "INCLUDE"){
				GLOBALS[INCLUDE] = str.substr(end+1);
			}
			else if(ident == "LIBS"){
				GLOBALS[LIBS] = str.substr(end+1);
			}
			else if(ident == "PKGS"){
				GLOBALS[PKGS] = str.substr(end+1);
			}
		}
		FILE_LINES.push_back(str);
	}
	str.clear(); // `str` will return...
	file.close();

	std::cout << "Found " << BUILD_TARGETS.size() << " build targets:\n";
	if(BUILD_TARGETS.size() > 15){
		std::cout << "Too many build targets to list, skipping to selection" << std::endl;
		goto getSelection;
	}

	for(auto& [key,val] : BUILD_TARGETS){
		str = FILE_LINES[val];
		str.pop_back();
		str = str.substr(1);
		std::cout << "(" << key+1 << ") " << str << std::endl;
		index++;
	}
	str.clear();

getSelection:
	index = -1;
	while(index == -1){
		std::cout << "Select target from [1-" << BUILD_TARGETS.size() << "]: ";
		std::getline(std::cin, str);
		try {
			if(str.empty() || str == "q"){
				index = -2;
				break;
			}
			std::stoi(str);
		} catch(...) {
			continue;
		}
		index = std::stoi(str);
	}
	if(index == -2){
		return 0;
	}
	str.clear();

	std::string BUILD_INSTRUCTIONS[INSTRUCTION_COUNT];
	BUILD_INSTRUCTIONS[NAME] = FILE_LINES[BUILD_TARGETS[index-1]].substr(1);
	BUILD_INSTRUCTIONS[NAME].pop_back();
	for(int i = BUILD_TARGETS[index-1]+1; i < FILE_LINES.size(); i++){
		if(FILE_LINES[i][0] == '['){
			break;
		}
		else if(FILE_LINES[i][0] == '$'){
			int end = FILE_LINES[i].find_first_of(':');
			str = FILE_LINES[i].substr(1, end-1);
			if(str == "BUILD"){
				BUILD_INSTRUCTIONS[BUILD] = str.substr(end+1);
			}
			else if(str == "SRCS"){
				BUILD_INSTRUCTIONS[SRCS] = str.substr(end+1);
			}
			else if(str == "FLAGS"){
				BUILD_INSTRUCTIONS[FLAGS] = str.substr(end+1);
			}
			else if(str == "NAME"){
				BUILD_INSTRUCTIONS[NAME] = str.substr(end+1);
			}
			else if(str == "INCLUDE"){
				BUILD_INSTRUCTIONS[INCLUDE] = str.substr(end+1);
			}
			else if(str == "LIBS"){
				BUILD_INSTRUCTIONS[LIBS] = str.substr(end+1);
			}
			else if(str == "PKGS"){
				BUILD_INSTRUCTIONS[PKGS] = str.substr(end+1);
			}
		}
	}

	std::string finalInstructions[INSTRUCTION_COUNT];
	finalInstructions[BUILD] = BUILD_INSTRUCTIONS[BUILD].empty() ? GLOBALS[BUILD] : BUILD_INSTRUCTIONS[BUILD];
	finalInstructions[NAME] = BUILD_INSTRUCTIONS[NAME].empty() ? GLOBALS[NAME] : BUILD_INSTRUCTIONS[NAME];
	finalInstructions[SRCS] = GLOBALS[SRCS] + " " + BUILD_INSTRUCTIONS[SRCS];
	finalInstructions[FLAGS] = GLOBALS[FLAGS] + " " + BUILD_INSTRUCTIONS[FLAGS];
	finalInstructions[INCLUDE] = GLOBALS[INCLUDE] + " " + BUILD_INSTRUCTIONS[INCLUDE];
	finalInstructions[LIBS] = GLOBALS[LIBS] + " " + BUILD_INSTRUCTIONS[LIBS];
	finalInstructions[PKGS] = GLOBALS[PKGS] + " " + BUILD_INSTRUCTIONS[PKGS];

	std::string finalCommand = finalInstructions[BUILD] + " " + finalInstructions[SRCS] + " -o \"" + finalInstructions[NAME] + "\" " + finalInstructions[FLAGS] + " " + finalInstructions[INCLUDE] + " " + finalInstructions[LIBS];
	if(!finalInstructions[PKGS].empty() && finalInstructions[PKGS] != " "){ // b/c the added space when setting up `finalInstructions`
		finalCommand = finalCommand + " $(pkg-config " + finalInstructions[PKGS] + ")";
	}
	std::cout << "Building with " << finalCommand << std::endl;
	std::system(finalCommand.c_str());

	return 0;
}
