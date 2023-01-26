#pragma once

#include <iostream>

static void exit_with_perror(const std::string& msg)
{
	system("clear");
	std::cerr << msg << std::endl;
	exit(EXIT_FAILURE);
}