#pragma once
#include <string>
#include <string.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

#define ERROR_FILE_OPEN -3

struct employee
{
	int num;
	char name[10];
	double hours;

	employee();

	employee(int new_num, char new_name[], double new_hours);

	employee(employee& emp);

	~employee() = default;

	void input();

	void output();

	void output_file(FILE* file);

	void input_file(FILE* file);

	void output_file_txt(FILE* file);

	void input_file_txt(FILE* file);

	employee& operator = (employee& sourse);
};

namespace MyFunctions
{
	void sendMessage(FILE* file, const char* message);

	void readMessage(FILE* file, char* readHere);

	bool all_zero(const int* threads, int emount);
}