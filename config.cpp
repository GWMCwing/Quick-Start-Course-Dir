#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
class Config {
public:
    int setKeyValue(std::string key, std::string value) {
        if (key == "path") {
            this->setPath(value);
            return 0;
        }
        if (key == "depth") {
            this->setDepth(stoi(value));
            return 0;
        }
        std::cout << "Ignoring: " << key << " and " << value << std::endl;
        return 1;
    }
    const std::string getPath() const { return this->path; }
    const int getDepth() const { return this->depth; }
    const int verify() {
        int rInt = 0;
        std::ifstream test(this->path);
        if (!test) rInt++;
        if (depth <= 0) rInt += 2;
        return rInt;
    }
private:
    Config& setPath(std::string path) {
        this->path = path;
        return *this;
    }
    Config& setDepth(int depth) {
        this->depth = depth;
        return *this;
    }
    std::string path;
    int depth;
};

Config* getConfig() {
    std::fstream config_f("./config.cfg");
    if (!config_f.is_open()) return nullptr;
    std::string line;
    Config* config = new Config();
    while (std::getline(config_f, line)) {
        std::istringstream is_line(line);
        if (is_line.peek() == '#') continue;
        std::string key;
        if (std::getline(is_line, key, ':')) {
            std::string value;
            if (std::getline(is_line, value))
                config->setKeyValue(key, value);
        }
    }
    if (!config->verify()) {
        return nullptr;
    }
    return config;
}