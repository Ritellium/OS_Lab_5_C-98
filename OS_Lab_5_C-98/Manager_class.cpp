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

Manager::Manager() : works(false) {}

Manager::~Manager()
{
	CloseHandle(client_PrInf.hThread);
	CloseHandle(client_PrInf.hProcess);
}

bool Manager::create(int number, Server* st)
{
	works = true;
	serv = st;
	state = new StateBegin(this);

	LPSTR clientNum = new char[StandartSTRSize];
	itoa(number, clientNum, 10);

	LPSTR pipe_name = new char[StandartSTRSize];
	strcpy(pipe_name, "\\\\.\\pipe\\communication_pipe_");
	strcat(pipe_name, clientNum);

	communication_pipe = CreateNamedPipeA(pipe_name,
		PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_WAIT,
		2, sizeof(employee) + 4, sizeof(employee) + 4, INFINITE, nullptr);

	if (communication_pipe == INVALID_HANDLE_VALUE)
	{
		delete[] pipe_name;
		delete[] clientNum;
		return false;
	}

	char ClientAllocation[] = "Client.exe";

	LPSTR data = new char[BigSTRSize];
	strcpy(data, ClientAllocation); // argv[0]
	strcat(data, " ");
	strcat(data, clientNum); // argv[1]
	strcat(data, " ");
	strcat(data, pipe_name); // argv[2]

	delete[] clientNum;

	ZeroMemory(&client_StartInf, sizeof(_STARTUPINFOW));
	client_StartInf.cb = sizeof(_STARTUPINFOW);

	if (!CreateProcessA(nullptr, data, nullptr, nullptr, FALSE,
		CREATE_NEW_CONSOLE, nullptr, nullptr, &client_StartInf, &client_PrInf))
	{
		delete[] data;
		delete[] pipe_name;
		return false;
	}

	delete[] data;
	delete[] pipe_name;
	return true;
}

bool Manager::is_working() const
{
	return works;
}

void Manager::Manage()
{
	state->action();
}


void StateBegin::action()
{
	char todo;
	if (ReadFile(owner->communication_pipe, &todo, sizeof(char), &owner->some_buffer, owner->forReadWrite))
	{
		if (todo == read)
		{
			owner->state = new StateRead(owner);
			owner->state->action();
			delete this;
		}
		else if(todo == modify)
		{
			owner->state = new StateModify(owner);
			owner->state->action();
			delete this;
		}
		else
		{
			owner->state = new StateEnd(owner);
			owner->state->action();
			delete this;
		}
	}
}

void StateRead::action()
{
	int num;
	if (ReadFile(owner->communication_pipe, &num, sizeof(char), &owner->some_buffer, owner->forReadWrite))
	{
		employee buf;
		int index = owner->serv->find_by_number(num);

		if (index != -1)
		{
			owner->serv->readRecord(index, buf);
		}
		WriteFile(owner->communication_pipe, &buf, sizeof(employee), &owner->some_buffer, owner->forReadWrite);

		owner->state = new StateBegin(owner);
		owner->state->action();
		delete this;
	}
}

void StateModify::action()
{
	int num;
	if (ReadFile(owner->communication_pipe, &num, sizeof(char), &owner->some_buffer, owner->forReadWrite))
	{
		int index = owner->serv->find_by_number(num);

		if (index != -1)
		{
			buffer = index;
		}
		else
		{
			employee buf;
			WriteFile(owner->communication_pipe, &buf, sizeof(employee), &owner->some_buffer, owner->forReadWrite);
			owner->state = new StateBegin(owner);
			delete this;
		}
	}
	else if (buffer != -1)
	{
		if (owner->serv->record_access[buffer] == -1)
		{
			owner->serv->record_access[buffer] = buffer;

			WriteFile(owner->communication_pipe, &owner->serv->records[buffer], sizeof(employee), &owner->some_buffer, owner->forReadWrite);
			owner->state = new StateWrite(owner, buffer);
			delete this;
		}
	}
}

void StateWrite::action()
{
	employee buf;
	if (ReadFile(owner->communication_pipe, &buf, sizeof(employee), &owner->some_buffer, owner->forReadWrite))
	{
		owner->serv->records[buffer] = buf;
		owner->serv->overrideRecord(buffer, buf);

		owner->serv->record_access[buffer] = -1;

		owner->state = new StateBegin(owner);
		delete this;
	}
}

void StateEnd::action()
{
	owner->works = false;
}