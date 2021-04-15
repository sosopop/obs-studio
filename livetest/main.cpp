#include "mainwindow.h"

#include <QApplication>
#include <obs.h>
#ifdef _WIN32
#include <windows.h>
#include <filesystem>
#else
#include <signal.h>
#include <pthread.h>
#endif

#include <iostream>
using namespace std;

static void do_log(int log_level, const char *msg, va_list args, void *param)
{
	char bla[4096];
	vsnprintf(bla, 4095, msg, args);

	printf("%s\n", bla);

	UNUSED_PARAMETER(param);
	UNUSED_PARAMETER(log_level);
}


#define MAX_CRASH_REPORT_SIZE (150 * 1024)

#ifdef _WIN32

static void load_debug_privilege(void)
{
	const DWORD flags = TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY;
	TOKEN_PRIVILEGES tp;
	HANDLE token;
	LUID val;

	if (!OpenProcessToken(GetCurrentProcess(), flags, &token)) {
		return;
	}

	if (!!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &val)) {
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = val;
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		AdjustTokenPrivileges(token, false, &tp, sizeof(tp), NULL,
				      NULL);
	}

	if (!!LookupPrivilegeValue(NULL, SE_INC_BASE_PRIORITY_NAME, &val)) {
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = val;
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		if (!AdjustTokenPrivileges(token, false, &tp, sizeof(tp), NULL,
					   NULL)) {
			blog(LOG_INFO, "Could not set privilege to "
				       "increase GPU priority");
		}
	}

	CloseHandle(token);
}
#endif

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	SetErrorMode(SEM_FAILCRITICALERRORS);
	load_debug_privilege();

	base_set_log_handler(do_log, nullptr);
	obs_set_cmdline_args(argc, argv);

	if (!obs_startup("en-US", nullptr, nullptr))
		throw "Couldn't create Test";
	obs_load_all_modules();

	int ret = 0;
	{
		MainWindow w;
		w.show();
		ret = a.exec();
	}
	obs_shutdown();
	return ret;
}
