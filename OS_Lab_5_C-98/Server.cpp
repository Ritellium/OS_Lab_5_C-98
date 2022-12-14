#include "../Employee.h"
#include "Server_class.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <string.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <charconv>
#include <tchar.h>

#pragma warning (disable:4996)

/*
написать программу для управления доступом параллельных процессов к файлу по именованным каналам.

remember, no STD...
*/

int main(int argc, char* argv[])
{
	
	// Input for Creator

	LPSTR file_name = new char[StandartSTRSize];
	printf("input name of binary file (it will be created): ");
	scanf_s("%s", file_name, StandartSTRSize - 1);

	int studentEmount = 0;
	printf("input number of records in .bin file: ");
	scanf_s("%d", &studentEmount);

	Server main_server;
	main_server.CreateDataBase(file_name, studentEmount);

	int clientEmount = 0;
	printf("input number of client processes: ");
	scanf_s("%d", &clientEmount);

	if (!main_server.CreateClients(clientEmount))
	{
		printf("something went wrong :( \n");
		return 0;
	}

	main_server.Work();

	return 0;
}