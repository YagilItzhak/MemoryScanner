#include "MemoryFormatReader.h"


std::vector<MemoryFormatReader::FormatSpecifier> MemoryFormatReader::parseFormatString(const std::string& format)
{
	std::vector<FormatSpecifier> formatSpecifiers;

	size_t formatIndex = 0;
	const size_t formatSize = format.size();

	// Loop until all format specifiers are parsed
	while (formatIndex < formatSize)
	{
		FormatSpecifier specifier;

		specifier.numSize = 0;
		specifier.isOptional = false;

		if (format.at(formatIndex) == '?') {
			specifier.isOptional = true;
			formatIndex++;
		}

		while (format[formatIndex] != ':' && formatIndex < formatSize) {
			specifier.numSize = (specifier.numSize * 10) + (format[formatIndex] - '0');
			formatIndex++;
		}

		// Check if we have reached the end of the format
		if (formatIndex >= formatSize) {
			throw std::runtime_error("Insufficient format specification");
		}

		// Read the data type from format
		if (format[formatIndex] == ':') {
			formatIndex++;
			specifier.dataType = format[formatIndex];
		}
		else {
			throw std::runtime_error("Invalid format specification");
		}

		formatSpecifiers.push_back(specifier);

		formatIndex++;
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
            }
                break;
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

    // Check if the actual data size matches the expected data size
    if (bitIndex != expectedDataSize) {
        throw std::runtime_error("Unmatched data in the memory block");
    }

    return matches;
}
