#include "MemoryScanner.h"
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

#include "MemoryFormatReader.h"

MemoryScannner::MemoryScannner(const wchar_t path[MAX_PATH]) {
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		throw std::runtime_error("Failed to create process snapshot.");
	}

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hSnapshot, &pe32)) {
		CloseHandle(hSnapshot);
		throw std::runtime_error("Failed to retrieve process information.");
	}

	do {
		if (!wcscmp(path, pe32.szExeFile)) {
			HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
			if (process != NULL) {
				this->processes.push_back(process);
			}
			else {
				CloseHandle(hSnapshot);
				throw std::runtime_error("Failed to open process.");
			}
		}
	} while (Process32Next(hSnapshot, &pe32));

	CloseHandle(hSnapshot);

	if (this->processes.empty()) {
		throw std::runtime_error("No process found.");
	}
}


MemoryScannner::~MemoryScannner(void)
{
	for (HANDLE process : this->processes)
	{
		CloseHandle(process);
	}
}

void MemoryScannner::searchProcess(HANDLE process, const int value)
{
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);

	MEMORY_BASIC_INFORMATION memoryInfo;
	unsigned char* address = reinterpret_cast<unsigned char*>(systemInfo.lpMinimumApplicationAddress);

	while (address < systemInfo.lpMaximumApplicationAddress)
	{
		if (VirtualQueryEx(process, address, &memoryInfo, sizeof(memoryInfo)) == sizeof(memoryInfo))
		{
			if (isMemoryRegionValid(memoryInfo))
			{
				searchMemoryRegion(process, memoryInfo, value);
			}
			address += memoryInfo.RegionSize;
		}
		address += systemInfo.dwPageSize;
	}
}

inline bool MemoryScannner::isMemoryRegionValid(const MEMORY_BASIC_INFORMATION& memoryInfo)
{
	return (memoryInfo.State == MEM_COMMIT) && (memoryInfo.Protect == PAGE_READWRITE || memoryInfo.Protect == PAGE_EXECUTE_READWRITE);
}

void MemoryScannner::searchMemoryRegion(HANDLE process, const MEMORY_BASIC_INFORMATION& memoryInfo, const int value)
{
	const size_t bufferSize = static_cast<size_t>(memoryInfo.RegionSize);
	unsigned char* buffer = new unsigned char[bufferSize];
	size_t bytesRead;

	if (ReadProcessMemory(process, memoryInfo.BaseAddress, buffer, bufferSize, &bytesRead) && bytesRead == bufferSize)
	{
		for (size_t i = 0; i < bufferSize - sizeof(value); i++)
		{
			const int* currentValue = reinterpret_cast<int*>(&buffer[i]);
			if (*currentValue == value)
			{
				this->addresses.push_back(reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(memoryInfo.BaseAddress) + i));
			}
		}
	}
	delete[] buffer;
}

void MemoryScannner::filterProcess(HANDLE process, const int value)
{
	std::list<void*> temp;
	int buffer;
	size_t bytes_read;

	for (void* address : this->addresses)
	{
		ReadProcessMemory(process, address, &buffer, sizeof(value), &bytes_read);

		if (value == buffer)
		{
			temp.push_back(address);
		}

	}
}

void MemoryScannner::write(const int value)
{
	for (const HANDLE process : this->processes)
	{
		writeProcess(process, value);
	}
}

void MemoryScannner::writeProcess(const HANDLE process, const int value)
{
	size_t bytes_written;

	for (void* address : this->addresses)
	{
		WriteProcessMemory(process, address, &value, sizeof(value), &bytes_written);
	}
}

void MemoryScannner::search(const int value)
{
	for (HANDLE process : this->processes)
	{
		searchProcess(process, value);
	}
}

void MemoryScannner::filter(const int value)
{
	for (const HANDLE process : this->processes)
	{
		filterProcess(process, value);
	}

}

void MemoryScannner::print(void) const
{
	if (this->addresses.empty())
		return;

	puts("Those are the addresses:");
	for (const void* address : this->addresses)
	{
		std::cout << std::hex << address << '\n';
	}
}
