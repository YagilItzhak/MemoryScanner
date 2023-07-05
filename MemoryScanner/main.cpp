#include <iostream>
#include "MemoryScannner.h"

constexpr size_t MAX_PATH_LENGTH = 260;
constexpr size_t MAX_OPERWTION_LENGTH = 6;

int main(void)
{
	wchar_t path[260];
	unsigned long long int value;

	puts("Please enter the prossess you what to follow:");
	std::wcin.getline(path, 260);
	puts("Please enter the value: ");
	std::cin >> value;

	MemoryScannner ms(path);
	ms.search(value);
	
	return 0;
}