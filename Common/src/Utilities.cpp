#include "Effie/Utilities.h"
#include <fstream>

using namespace Effie;

std::string Utilities::ReadFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        LOG(ERROR) << "Could not open " << filename;
        throw std::runtime_error("failed to open file!");
    }

    std::string data;

    file.seekg(0, std::ios::end);
    data.reserve(file.tellg());
    file.seekg(0, std::ios::beg);
    data.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    return std::move(data);
}
