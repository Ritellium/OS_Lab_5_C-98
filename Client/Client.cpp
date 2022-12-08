#include "../Employee.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <string.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

constexpr int StandartSTRSize = 21;
constexpr int SmallSleepTime = 10;
constexpr int EndTime = 250;
constexpr char read = 'r';
constexpr char modify = 'm';
constexpr char new_record = 'n';

#pragma warning (disable:4996)

int main(int argc, char* argv[])
{
    LPCSTR pipe_name = argv[2];
    LPCSTR answer_name = argv[3];

    printf("I'm Client number %s, type below to read/sent messages\n", argv[1]);

    HANDLE pipe_to_main = CreateFileA(pipe_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    HANDLE answer = OpenEventA(EVENT_ALL_ACCESS, EVENT_MODIFY_STATE, answer_name);

    if (pipe_to_main == INVALID_HANDLE_VALUE || answer == INVALID_HANDLE_VALUE)
    {
        printf("Wrong HANDLE of pipe");

        return 0;
    }

    int action = 0;

    do
    {
        printf("<1> to read recoed, <2> to modify record, <other> to end work: ");
        scanf_s("%d", &action);

        if (action == read)
        {
            
        }
        else if (action == modify)
        {
            
        }
        else
        {
            printf("Process stopping...\n");
            break;
        }
    } while (true);

    Sleep(EndTime);
    return 0;
}
