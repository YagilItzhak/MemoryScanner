#include <iostream>
#include "MemoryScanner.h"

int askForValue(void)
{
	int value;
	fputs("Please enter the value: ", stdout);
	std::cin >> value;
	return value;
}


int main(void)
{
	wchar_t path[MAX_PATH];
	int value;
	char option;

	fputs("Please enter the program you what to follow: ", stdout);
	std::wcin.getline(path, MAX_PATH);

	MemoryScannner ms(path);
	

	value = askForValue();

	getchar();

	ms.search(value);
	ms.print();

	do
	{
		puts("Please enter u to enter an updated value");
		puts("             w to write a value");
		puts("             e to exit");

		option = getchar();

		if (option == 'u')
		{
			value = askForValue();
			ms.filter(value);
			ms.print();
		}
		else if (option == 'w')
		{
			value = askForValue();
			ms.write(value);
		}


	} while (option != 'e');
	
	return 0;
}