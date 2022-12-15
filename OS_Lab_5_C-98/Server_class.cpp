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
	delete[] records;
}

bool Server::CreateDataBase(const char* _filename, int _emount)
{
	file_name = new char[StandartSTRSize];
	strcpy(file_name, _filename);

	if (fopen_s(&database, _filename, "wb"))
	{
		return false;
	}
	else
	{
		recordEmount = _emount;
		records = new employee[recordEmount];
		record_access = new int[recordEmount];

		for (int i = 0; i < recordEmount; i++)
		{
			records[i].input();
			records[i].output_file_bin(database);

			record_access[i] = -1;
		}
		for (int i = 0; i < recordEmount; i++)
		{
			records[i].output();
		}

		fclose(database);

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

int Server::find_by_number(int number) const
{
	for (int i = 0; i < recordEmount; i++)
	{
		if (records[i].num == number)
		{
			return i;
		}
	}
	return -1;
}

void Server::readRecord(int recordNum, employee& result)
{
	if (fopen_s(&database, file_name, "r+b") == 0)
	{
		fseek(database, recordNum * sizeof(employee), SEEK_SET);
		fread(&result, sizeof(employee), 1, database);
		fclose(database);
	}
}

void Server::overrideRecord(int recordNum, employee const& newRecord)
{
	if (fopen_s(&database, file_name, "r+b") == 0)
	{
		fseek(database, recordNum * sizeof(employee), SEEK_SET);
		fwrite(&newRecord, sizeof(employee), 1, database);
		fclose(database);
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
	} while (someoneWorks);
}

void Server::OutputDataBase()
{
	if (fopen_s(&database, file_name, "r+b") == 0)
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