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

Server::Server() : recordEmount(0), clientEmount(0) {}

Server::~Server()
{
	delete[] client_managers;
	delete[] file_name;
	delete[] record_access;
}

void Server::InputDataBaseConsole(int _emount)
{
	recordEmount = _emount;
	record_access = new int[recordEmount];

	employee buffer;

	for (int i = 0; i < recordEmount; i++)
	{
		buffer.input();
		buffer.output_file_bin(database);

		record_access[i] = -1;
	}
}

void Server::InputDataBaseFile(LPCSTR filename)
{
	FILE* input;
	fopen_s(&input, filename, "r");
	employee buffer;

	int _emount = 0;
	fscanf_s(input, "%d", &_emount);

	recordEmount = _emount;
	record_access = new int[recordEmount];

	for (int i = 0; i < recordEmount; i++)
	{
		buffer.input_file_txt(input);
		buffer.output_file_bin(database);

		record_access[i] = -1;
	}

	fclose(input);
}

bool Server::CreateDataBase(const char* _filename, int _emount, char mode, LPCSTR input_filename)
{
	file_name = new char[StandartSTRSize];
	strcpy(file_name, _filename);

	if (fopen_s(&database, _filename, "a+b"))
	{
		return false;
	}
	else
	{
		if (mode == console)
		{
			InputDataBaseConsole(_emount);
		}
		else if (mode == file)
		{
			InputDataBaseFile(input_filename);
		}
		else
		{
			fclose(database);
		}

		fclose(database);

		OutputDataBase();

		return true;
	}
}

bool Server::CreateClients(int _emount)
{
	clientEmount = _emount;
	client_managers = new Manager[clientEmount];

	for (int i = 0; i < clientEmount; i++)
	{
		if (!client_managers[i].create(i + 1, this))
		{
			return false;
		}
	}

	return true;
}

int Server::find_by_number(int number)
{
	if (fopen_s(&database, file_name, "rb") == 0)
	{
		employee buf;
		for (int i = 0; i < recordEmount; i++)
		{
			buf.input_file_bin(database); 
			if (buf.num == number)
			{
				return i;
			}
		}

		fclose(database);
	}
	return -1;
}

void Server::readRecord(int recordNum, employee& result)
{
	if (fopen_s(&database, file_name, "rb") == 0)
	{
		fseek(database, recordNum * sizeof(employee), SEEK_SET);
		fread(&result, sizeof(employee), 1, database);
		fclose(database);
	}
	else
	{
		std::cout << "error code " << GetLastError() << std::endl;
	}
}

void Server::overrideRecord(int recordNum, employee const& newRecord)
{
	if (fopen_s(&database, file_name, "r+b") == 0)
	{
		fseek(database, recordNum * sizeof(employee), SEEK_SET);
		fwrite(&newRecord, sizeof(employee), 1, database);
		std::cout << "record " << recordNum << " modified" << std::endl;
		fclose(database);
	}
	else
	{
		std::cout << "error code " << GetLastError() << std::endl;
	}
}

void Server::Work()
{
	bool someoneWorks;
	do
	{
		someoneWorks = false;
		for (int i = 0; i < clientEmount; i++)
		{
			if (client_managers[i].is_working())
			{
				client_managers[i].Manage();
				someoneWorks = true;
			}
		}		

		Sleep(ServerCycleTime);
	} while (someoneWorks);
}

void Server::OutputDataBase()
{
	printf("Database now: \n");
	if (fopen_s(&database, file_name, "rb") == 0)
	{
		employee buf;
		for (int i = 0; i < recordEmount; i++)
		{
			buf.input_file_bin(database);
			buf.output();
		}

		fclose(database);
	}
}