#include "externalproc.h"

ExternalProc::ExternalProc(std::string c, std::string a) : command{ c }, arguments{ a }{};

int ExternalProc::start()
{
	prepare_pipes();

	BOOL ret;
	std::string concat = command + " " + arguments;

	startupinfo.cb = sizeof(STARTUPINFO);
	//use combined stderr + stdout
	startupinfo.hStdError = output_write;
	startupinfo.hStdOutput = output_write;
	startupinfo.hStdInput = stdin_read;
	startupinfo.dwFlags |= STARTF_USESTDHANDLES;

	ret = CreateProcessA(
		NULL,
		(LPSTR)concat.c_str(),
		NULL,
		NULL,
		TRUE,
		CREATE_NO_WINDOW,
		NULL,
		NULL,
		&startupinfo,
		&procinfo);

	return ret;

}

void ExternalProc::kill()
{ 
	TerminateProcess(procinfo.hProcess, 0);
}

bool ExternalProc::is_alive()
{
	DWORD exitcode;
	bool ret = GetExitCodeProcess(procinfo.hProcess, &exitcode);

	if (exitcode == STILL_ACTIVE) {
		return true;
	}
	else {
		return false;
	}

	
}

std::string ExternalProc::read_output(int n)
{
	int ret;
	DWORD bytesRead = 0;
	char* buffer = new char[n];
	std::string output;
	ret = ReadFile(output_read, buffer, n, &bytesRead, NULL);

	output =  std::string(&buffer[0], bytesRead);
	delete[] buffer;

	return output;

}

std::string ExternalProc::read_output()
{
	//read all bytes available, nonblocking
	DWORD available = 0;

	PeekNamedPipe(output_read, NULL, 0, NULL, &available, NULL);

	if (available <= 0) {
		return std::string("");
	}
	else {
		return read_output(available);
	}

	
}

std::string ExternalProc::read_until_end(int t)
{
	// read until process finished or timeout
	DWORD start = GetTickCount64();

	while (true) {

		if (!is_alive()) {
			return read_output();
		}

		auto delta = (GetTickCount64() - start) / 1000;
		if (delta >= t) {
			return read_output()+"\n<TIMEOUT REACHED>";
		}
	}

	return std::string();
}

int ExternalProc::write_stdin(const std::string &inp) {
	DWORD written = 0;
	WriteFile(stdin_write, inp.c_str(), inp.size(), &written, NULL);
	return written;
}

int ExternalProc::prepare_pipes()
{
	std::cout << "preparing pipes\n";

	SECURITY_ATTRIBUTES sattr;

	sattr.nLength = sizeof(SECURITY_ATTRIBUTES);
	sattr.bInheritHandle = TRUE;
	sattr.lpSecurityDescriptor = NULL;

	int ret = CreatePipe(&output_read, &output_write, &sattr, 0);

	SetHandleInformation(output_read, HANDLE_FLAG_INHERIT, 0);

	ret = CreatePipe(&stdin_read, &stdin_write, &sattr, 0);

	SetHandleInformation(stdin_write, HANDLE_FLAG_INHERIT, 0);

	return ret;
}
