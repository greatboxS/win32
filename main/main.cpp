#include "stdio.h"
#include <string.h>
#include "lib/d_lib.h"
#include <Windows.h>
#include <signal.h>

__declspec(dllimport) int local_var;

DWORD ChildThreadHandler(void* parameter);
void SignalHandler(int);

bool CreateChildProcess(PROCESS_INFORMATION* info, char* argv)
{
	DWORD flags = CREATE_SUSPENDED | CREATE_NEW_CONSOLE | HIGH_PRIORITY_CLASS | CREATE_BREAKAWAY_FROM_JOB;
	memset(info, 0, sizeof(PROCESS_INFORMATION));
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	// Start the child process. 
	if (!CreateProcess(NULL, argv, NULL, NULL, FALSE, flags, NULL, NULL, &si, info))
	{
		printf("CreateProcess failed (%d).\n", GetLastError());
		return false;
	}

	printf("Create process successed\n");
	return true;
}

static struct PROCESS_WRAPPER {
	HANDLE JOB_OBJECT_HANDLE;
	bool IS_CHILD_PROCESS;
	PROCESS_INFORMATION INFO[5];
}PROCESS;

static struct CHILD_PROCESS_WRAPPER {
	struct THREAD_HANDLE
	{
		DWORD ID[5];
		HANDLE HANDLE[5];
	}THREAD;

}CHILD_PROCESS;


int main(int argc, char const* argv[])
{
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobExtendedLimitInfo;
	BOOL jobContained = 0;
	char cmd[512];

	if (argc <= 1) exit(-1);

	if (strstr(argv[argc - 1], "child_process") != NULL)
		PROCESS.IS_CHILD_PROCESS = true;

	signal(SIGSEGV, SignalHandler);
	signal(SIGINT, SignalHandler);
	signal(SIGABRT, SignalHandler);
	signal(SIGBREAK, SignalHandler);

	// Parent only
	if (!PROCESS.IS_CHILD_PROCESS) {
		PROCESS.JOB_OBJECT_HANDLE = CreateJobObject(NULL, NULL);
		if (!PROCESS.JOB_OBJECT_HANDLE)
		{
			printf("Create job object failed\n");
			exit(0);
		}

		memset(cmd, 0, 512);
		ZeroMemory(PROCESS.INFO, sizeof(PROCESS.INFO));

		if (!AssignProcessToJobObject(PROCESS.JOB_OBJECT_HANDLE, GetCurrentProcess())) {
			printf("Can not assign all child process to this Job Object\n");
			CloseHandle(PROCESS.JOB_OBJECT_HANDLE);
			exit(0);
		}

		memset(&jobExtendedLimitInfo, 0, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION));
		memset(&jobExtendedLimitInfo.BasicLimitInformation, 0, sizeof(JOBOBJECT_BASIC_LIMIT_INFORMATION));
		jobExtendedLimitInfo.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE | JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION | JOB_OBJECT_LIMIT_BREAKAWAY_OK;
		if (!SetInformationJobObject(PROCESS.JOB_OBJECT_HANDLE, JobObjectExtendedLimitInformation, &jobExtendedLimitInfo, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION)))
		{
			printf("Failed to set job object information!\n");
			CloseHandle(PROCESS.JOB_OBJECT_HANDLE);
			exit(0);
		}

		for (int i = 0; i < sizeof(PROCESS.INFO) / sizeof(PROCESS_INFORMATION); i++)
		{
			sprintf(cmd, "main.exe %d child_process", i);
			if (CreateChildProcess(&PROCESS.INFO[i], cmd))
			{
				jobContained = 0;
				if (IsProcessInJob(PROCESS.INFO[i].hProcess, PROCESS.JOB_OBJECT_HANDLE, &jobContained))
				{
					if (!jobContained) {
						if (!AssignProcessToJobObject(PROCESS.JOB_OBJECT_HANDLE, PROCESS.INFO[i].hProcess))
						{
							printf("AssignProcessToJobObject failed (%d)\n", GetLastError());
							if (!TerminateProcess(PROCESS.INFO[i].hProcess, 0))
							{
								printf("Can not terminate child process pid: %d\n", PROCESS.INFO[i].dwProcessId);
							}
							else
							{
								printf("Terminated child process pid: %d\n", PROCESS.INFO[i].dwProcessId);
								CloseHandle(PROCESS.INFO[i].hProcess);
							}
						}
						else
						{
							printf("Resume child process\n");
							ResumeThread(PROCESS.INFO[i].hThread);
						}
					}
					else
					{
						printf("Current Job Object contained child process %d\n", PROCESS.INFO[i].dwProcessId);
						ResumeThread(PROCESS.INFO[i].hThread);
					}
				}
			}
		}
		// Wait until child process exits.
		printf("Waiting for child process\n");
		WaitForSingleObject(PROCESS.JOB_OBJECT_HANDLE, INFINITE);
		TerminateJobObject(PROCESS.JOB_OBJECT_HANDLE, 0);
		CloseHandle(PROCESS.JOB_OBJECT_HANDLE);

		for (int i = 0; i < sizeof(PROCESS.INFO) / sizeof(PROCESS_INFORMATION); i++)
		{
			// Close process and thread handles. 
			CloseHandle(PROCESS.INFO[i].hProcess);
			CloseHandle(PROCESS.INFO[i].hThread);
		}
	}
	//Chile Process Only
	else
	{
		for (int i = 0; i < 5; i++)
		{
			CHILD_PROCESS.THREAD.HANDLE[i] = CreateThread(NULL, 1024, ChildThreadHandler, (LPVOID)argv[1], 0, &CHILD_PROCESS.THREAD.ID[i]);
			if (!CHILD_PROCESS.THREAD.HANDLE[i])
			{
				printf("Create Child process thread faield\n");
				exit(0);
			}
			else
			{
				printf("Create Child process thread successed\n");
			}
		}
		WaitForMultipleObjects(5, CHILD_PROCESS.THREAD.HANDLE, true, INFINITE);
		CloseHandle(CHILD_PROCESS.THREAD.HANDLE);
	}
	return 0;
}

DWORD ChildThreadHandler(void* parameter)
{
	static int counter = 0;
	const char* argv = (const char*)parameter;
	printf("Child process Id: %s\n", argv);
	
	while (true)
	{
		//printf("%d\n", ++counter);
	}
	return 0;
}

void SignalHandler(int signal) {

	printf("Receive signal %d\n", signal);
	if (!PROCESS.IS_CHILD_PROCESS)
	{
		printf("Terminate process Job Object\n");
		if (!TerminateJobObject(PROCESS.JOB_OBJECT_HANDLE, 0)) {
			printf("Terminate job object failed\n");
		}
		if (!CloseHandle(PROCESS.JOB_OBJECT_HANDLE)) {
			printf("Close job object handle failed\n");
		}
	}
}
