/*#include <bitset>
#include <cstring>
#include <string>
#include <vector>
#include <optional>
#include <stdexcept>
#include <map>

namespace MemoryFormatReader {
    constexpr inline std::size_t BITS_IN_BYTE = 8;

    struct FormatSpecifier {
        std::size_t numSize;
        char dataType;
        bool isOptional;
    };

    static bool isFormatStringValid(const std::string& format);

    template <typename T>
    static std::optional<T> readNumberFromMemory(const void* memory, const std::size_t numSize)
    {

        if (numSize > BITS_IN_BYTE) {
            throw std::runtime_error("Number size exceeds maximum supported size");
        }

        std::bitset<BITS_IN_BYTE> bits;
        std::memcpy(&bits, memory, (numSize + BITS_IN_BYTE - 1) / BITS_IN_BYTE);

        return static_cast<T>(bits.to_ullong() >> (BITS_IN_BYTE - numSize));
    }

    static std::string readStringFromMemory(const void* memory, const std::size_t numSize) noexcept;

    std::vector<FormatSpecifier> parseFormatString(const std::string& format);

    std::map<const void*, std::string> readDataFromMemory(const void* memory, const std::vector<MemoryFormatReader::FormatSpecifier>& formatSpecifiers, const size_t dataSize);

}*/