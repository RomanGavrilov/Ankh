#include "utils/file-io.hpp"
#include <fstream>
#include <stdexcept>

namespace ankh
{

    std::vector<char> read_binary(const std::string &path)
    {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file: " + path);
        }

        size_t size = static_cast<size_t>(file.tellg());
        std::vector<char> data(size);

        file.seekg(0);
        file.read(data.data(), size);
        file.close();

        return data;
    }

} // namespace ankh
