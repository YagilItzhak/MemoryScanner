#include <iostream>
#include <vector>
#include <string_view>
#include <string>
#include <variant>
#include <Windows.h>
#include <TlHelp32.h>


std::vector<DWORD> findPattern(const BYTE* pattern, const std::string_view mask, const address_range_t range)
{
	std::vector<DWORD> addresses;

	const size_t maskLength = mask.length();

	bool found;

	for (DWORD address = range.startAddress; address < range.endAddress - maskLength; address++)
	{
		found = true;

		for (DWORD i = 0; i < maskLength; i++)
		{
			if (!(mask[i] == '?' || pattern[i] == *(BYTE*)(address + 1)))
			{
				found = false;
				break;
			}
		}

		if (found)
		{
			addresses.push_back(address);
		}
	}
	return addresses;
}

void printAddresses(const std::vector<DWORD>& addresses)
{
	if (addresses.empty())
	{
		(void) puts("Pattern not found.");
		return;
	}

	(void) puts("Pattern found at the following addresses:\n");
	for (const DWORD address : addresses)
	{
		std::cout << reinterpret_cast<void*>(address) << '\n';
	}
}

struct address_range_t
{
	DWORD startAddress;
	DWORD endAddress;
};

std::variant<std::string, address_range_t> getProcessMemoryRange(const DWORD processId)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		return "Failed to create process snapshot.";
	}
	MODULEENTRY32 me32;
	me32.dwSize = sizeof(MODULEENTRY32);
	if (!Module32First(hSnapshot, &me32))
	{
		CloseHandle(hSnapshot);
		return "Failed to retrieve module information.";
	}
	address_range_t range;
	range.startAddress = reinterpret_cast<DWORD>(me32.modBaseAddr);
	range.endAddress = range.startAddress + me32.modBaseSize;

	CloseHandle(hSnapshot);
	return range;
}

int main(void)
{
	const DWORD startAddress = 0x00000000;
	const DWORD endAddress = 0xFFFFFFFF;

	int prossesID;

	std::cin >> prossesID;
	
	const auto rangeOrError = getProcessMemoryRange(prossesID);
	if (auto error = std::get_if<std::string>(&rangeOrError))
	{
		perror(error->c_str());
		return EXIT_FAILURE;
	}

	const address_range_t range = std::get<address_range_t>(rangeOrError);

	const BYTE pattern[] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
	const char* mask = "xxxxx";

	std::vector<DWORD> addresses = findPattern(pattern, mask, range);

	printAddresses(addresses);

	return 0;
}