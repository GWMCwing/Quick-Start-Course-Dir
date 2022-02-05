// // TODO outputCount should be inside config.json
//TODO check for outdated fileStructure.json base on timeUnix
//TODO colored output
//TODO error catching
#pragma warning(disable:28020) //The expression '0<=_Param_(1)&&_Param_(1)<=400-1' is not true at this call.
#define NOMINMAX
//
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <cctype>
#include <string>
#include <time.h>
#include "json.hpp"
#include <shobjidl.h>
#include <windows.h>
#pragma comment(lib, "shell32.lib")
#include <shellapi.h>
//
using json = nlohmann::json;
// defining functions
std::string getExeDirectory();
json getConfig();
json getFileStructureJson(const std::string dirPath, const json& configJson, std::vector<std::string>& courseNameVector);
//
void sendUsage();
//
json generateFileStructureJson(const std::string dirPath, const json& configJson, std::vector<std::string>& courseNameVector);
void createFileStructureFile(const std::string dirPath, const json& configJson, std::vector<std::string>& courseNameVector);
void appendKeyJsonDepthTwo_Dir(const std::string dirPath, const json& configJson, const std::string depthOneKey, json& courseJson, std::vector<std::string>& courseNameVector);
void appendCurrentSemester(const std::string dirPath, const json& configJson, const std::string& depthOneKey, json& targetJson);
//
bool checkAlias(const std::string name, const std::string alias, const json& configJson);
int LevenshteinDistance(std::string s1, std::string s2);
std::vector<int> generateDistanceVector(const std::string& s1, const std::vector<std::string>& courseNameVector);
std::vector<int> getBestMatchDistanceIndex(const std::vector<int>& distanceVector, const int& returnCount);
std::vector<std::string> getBestMatchCourseNameStringVector(const std::vector<int>& bestMatchDistanceIndex, const std::vector<std::string>& courseNameVector);
std::vector<std::string> getCourseNameVector(const json& fileStructureJson);
//
HINSTANCE openFolder(const std::string& path);
int printUserSelectIndex(const std::vector<std::string>& choiceVector);
void printToConsole(const std::string text, const int& type);
const enum class PrintType {
    Verbose,
    Normal,
    Warning,
    Error,
    Success
};

// main
int main(const int argc, const char** argv) {
    const json configJson = getConfig();
    const std::string dirPath = configJson["dirPath"];
    const std::string LASTEDIT = "5/2/2022-15:19";
    std::vector<std::string> courseNameVector = {};
    const json fileStructureJson = getFileStructureJson(dirPath, configJson, courseNameVector);
    std::string courseName, course, courseId;
    const int fuzzyReturnCount = 5;
    //
    std::cout << "Last Edit: " << LASTEDIT << std::endl;
    if (argc < 2) {
        sendUsage();
        system("pause");
        return 0;
    }
    // json fileStructureJson = getFileStructureJson(dirPath);
    courseName = argv[1];
    if (argc > 2) {
        courseName += argv[2];
    }
    if (argc > 3) {
        std::cout << "Ignoring: ";
        for (int i = 3;i < argc;++i) {
            std::cout << argv[i] << ", ";
        }
        std::cout << "\n";
    }
    //
    if (checkAlias(courseName, "generateStructure", configJson)) {
        createFileStructureFile(dirPath, configJson, courseNameVector);
        return 0;
    } else if (checkAlias(courseName, "currentSemester", configJson)) {
        const std::string targetPath = fileStructureJson["currentSemester"]["path"];
        std::cout << "Opening: " << targetPath << std::endl;
        HINSTANCE err_code = openFolder(targetPath);
        return 0;
    } else {
        std::cout << "Alias not found" << "\nFuzzy matching: " << courseName << std::endl;
        const size_t courseIdIndex = courseName.find_first_of("0123456789");
        courseNameVector = getCourseNameVector(fileStructureJson);
        int keyDepth;
        std::string searchTarget;
        //
        if (courseIdIndex == std::string::npos) {
            std::cout << "Course Id or Year number not found\n";
            course = courseName;
            std::cout << "Searching for: " << course << std::endl;
            searchTarget = course;
            keyDepth = 1;
        } else {
            course = courseName.substr(0, courseIdIndex);
            courseId = courseName.substr(courseIdIndex);
            std::transform(course.begin(), course.end(), course.begin(), ::toupper);
            // std::cout << "Searching Course: " << course << " Id: " << courseId << "\n";
            searchTarget = course + " " + courseId;
            keyDepth = 2;
        }
        // get best match course name
        std::cout << "generating distance vector\n";
        std::vector<int> distanceVector = generateDistanceVector(searchTarget, courseNameVector);
        std::cout << "generating best match distance index vector\n";
        std::vector<int> bestMatchDistanceIndex = getBestMatchDistanceIndex(distanceVector, fuzzyReturnCount);
        // get json object
        std::cout << "generating best match course name string vector\n";
        std::vector<std::string> bestMatchCourseNameStringVector = getBestMatchCourseNameStringVector(bestMatchDistanceIndex, courseNameVector);
        int selectedIndex;
        if (distanceVector[bestMatchDistanceIndex[0]] == 0) {
            selectedIndex = 0;
        } else {
            // output to select
            selectedIndex = printUserSelectIndex(bestMatchCourseNameStringVector);
        }
        //
        if (selectedIndex == -1) {
            std::cout << "Selected unknown\nExiting...\n";
            return 0;
        } else {
            std::string targetKey = bestMatchCourseNameStringVector[selectedIndex];
            std::cout << "Selected: " << targetKey << std::endl;
            const size_t spaceIndex = targetKey.find_first_of(" ");
            // std::cout << "spaceIndex: " << spaceIndex << std::endl;
            std::string rootKey = "course";
            if (targetKey._Starts_with("Year") || targetKey._Starts_with("YEAR")) {
                rootKey = "year";
            }
            if (spaceIndex == std::string::npos) {
                // 1 key
                std::string targetPath = fileStructureJson[rootKey][targetKey]["path"];
                std::cout << "Opening: " << targetPath << std::endl;
                HINSTANCE err_codePath = openFolder(targetPath);
                return 0;
            } else {
                // 2 key
                std::string key1 = targetKey.substr(0, spaceIndex);
                std::string key2 = targetKey.substr(spaceIndex + 1);
                std::cout << "key1: " << key1 << " key2: " << key2 << std::endl;
                std::string targetPath = fileStructureJson[rootKey][key1][key2]["path"];
                std::cout << "Opening: " << targetPath << std::endl;
                HINSTANCE err_codePath = openFolder(targetPath);
                return 0;
            }
        }
    }

    //

    return 0;
}
// get the directory of the exe file
std::string getExeDirectory() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos);
}
json getConfig() {
    json config;
    std::ifstream i(getExeDirectory() + "\\config.json");
    i >> config;
    return config;
}
// get the file structure json
json getFileStructureJson(const std::string dirPath, const json& configJson, std::vector<std::string>& courseNameVector) {
    json fileStructureJson;
    std::ifstream i(getExeDirectory() + "\\fileStructure.json");
    if (!i.good()) {
        i.clear();
        createFileStructureFile(dirPath, configJson, courseNameVector);
        i.open(getExeDirectory() + "\\fileStructure.json");
    }
    i >> fileStructureJson;
    i.close();
    return fileStructureJson;
}
// send usage 
void sendUsage() {
    std::cout << "Usage: " << "\n"
        << "\tNo args\t\t\t Display Help" << "\n"
        << "\t--g\t\t\t GenerateStructure" << "\n"
        << "\t--c\t\t\t CurrentSemester" << "\n"
        << "\t[Course/Year]\t\t Enable Fuzzy Search" << "\n"
        << "'--' is optional prefix" << "\n";
}
// generate the fileStructureJson Obj
json generateFileStructureJson(const std::string dirPath, const json& configJson, std::vector<std::string>& courseNameVector) {
    //
    std::cout << "Generating fileStructureJson\n";
    json fileStructureJson;
    fileStructureJson["timeUnix"] = std::time(nullptr);
    // append course
    std::cout << "Appending Course File Structure Json\n";
    json courseJson;
    appendKeyJsonDepthTwo_Dir(dirPath, configJson, "byCourse", courseJson, courseNameVector);
    fileStructureJson["course"] = courseJson;
    // append year
    std::cout << "Appending Year File Structure Json\n";
    json yearJson;
    appendKeyJsonDepthTwo_Dir(dirPath, configJson, "byYear", yearJson, courseNameVector);
    fileStructureJson["year"] = yearJson;
    // append Current Semester
    std::cout << "Appending Current Year File Structure Json\n";
    json currentSemesterJson;
    appendCurrentSemester(dirPath, configJson, "currentSemester", currentSemesterJson);
    fileStructureJson["currentSemester"] = currentSemesterJson;
    // return the json
    fileStructureJson["courseNameVector"] = courseNameVector;
    std::cout << "fileStructureJson Finalized\n";
    return fileStructureJson;
}
// create fileStructure.json
void createFileStructureFile(const std::string dirPath, const json& configJson, std::vector<std::string>& courseNameVector) {
    json fileStructureJson = generateFileStructureJson(dirPath, configJson, courseNameVector);
    const std::string path = getExeDirectory() + "\\fileStructure.json";
    std::ofstream o(path);
    o << std::setw(4) << fileStructureJson << std::endl;
    o.close();
    std::cout << "File fileStructure.json Created at \"" << path << "\"\n";
}
// append the key json (depth two)
void appendKeyJsonDepthTwo_Dir(const std::string dirPath, const json& configJson, const std::string depthOneKey, json& targetJson, std::vector<std::string>& courseNameVector) {
    const std::string depthOneDir = dirPath + static_cast<std::string>(configJson[depthOneKey]["path"]);
    std::cout << "Dir: \"" << depthOneDir << "\"" << std::endl;
    //
    for (const auto& depthOneEntry : std::filesystem::directory_iterator(depthOneDir)) {
        const std::string depthOnePath = depthOneEntry.path().string();
        std::string depthOnePathName = depthOneEntry.path().filename().string();
        if ((!depthOneEntry.is_directory() && !depthOneEntry.is_symlink()) ||
            depthOnePathName._Starts_with(".")) {
            continue;
        }
        //
        json depthOneJson;
        for (const auto& depthTwoEntry : std::filesystem::directory_iterator(depthOnePath)) {
            std::string depthTwoPathName = depthTwoEntry.path().filename().string();
            const std::string depthTwoPathString = depthTwoEntry.path().string();
            std::cout << "Adding Dir: \"" << depthTwoPathString << "\"\n";
            //
            if ((!depthTwoEntry.is_directory() &&
                !depthTwoEntry.is_symlink()) ||
                depthTwoPathName._Starts_with(".")) {
                continue;
            }
            //
            json depthTwoJson;
            depthTwoJson["path"] = depthTwoPathString;
            depthOneJson[depthTwoPathName] = depthTwoJson;
            courseNameVector.push_back(depthOnePathName + " " + depthTwoPathName);
        }
        courseNameVector.push_back(depthOnePathName);
        //
        std::string depthOneName = depthOneEntry.path().filename().string();
        depthOneJson["path"] = depthOnePath;
        targetJson[depthOneName] = depthOneJson;
    }
    //
}
// append the current semester
void appendCurrentSemester(const std::string dirPath, const json& configJson, const std::string& depthOneKey, json& targetJson) {
    const std::string depthOneDir = dirPath + static_cast<std::string>(configJson[depthOneKey]["path"]) + ".lnk";
    targetJson["path"] = depthOneDir;
}
// check if name is an alias in config
bool checkAlias(const std::string name, const std::string alias, const json& configJson) {
    auto aliasArr = configJson["inputName"][alias]["alias"];
    std::cout << "Checking alias: " << name << " : " << alias << "\n";
    for (std::string aliasName : aliasArr) {
        if (aliasName == name) {
            std::cout << "Found alias: " << aliasName << " : " << alias << std::endl;
            return true;
        }
    }
    return false;
}
// https://en.wikipedia.org/wiki/Levenshtein_distance
int LevenshteinDistance(std::string s1, std::string s2) {
    // initialize 0 matrix
    transform(s1.begin(), s1.end(), s1.begin(), ::toupper);
    transform(s2.begin(), s2.end(), s2.begin(), ::toupper);
    std::vector<std::vector<int>> distance = std::vector<std::vector<int>>(s1.size() + 1, std::vector<int>(s2.size() + 1, 0));
    const int iSize = s1.size() + 1;
    const int s1Size = s1.size();
    const int jSize = s2.size() + 1;
    const int s2Size = s2.size();
    // initialize first row and column
    for (int i = 0; i < iSize; ++i) {
        distance[i][0] = i;
    }
    for (int j = 0; j < jSize; ++j) {
        distance[0][j] = j;
    }
    // calculate the distance
    for (int j = 0; j < s2Size;++j) {
        for (int i = 0; i < s1Size;++i) {
            int cost;
            if (s1[i] == s2[j]) {
                cost = 0;
            } else {
                cost = 1;
            }
            distance[i + 1][j + 1] = std::min(
                std::min(distance[i][j + 1] + 1, distance[i + 1][j] + 1),
                distance[i][j] + cost
            );
        }
    }
    return distance[iSize - 1][jSize - 1];
}
//
std::vector<int> generateDistanceVector(const std::string& s1, const std::vector<std::string>& courseNameVector) {
    std::vector<int> distanceVector;
    for (const auto& courseName : courseNameVector) {
        distanceVector.push_back(LevenshteinDistance(s1, courseName));
    }
    return distanceVector;
}
//
std::vector<int> getBestMatchDistanceIndex(const std::vector<int>& distanceVector, const int& returnCount) {
    std::vector<int> bestMatchDistanceVector(returnCount, INT16_MAX);
    std::vector<int> bestMatchDistanceIndex(returnCount, -1);
    const int distanceVectorSize = distanceVector.size();
    int maxMinDistance = INT16_MAX;
    //
    for (int i = 0; i < distanceVectorSize;++i) {
        const int currentDistance = distanceVector[i];
        if (currentDistance < maxMinDistance) {
            for (int j = 0; j < returnCount; ++j) {
                if (currentDistance < bestMatchDistanceVector[j]) {
                    if (j == returnCount - 1) {
                        maxMinDistance = currentDistance;
                    }
                    //
                    for (int k = returnCount - 1; k > j; --k) {
                        bestMatchDistanceVector[k] = bestMatchDistanceVector[k - 1];
                        bestMatchDistanceIndex[k] = bestMatchDistanceIndex[k - 1];
                    }
                    bestMatchDistanceVector[j] = currentDistance;
                    bestMatchDistanceIndex[j] = i;
                    break;
                }
            }
        }
    }
    return bestMatchDistanceIndex;
}
std::vector<std::string> getBestMatchCourseNameStringVector(const std::vector<int>& bestMatchDistanceIndex, const std::vector<std::string>& courseNameVector) {
    std::vector<std::string> bestMatchCourseNameJsonVector;
    for (const auto& index : bestMatchDistanceIndex) {
        if (index == -1) {
            continue;
        }
        bestMatchCourseNameJsonVector.push_back(courseNameVector[index]);
    }
    return bestMatchCourseNameJsonVector;
}
std::vector<std::string> getCourseNameVector(const json& fileStructureJson) {
    std::vector<std::string> courseNameVector;
    for (auto& elem : fileStructureJson["courseNameVector"]) {
        courseNameVector.push_back(elem);
    }
    return courseNameVector;
}
// 
HINSTANCE openFolder(const std::string& path) {
    return ShellExecuteA(NULL, "open", path.c_str(), NULL, NULL, SW_SHOWDEFAULT);
}

int printUserSelectIndex(const std::vector<std::string>& choiceVector) {
    int userSelectIndex = -1;
    std::cout << "\nPlease select the folder you want to open: \n";
    for (int i = 0; i < choiceVector.size(); ++i) {
        std::cout << i + 1 << ": " << choiceVector[i] << "\n";
    }
    std::cout << "\n";
    do {
        std::cout << "Please input the index: ";
        std::cin >> userSelectIndex;
        if (std::cin.fail() || userSelectIndex == -1) exit(0);
    } while (userSelectIndex < 1 || userSelectIndex > choiceVector.size());
    return userSelectIndex - 1;
}
//
void printToConsole(const std::string text, const int& type) {
    //TODO
}