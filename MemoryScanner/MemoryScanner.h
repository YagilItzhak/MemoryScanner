#pragma once
#include <vector>
#include <list>
#include <Windows.h>
class MemoryScannner
{
public:
	MemoryScannner(const wchar_t path[MAX_PATH]);
	~MemoryScannner(void);

	void search(const int value);
	void filter(const int value);
	void write(const int value);
	void print(void) const;

private:
	inline static bool isMemoryRegionValid(const MEMORY_BASIC_INFORMATION& memoryInfo);

	void searchProcess(HANDLE process, const int value);
	void searchMemoryRegion(HANDLE process, const MEMORY_BASIC_INFORMATION& memoryInfo, const int value);

	void filterProcess(HANDLE process, const int value);

	void writeProcess(const HANDLE process, const int value);

private:
	std::vector<HANDLE> processes;
	std::list<void*> addresses;
};

