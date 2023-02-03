#ifndef UDATA_HPP
# define UDATA_HPP

#include <iostream>
#include <map>
#include <sys/_types/_uintptr_t.h>

typedef std::map<uintptr_t, std::string>			Udata;
typedef std::pair<uintptr_t, std::string> 			Event;
typedef std::map<uintptr_t, std::string>::iterator	Udata_iter;

#endif