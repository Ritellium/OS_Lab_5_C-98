#include "../Employee.h"
#include "Client_class.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <string.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

Client::Client()
{
	pipe_to_server = nullptr;
}

bool Client::Create(LPCSTR _pipe)
{
	pipe_to_server = CreateFileA(_pipe, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (pipe_to_server == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	return true;
}

void Client::Work()
{
	char action = '0';

	do
	{
		printf("<1> to read record, <2> to modify record, <other> to end work: ");
		scanf_s("%c", &action);

		if (action == read)
		{
			ActionRead();
		}
		else if (action == modify)
		{
			ActionModify();
		}
	} while (action == read || action == modify);

	ActionEnd();
}

void Client::ActionRead()
{
	int number;

	printf("Enter emloyee's number to read record: ");
	scanf_s("%d", &number);
	WriteFile(pipe_to_server, &read, sizeof(int), &some_buffer, nullptr);
	WriteFile(pipe_to_server, &number, sizeof(int), &some_buffer, nullptr);

	employee buf;
	ReadFile(pipe_to_server, &buf, sizeof(employee), &some_buffer, nullptr);

	if (buf.num != 0)
	{
		printf("Here is the record: \n");
		buf.output();
	}
	else
	{
		printf("What's a pity, no such record!\n");
	}

	char c;
	printf("Enter any char to complete operation: ");
	scanf_s("%c", &c);
}

void Client::ActionModify()
{
	int number;

	printf("Enter emloyee's number to read record: ");
	scanf_s("%d", &number);

	WriteFile(pipe_to_server, &modify, sizeof(int), &some_buffer, nullptr);
	WriteFile(pipe_to_server, &number, sizeof(int), &some_buffer, nullptr);

	printf("Waiting for server responce... (try to complete other clients) \n");

	employee buf;
	ReadFile(pipe_to_server, &buf, sizeof(employee), &some_buffer, nullptr);

	if (buf.num != 0)
	{
		printf("Here is the record: \n");
		buf.output();

		printf("Enter a record to replace: \n");
		buf.input();

		WriteFile(pipe_to_server, &buf, sizeof(employee), &some_buffer, nullptr);
	}
	else
	{
		printf("What's a pity, no such record!\n");
	}

	char c;
	printf("Enter any char to complete operation: ");
	scanf_s("%c", &c);
}

void Client::ActionEnd()
{
	char c;
	printf("Enter any char to exit program: ");
	scanf_s("%c", &c);

	WriteFile(pipe_to_server, &end, sizeof(int), &some_buffer, nullptr);
}