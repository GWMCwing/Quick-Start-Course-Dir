#pragma warning(disable:28020) //The expression '0<=_Param_(1)&&_Param_(1)<=400-1' is not true at this call.
#define NOMINMAX
//
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <shobjidl.h>
#include <windows.h>
#pragma comment(lib, "shell32.lib")
#include <shellapi.h>
// 
#include "config.cpp"
// 
int LevenshteinDistance(std::string s1, std::string s2);
Config* getConfig();

// 
int main() {
    Config* config = getConfig();
    if (!config) return 1;
    // 
    std::cout << config->getPath() << ", " << config->getDepth() << std::endl;
    // 
    delete config;
}

