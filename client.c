#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "include/common.h"

int main()
{
	clinic_info_t *app;		

	app = clinic_init();
	if(!app) {
		printf("Application is not initiaised with driver\n");
		return -EINVAL;
	}

	return 0;
}
