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

constexpr int StandartSTRSize = 50;
constexpr int BigSTRSize = 150;

constexpr char read = 'r';
constexpr char modify = 'm';

#pragma warning (disable:4996)

class Manager;
class StateAbstract;
class StateBegin;
class StateRead;
class StateModify;
class StateWrite;
class StateEnd;

class Server
{
	friend class StateAbstract;
	friend class StateBegin;
	friend class StateRead;
	friend class StateModify;
	friend class StateWrite;
	friend class StateEnd;

	int recordEmount;
	employee* records;
	int* record_access;

	FILE* database;
	LPSTR file_name;

	int clientEmount;
	Manager* client_managers;

	void readRecord(int recordNum, employee& result);

	void overrideRecord(int recordNum, employee const& newRecord);

public:

	Server();

	~Server();

	bool CreateDataBase(const char* _filename, int _emount);

	bool CreateClients(int _emount);

	int find_by_number(int number) const;

	void Work();

	void OutputDataBase();
};

class Manager
{
	friend class StateAbstract;
	friend class StateBegin;
	friend class StateRead;
	friend class StateModify;
	friend class StateWrite;
	friend class StateEnd;

	HANDLE communication_pipe;
	_STARTUPINFOA client_StartInf;
	_PROCESS_INFORMATION client_PrInf;
	bool works;

	Server* serv;
	StateAbstract* state;

	LPOVERLAPPED forReadWrite;
	DWORD some_buffer;

public:

	Manager();

	~Manager();

	bool create(int number, Server* st);

	bool is_working() const;

	void Manage();
};

class StateAbstract 
{
protected:
	Manager* owner;
	int buffer;

public:
	explicit StateAbstract(Manager* _m, int _buf = -1): owner(_m), buffer(_buf) {}

	virtual ~StateAbstract() = default;

	virtual void action() = 0;
};

class StateBegin: public StateAbstract
{

public:
	using StateAbstract::StateAbstract;

	~StateBegin() final = default;

	void action() override;
};

class StateRead: public StateAbstract
{

public:
	using StateAbstract::StateAbstract;

	~StateRead() final = default;

	void action() override;
};

class StateModify : public StateAbstract
{

public:
	using StateAbstract::StateAbstract;

	~StateModify() final = default;

	void action() override;
};

class StateWrite : public StateAbstract
{

public:
	using StateAbstract::StateAbstract;

	~StateWrite() final = default;

	void action() override;
};

class StateEnd : public StateAbstract
{

public:
	using StateAbstract::StateAbstract;

	~StateEnd() final = default;

	void action() override;
};