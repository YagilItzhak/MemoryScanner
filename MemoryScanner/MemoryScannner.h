#pragma once
#include <string_view>
#include <vector>
#include <Windows.h>
class MemoryScannner
{
public:
	MemoryScannner(const std::wstring_view path);
	~MemoryScannner(void);

	void search(const unsigned long long int value);

private:
	static bool isMemoryRegionValid(const MEMORY_BASIC_INFORMATION& memoryInfo);
	void searchProcess(HANDLE process, const unsigned long long int value);
	void searchMemoryRegion(HANDLE process, const MEMORY_BASIC_INFORMATION& memoryInfo, const unsigned long long int value);

private:
	std::vector<HANDLE> processes;
	std::vector<void*> addresses;
};

