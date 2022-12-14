#include "../Employee.h"
#include "Client_class.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <string.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

#pragma warning (disable:4996)

int main(int argc, char* argv[])
{
    Client client;
    client.Create(argv[2]);

    printf("I'm Client number %s, type below to read/sent messages\n", argv[1]);

    

    return 0;
}
