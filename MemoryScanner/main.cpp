#include <iostream>
#include "MemoryScannner.h"


int main(void)
{
	wchar_t path[MAX_PATH];
	unsigned long long int value;

	puts("Please enter the prossess you what to follow:");
	std::wcin.getline(path, MAX_PATH);
	puts("Please enter the value: ");
	std::cin >> value;

	MemoryScannner ms(path);
	ms.search(value);
	ms.print();
	
	return 0;
}