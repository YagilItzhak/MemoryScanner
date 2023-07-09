#include <bitset>
#include <cstring>
#include <string>
#include <vector>
#include <optional>
#include <stdexcept>
#include <map>

namespace MemoryFormatReader {
    constexpr std::size_t BITS_IN_BYTE = 8;

    struct FormatSpecifier {
        std::size_t numSize;
        char dataType;
        bool isOptional;
    };

    std::vector<FormatSpecifier> parseFormatString(const std::string& format);

    template <typename T>
    std::optional<T> readNumberFromMemory(const void* memory, std::size_t numSize)
    {
        constexpr std::size_t BYTE_SIZE = sizeof(std::byte);

        if (numSize > BITS_IN_BYTE * BYTE_SIZE) {
            throw std::runtime_error("Number size exceeds maximum supported size");
        }

        std::bitset<BITS_IN_BYTE * BYTE_SIZE> bits;
        std::memcpy(&bits, memory, (numSize + BITS_IN_BYTE - 1) / BITS_IN_BYTE);

        return static_cast<T>(bits.to_ullong() >> (BITS_IN_BYTE * BYTE_SIZE - numSize));
    }

    std::string readStringFromMemory(const void* memory, const std::size_t numSize);

    std::map<const void*, std::string> readDataFromMemory(const void* memory, const std::vector<MemoryFormatReader::FormatSpecifier>& formatSpecifiers, const size_t dataSize);

}