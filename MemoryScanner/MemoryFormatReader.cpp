/*#include "MemoryFormatReader.h"



bool MemoryFormatReader::isFormatStringValid(const std::string& format)
{
    const size_t formatSize = format.size();
    size_t formatIndex = 0;

    // Skip leading whitespace
    while (formatIndex < formatSize && isspace(format.at(formatIndex)))
    {
        formatIndex++;
    }

    // Loop until all format specifiers are checked
    while (formatIndex < formatSize)
    {
        // Check for optional indicator
        if (format.at(formatIndex) == '?')
        {
            formatIndex++;
        }

        // Check for number component
        while (formatIndex < formatSize && format.at(formatIndex) != '%' && isdigit(format.at(formatIndex)))
        {
            formatIndex++;
        }

        // Check if we have reached the end of the format
        if (formatIndex >= formatSize)
        {
            return false;
        }

        // Check for percent separator
        if (format.at(formatIndex) == '%')
        {
            formatIndex++;
        }
        else
        {
            return false;
        }

        formatIndex++;

        // Skip any remaining characters until the next format specifier or the end of the format
        while (formatIndex < formatSize && format.at(formatIndex) != '?' && format.at(formatIndex) != '%' && !isspace(format.at(formatIndex)))
        {
            formatIndex++;
        }

        // Skip whitespace after the format specifier
        while (formatIndex < formatSize && isspace(format.at(formatIndex)))
        {
            formatIndex++;
        }
    }

    return true;
}


std::vector<MemoryFormatReader::FormatSpecifier> MemoryFormatReader::parseFormatString(const std::string& format)
{
    std::vector<FormatSpecifier> formatSpecifiers;

    std::size_t pos = 0;
    std::size_t len = format.length();

    while (pos < len) {
        if (format[pos] == '%') {
            if (pos + 2 < len && (format[pos + 1] == 'u' || format[pos + 1] == 'i' || format[pos + 1] == 'f')) {
                std::size_t numSize = 0;
                char dataType = format[pos + 1];
                bool isOptional = false;
                std::optional<std::string> value;

                pos += 2;
                while (pos < len && std::isdigit(format[pos])) {
                    numSize = numSize * 10 + (format[pos] - '0');
                    ++pos;
                }

                if (pos < len && format[pos] == '=') {
                    ++pos;
                    std::string val;
                    while (pos < len && std::isdigit(format[pos])) {
                        val += format[pos];
                        ++pos;
                    }
                    value = val;
                }

                if (pos < len && format[pos] == '?') {
                    isOptional = true;
                    ++pos;
                }

                formatSpecifiers.push_back({ numSize, dataType, isOptional, value });
            }
            else {
                throw std::runtime_error("Invalid format string");
            }
        }
        else {
            std::string str;
            while (pos < len && format[pos] != '%') {
                str += format[pos];
                ++pos;
            }
            formatSpecifiers.push_back({ 0, 's', false, str });
        }
    }

    return formatSpecifiers;
}


std::string MemoryFormatReader::readStringFromMemory(const void* memory, const std::size_t numSize)
{
	if (numSize % BITS_IN_BYTE != 0) {
		throw std::runtime_error("String size must be a multiple of 8 bits");
	}

	const std::size_t byteSize = numSize / BITS_IN_BYTE;

	return std::string(static_cast<const char*>(memory), byteSize);
}

std::map<const void*, std::string> MemoryFormatReader::readDataFromMemory(const void* memory, const std::vector<FormatSpecifier>& formatSpecifiers, const size_t dataSize)
{
    std::map<const void*, std::string> matches;

    size_t bitIndex = 0;

    // Calculate the expected data size based on format specifiers
    size_t expectedDataSize = 0;
    for (const auto& specifier : formatSpecifiers)
    {
        expectedDataSize += specifier.numSize;
    }

    // Check if the actual data size matches the expected data size
    if (bitIndex / BITS_IN_BYTE != expectedDataSize / BITS_IN_BYTE) {
        throw std::runtime_error("Unmatched data in the memory block");
    }

    try {
        // Loop through each format specifier
        for (const auto& specifier : formatSpecifiers)
        {
            // Check if the number size exceeds the remaining data size (if not optional)
            if (!specifier.isOptional && bitIndex + specifier.numSize > dataSize)
            {
                throw std::runtime_error("Insufficient data size");
            }

            // Read the number or string from memory if not optional
            if (!specifier.isOptional)
            {
                const std::byte* dataPointer = static_cast<const std::byte*>(memory) + (bitIndex / BITS_IN_BYTE);

                switch (specifier.dataType)
                {
                case 'u':
                {
                    // Unsigned integer
                    std::optional<std::uint64_t> number = readNumberFromMemory<std::uint64_t>(dataPointer, specifier.numSize);
                    if (number.has_value())
                    {
                        matches.emplace(dataPointer, std::to_string(number.value()));
                    }
                    break;
                }
                case 'i':
                {
                    // Signed integer
                    std::optional<std::int64_t> number = readNumberFromMemory<std::int64_t>(dataPointer, specifier.numSize);
                    if (number.has_value())
                    {
                        matches.emplace(dataPointer, std::to_string(number.value()));
                    }
                    break;
                }
                case 'f':
                {
                    // Floating-point number
                    if (specifier.numSize == 32)
                    {
                        std::optional<float> number = readNumberFromMemory<float>(dataPointer, specifier.numSize);
                        if (number.has_value())
                        {
                            matches.emplace(dataPointer, std::to_string(number.value()));
                        }
                    }
                    else if (specifier.numSize == 64)
                    {
                        std::optional<double> number = readNumberFromMemory<double>(dataPointer, specifier.numSize);
                        if (number.has_value())
                        {
                            matches.emplace(dataPointer, std::to_string(number.value()));
                        }
                    }
                    else
                    {
                        throw std::runtime_error("Invalid floating-point number size");
                    }
                    break;
                }
                default:
                {
                    // String
                    const std::string expectedString = specifier.dataType == '?' ? "" : std::string(1, specifier.dataType);
                    const std::string actualString = readStringFromMemory(dataPointer, specifier.numSize);

                    if (actualString == expectedString)
                    {
                        matches.emplace(dataPointer, actualString);
                    }
                    break;
                }
                }
            }

            // Increment the bit index
            bitIndex += specifier.numSize;
        }
    }
    catch (const std::exception& e) {
        // Clean up any resources and re-throw the exception
        matches.clear();
        throw;
    }

    return matches;
}
*/