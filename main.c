#include <stdio.h>
#include "debuglog.h"

int main()
{
	int anint = 10;

	DEBUGLOG_INIT("afile.log");
	DEBUGLOG_LOG(1, "the value is: %d", anint);
	DEBUGLOG_CLOSE();

	return 0;
}
