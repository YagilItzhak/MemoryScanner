#include "MemoryScannner.h"
#include <Windows.h>
#include <TlHelp32.h>

MemoryScannner::MemoryScannner(const std::wstring_view path) 
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(MODULEENTRY32);
	HANDLE process;
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		perror("Failed to create process snapshot.");
		exit(EXIT_FAILURE);
	}
	if (!Process32First(hSnapshot, &pe32))
	{
		perror("Failed to retrieve module information.");
		exit(EXIT_FAILURE);
	}

	do {

		if (path == pe32.szExeFile)
		{
			process = OpenProcess(PROCESS_ALL_ACCESS, true, pe32.th32ProcessID);

			this->processes.push_back(process);
		}
	} while (Process32Next(hSnapshot, &pe32));

	CloseHandle(hSnapshot);
}

MemoryScannner::~MemoryScannner(void)
{
	for (HANDLE process : this->processes)
	{
		CloseHandle(process);
	}
}

void MemoryScannner::searchProcess(HANDLE process, const unsigned long long int value)
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

bool MemoryScannner::isMemoryRegionValid(const MEMORY_BASIC_INFORMATION& memoryInfo)
{
	return (memoryInfo.State == MEM_COMMIT) && (memoryInfo.Protect == PAGE_READWRITE || memoryInfo.Protect == PAGE_EXECUTE_READWRITE);
}

void MemoryScannner::searchMemoryRegion(HANDLE process, const MEMORY_BASIC_INFORMATION& memoryInfo, const unsigned long long int value)
{
	const size_t bufferSize = static_cast<size_t>(memoryInfo.RegionSize);
	unsigned char* buffer = new unsigned char[bufferSize];
	size_t bytesRead;

	if (ReadProcessMemory(process, memoryInfo.BaseAddress, buffer, bufferSize, &bytesRead) && bytesRead == bufferSize)
	{
		for (size_t i = 0; i < bufferSize - sizeof(value); i++)
		{
			if (*reinterpret_cast<unsigned long long int*>(buffer + i) == value)
			{
				this->addresses.push_back(reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(memoryInfo.BaseAddress) + i));
			}
		}
	}
	delete buffer;
}

void MemoryScannner::search(const unsigned long long int value)
{
	for (HANDLE process : this->processes)
	{
		searchProcess(process, value);
	}
}
