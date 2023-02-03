#ifndef UDATA_HPP
# define UDATA_HPP

#include <iostream>
#include <map>

typedef std::map<uintptr_t, std::string>		Udata;
typedef std::pair<int , std::string> 			Event;
typedef std::map<int, std::string>::iterator	Event_iter;

#endif