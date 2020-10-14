#pragma once
#include <string>
#include <iostream>
#include "windows.h"

class ExternalProc
{
public:

	ExternalProc(std::string c,std::string a);

	std::string command;
	std::string arguments;

	int start();
	void kill();
	bool is_alive();

	std::string read_output(int n);
	std::string read_output();
	std::string read_until_end(int t=30);
	int write_stdin(const std::string& inp);

private:
	HANDLE stdin_read = NULL;
	HANDLE stdin_write = NULL;

	HANDLE output_read = NULL;
	HANDLE output_write = NULL;


	PROCESS_INFORMATION procinfo;
	STARTUPINFOA startupinfo;

	int prepare_pipes();

};

