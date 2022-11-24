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
constexpr int EndTime = 250;//

constexpr int eployeeSize = 24 + 4; // struct_size + len for name


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

	HANDLE* record_locks = new HANDLE[studentEmount];
	HANDLE* continue_work = new HANDLE[clientEmount];
	HANDLE* end_work = new HANDLE[clientEmount];
	HANDLE* communication_pipes = new HANDLE[clientEmount];

	LPSTR* names_record_locks = new LPSTR[studentEmount];
	LPSTR* names_continue_work = new LPSTR[clientEmount];
	LPSTR* names_end_work = new LPSTR[clientEmount];

	LPSTR* pipe_names = new LPSTR[clientEmount];
	for (int i = 0; i < clientEmount; i++)
	{
		LPSTR clientNum = new char[StandartSTRSize];
		itoa(i + 1, clientNum, 10);
		pipe_names[i] = new char[50];
		strcpy(pipe_names[i], "\\\\.\\pipe\\communication_pipe_");
		strcat(pipe_names[i], clientNum);

		delete[] clientNum;

		communication_pipes[i] = CreateNamedPipeA(pipe_names[i], 
			PIPE_ACCESS_INBOUND, PIPE_TYPE_MESSAGE | PIPE_WAIT, 
			1, 0, 0, INFINITE, nullptr);

		if (communication_pipes[i] == INVALID_HANDLE_VALUE)
		{
			printf("Creation of the named pipe failed.");
			{
				delete[] file_name;

				delete[] names_end_work;
				delete[] names_continue_work;
				delete[] names_record_locks;

				delete[] end_work;
				delete[] continue_work;
				delete[] record_locks;
			}
			return 0;
		}
	}

	// Events initialization 
	for (int i = 0; i < clientEmount; i++)
	{
		LPSTR clientNum = new char[StandartSTRSize];
		itoa(i + 1, clientNum, 10);

		names_continue_work[i] = new char[StandartSTRSize];
		strcpy(names_continue_work[i], "continue_work_");
		strcat(names_continue_work[i], clientNum);
		names_end_work[i] = new char[StandartSTRSize];
		strcpy(names_end_work[i], "end_work_");
		strcat(names_end_work[i], clientNum);

		delete[] clientNum;

		continue_work[i] = CreateEventA(nullptr, FALSE, FALSE, names_continue_work[i]);
		end_work[i] = CreateEventA(nullptr, FALSE, FALSE, names_end_work[i]);

		if (continue_work[i] == nullptr || end_work[i] == nullptr)
		{
			printf("Wrong HANDLEs of sync objects");
			// Чистка
			{
				delete[] names_end_work;
				delete[] names_continue_work;
				delete[] names_record_locks;

				delete[] end_work;
				delete[] continue_work;
				delete[] record_locks;
			}
			return 0;
		}
	}

	for (int i = 0; i < studentEmount; i++)
	{
		LPSTR recordNum = new char[StandartSTRSize];
		itoa(i + 1, recordNum, 10);

		names_record_locks[i] = new char[StandartSTRSize];
		strcpy(names_record_locks[i], "record_lock_");
		strcat(names_record_locks[i], recordNum);

		record_locks[i] = CreateMutexA(nullptr, FALSE, names_record_locks[i]);

		if (record_locks[i] == nullptr)
		{
			printf("Wrong HANDLEs of sync objects");
			// Чистка
			{
				delete[] names_end_work;
				delete[] names_continue_work;
				delete[] names_record_locks;

				delete[] end_work;
				delete[] continue_work;
				delete[] record_locks;
			}
			return 0;
		}
	}

	// Student records creation

	fopen_s(&file_bin, file_name, "r+b");
	for (int i = 0; i < studentEmount; i++)
	{
		employee emp;
		emp.input();
		emp.output_file(file_bin);
	}
	fclose(file_bin);

	fopen_s(&file_bin, file_name, "r+b");
	for (int i = 0; i < studentEmount; i++)
	{
		employee emp;
		emp.input_file(file_bin);
		emp.output();
	}
	fclose(file_bin);

	// Senders work
	LPSTR SenderAllocation = new char[StandartSTRSize];
	strcpy(SenderAllocation, "Sender.exe");

	LPSTR str_record_locks = new char[BigSTRSize];
	itoa(studentEmount, str_record_locks, 10); // argv[5]
	for (int j = 0; j < studentEmount; j++)
	{
		strcat(str_record_locks, " ");
		strcat(str_record_locks, names_record_locks[j]); // argv[6] -> argv[6 + studentEmount - 1]
	}

	LPSTR* data = new LPSTR[clientEmount];
	for (int i = 0; i < clientEmount; i++)
	{
		LPSTR clientNum = new char[StandartSTRSize];
		itoa(i + 1, clientNum, 10);		

		data[i] = new char[BigSTRSize];
		strcpy(data[i], SenderAllocation); // argv[1]
		strcat(data[i], " ");
		strcat(data[i], file_name); // argv[1]
		strcat(data[i], " ");
		strcat(data[i], names_continue_work[i]); // argv[2]
		strcat(data[i], " ");
		strcat(data[i], names_end_work[i]); // argv[3]
		strcat(data[i], " ");
		strcat(data[i], clientNum); // argv[4]

		strcat(data[i], " ");
		strcat(data[i], str_record_locks); // argv[5] && argv[6] -> argv[6 + studentEmount - 1]
	}

	_STARTUPINFOA* senders_StartInf = new _STARTUPINFOA[clientEmount];
	_PROCESS_INFORMATION* senders_PrInf = new _PROCESS_INFORMATION[clientEmount];

	for (int i = 0; i < clientEmount; i++)
	{
		ZeroMemory(&senders_StartInf[i], sizeof(_STARTUPINFOW));
		senders_StartInf[i].cb = sizeof(_STARTUPINFOW);
	}

	// создаем Senders
	for (int i = 0; i < clientEmount; i++)
	{
		if (!CreateProcessA(nullptr, data[i], nullptr, nullptr, FALSE,
			CREATE_NEW_CONSOLE, nullptr, nullptr, &senders_StartInf[i], &senders_PrInf[i]))
		{
			printf("The Sender %d is not created. Process stopping...\n", (i + 1));
			// Чистка
			{
				delete[] names_end_work;
				delete[] names_continue_work;
				delete[] names_record_locks;

				delete[] end_work;
				delete[] continue_work;
				delete[] record_locks;

				delete[] data;

				delete[] senders_StartInf;
				delete[] senders_PrInf;
			}// Чистка
			return 0;
		}
		else
		{
			printf("The Client %d is runned.\n", (i + 1));
		}
	}

	int action = 0;
	int* end_of_threads = new int[clientEmount];
	for (int i = 0; i < clientEmount; ++i)
	{
		end_of_threads[i] = 1; // proof of thread not closed (needed further)
	}


	// from here there is no god
	while (!MyFunctions::all_zero(end_of_threads, clientEmount))
	{
		for (int i = 0; i < sendersEmount; ++i)
		{
			if (end_of_threads[i] == 1)
			{
				WaitForSingleObject(senders_ready[i], INFINITE);
				if (WaitForSingleObject(end_work[i], SmallSleepTime) != WAIT_TIMEOUT)
				{
					end_of_threads[i] = 0;
					CloseHandle(senders_PrInf[i].hThread);
					CloseHandle(senders_PrInf[i].hProcess);
				}
			}
		}

		printf("<1> to read messages, <other> to end work : \n");
		scanf_s("%d", &action);

		if (action == 1)
		{
			fopen_s(&file_bin, file_name, "rb");

			char* readed_message = new char[StandartSTRSize];
			printf("Messages: \n");

			while (WaitForSingleObject(senders_counter, SmallSleepTime) != WAIT_TIMEOUT)
			{
				MyFunctions::readMessage(file_bin, readed_message);
				printf("%s\n", readed_message);
			}

			delete[] readed_message;
			fclose(file_bin);

			ReleaseSemaphore(string_counter, stringsEmount, nullptr);
			for (int i = 0; i < sendersEmount; i++)
			{
				SetEvent(continue_work[i]);
			}
		}

		if (action != 1)
		{
			printf("There are no more working senders, process stopping...");
			for (int i = 0; i < sendersEmount; i++)
			{
				if (end_of_threads[i] == 1)
				{
					SetEvent(continue_work[i]);
					Sleep(SmallSleepTime);
					SetEvent(end_work[i]);
					Sleep(SmallSleepTime);
					CloseHandle(senders_PrInf[i].hThread);
					CloseHandle(senders_PrInf[i].hProcess);
				}
			}
			break;
		}

		fopen_s(&file_bin, file_name, "wb");
		fclose(file_bin);

		printf("Now waiting for messages\n");
		Sleep(EndTime);
	}
	// Чистка
	{
		delete[] names_end_work;
		delete[] names_continue_work;
		delete[] names_senders_ready;

		delete[] end_work;
		delete[] continue_work;
		delete[] senders_ready;

		delete[] data;

		delete[] senders_StartInf;
		delete[] senders_PrInf;
	}
	return 0;
}