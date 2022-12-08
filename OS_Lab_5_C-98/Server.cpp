#include "../Employee.h"
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

constexpr int StandartSTRSize = 50;
constexpr int BigSTRSize = 1000;
constexpr int MaxSynchoTime = 50;//
constexpr int SmallSleepTime = 10;//
constexpr int EndTime = 250;
constexpr char read = 'r';
constexpr char modify = 'm';
constexpr char new_record = 'n';


/*
Написать программу для передачи сообщений между процессами через общий файл.
Программа включает один процесс Receiver и несколько процессов Sender.
Процессы Sender посылают сообщения процессу Receiver

remember, no STD...
*/

int main(int argc, char* argv[])
{
	errno_t error_file;
	FILE* file_bin;
	// Input for Creator

	LPSTR file_name = new char[StandartSTRSize];
	printf("input name of binary file (it will be created): ");
	scanf_s("%s", file_name, StandartSTRSize - 1);

	error_file = fopen_s(&file_bin, file_name, "wb");
	if (error_file != 0)
	{
		delete[] file_name;
		printf("Error opening file");
		return 0;
	}
	fclose(file_bin); // just file creation (and check of input)

	int studentEmount = 0;
	printf("input number of records in .bin file: ");
	scanf_s("%d", &studentEmount);

	int clientEmount = 0;
	printf("input number of client processes: ");
	scanf_s("%d", &clientEmount);

	HANDLE* communication_pipes = new HANDLE[clientEmount];
	HANDLE* answers = new HANDLE[clientEmount];

	LPSTR* pipe_names = new LPSTR[clientEmount];
	LPSTR* answer_names = new LPSTR[clientEmount];
	for (int i = 0; i < clientEmount; i++)
	{
		LPSTR clientNum = new char[StandartSTRSize];
		itoa(i + 1, clientNum, 10);

		pipe_names[i] = new char[StandartSTRSize];
		strcpy(pipe_names[i], "\\\\.\\pipe\\communication_pipe_");
		strcat(pipe_names[i], clientNum);

		answer_names[i] = new char[StandartSTRSize];
		strcpy(answer_names[i], "answer_");
		strcat(answer_names[i], clientNum);

		delete[] clientNum;

		communication_pipes[i] = CreateNamedPipeA(pipe_names[i], 
			PIPE_ACCESS_INBOUND, PIPE_TYPE_MESSAGE, 
			1, 0, 0, INFINITE, nullptr);

		if (communication_pipes[i] == INVALID_HANDLE_VALUE)
		{
			printf("Creation of the named pipe failed.");
			{
				delete[] file_name;
			}
			return 0;
		}

		answers[i] = CreateEventA(nullptr, FALSE, FALSE, answer_names[i]);

		if (answers[i] == INVALID_HANDLE_VALUE)
		{
			printf("Creation of the event failed.");
			{
				delete[] file_name;
			}
			return 0;
		}
	}

	// Student records creation

	employee* records = new employee[studentEmount];

	fopen_s(&file_bin, file_name, "r+b");
	for (int i = 0; i < studentEmount; i++)
	{
		records[i].input();
		records[i].output_file_bin(file_bin);
		records[i].output();
	}

	fclose(file_bin);

	// CLients work
	LPSTR ClientAllocation = new char[StandartSTRSize];
	strcpy(ClientAllocation, "Client.exe");

	LPSTR* data = new LPSTR[clientEmount];
	for (int i = 0; i < clientEmount; i++)
	{
		LPSTR clientNum = new char[StandartSTRSize];
		itoa(i + 1, clientNum, 10);		

		data[i] = new char[BigSTRSize];
		strcpy(data[i], ClientAllocation); // argv[0]
		strcat(data[i], " ");
		strcat(data[i], clientNum); // argv[1]
		strcat(data[i], " ");
		strcat(data[i], pipe_names[i]); // argv[2]
		strcat(data[i], " ");
		strcat(data[i], answer_names[i]); // argv[3]

		delete[] clientNum;
	}

	_STARTUPINFOA* clients_StartInf = new _STARTUPINFOA[clientEmount];
	_PROCESS_INFORMATION* clients_PrInf = new _PROCESS_INFORMATION[clientEmount];

	for (int i = 0; i < clientEmount; i++)
	{
		ZeroMemory(&clients_StartInf[i], sizeof(_STARTUPINFOW));
		clients_StartInf[i].cb = sizeof(_STARTUPINFOW);
	}

	// создаем Senders
	for (int i = 0; i < clientEmount; i++)
	{
		if (!CreateProcessA(nullptr, data[i], nullptr, nullptr, FALSE,
			CREATE_NEW_CONSOLE, nullptr, nullptr, &clients_StartInf[i], &clients_PrInf[i]))
		{
			printf("The Client %d is not created. Server stopping...\n", (i + 1));
			// Чистка
			{
				delete[] file_name;
				delete[] data;

				delete[] clients_StartInf;
				delete[] clients_PrInf;
			}// Чистка
			return 0;
		}
		else
		{
			printf("The Client %d is runned.\n", (i + 1));
		}
	}

	bool* end_of_threads = new bool[clientEmount];
	for (int i = 0; i < clientEmount; ++i)
	{
		end_of_threads[i] = true; // proof of thread not closed (needed further)
	}
	// from here there is no god

	bool* record_access = new bool[studentEmount];
	for (int i = 0; i < studentEmount; i++)
	{
		record_access[i] = true;
	}

	int* modify_indexes = new int[clientEmount];
	for (int i = 0; i < clientEmount; ++i)
	{
		modify_indexes[i] = -1;
	}

	char action;
	int employee_num;
	employee new_one;

	while (!MyFunctions::all_zero(end_of_threads, clientEmount))
	{
		for (int i = 0; i < clientEmount; ++i)
		{
			if (end_of_threads[i] == 1)
			{
				if (modify_indexes[i] == -1)
				{
					ReadFile(communication_pipes[i], &action, sizeof(char), nullptr, nullptr);
				}
				else
				{
					action = modify;
				}
				if (action == read || action == modify)
				{
					int index;
					if (modify_indexes[i] == -1)
					{
						ReadFile(communication_pipes[i], &employee_num, sizeof(int), nullptr, nullptr);
						index = MyFunctions::find_by_number(records, studentEmount, employee_num);
					}
					else
					{
						index = modify_indexes[i];
					}

					if (action == modify && record_access[index])
					{
						record_access[index] = false;
						WriteFile(communication_pipes[i], &records[index], sizeof(employee), nullptr, nullptr);
						modify_indexes[i] = index;
					}
					else if (action == modify)
					{
						continue;
					}
				}
				else
				{
					ReadFile(communication_pipes[i], &new_one, sizeof(employee), nullptr, nullptr);

					records[modify_indexes[i]] = new_one;

					record_access[modify_indexes[i]] = true;
					modify_indexes[i] = -1;
				}




			}
		}
	}
	// Чистка
	{
		delete[] data;

	}

	Sleep(EndTime);
	return 0;
}