#include "app.h"

#include <climits>
#include <iostream>

int main()
{
	App app;
	app.init();
	app.exec();
	app.deinit();

	return 0;
}
