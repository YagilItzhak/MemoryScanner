#include "MemoryScannner.h"
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

MemoryScannner::MemoryScannner(const wchar_t path[MAX_PATH])
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
	if (Process32First(hSnapshot, &pe32) == ERROR_NO_MORE_FILES)
	{
		perror("Failed to retrieve module information.");
		exit(EXIT_FAILURE);
	}

	do {
		if (!wcscmp(path, pe32.szExeFile))
		{
			process = OpenProcess(PROCESS_ALL_ACCESS, true, pe32.th32ProcessID);

			this->processes.push_back(process);
		}
	} while (Process32Next(hSnapshot, &pe32));

	CloseHandle(hSnapshot);

	if (this->processes.empty())
	{
		perror("No process found");
		exit(EXIT_FAILURE);
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

bool MemoryScannner::isMemoryRegionValid(const MEMORY_BASIC_INFORMATION& memoryInfo)
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
			if (*reinterpret_cast<unsigned long long int*>(buffer + i) == value)
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
	puts("Those are the addresses:");
	for (const void* address : this->addresses)
	{
		std::cout << std::hex << address << '\n';
	}
}
