#include <ctime>
#include <iostream>
#include <exception>
#include <map>
#include <vector>
#include "color.hpp"
#include <stdint.h> // cstdint c++11

typedef std::pair<uintptr_t, std::string> Udata;
typedef std::map<uintptr_t, std::string> Udata_s;

class test353Exception : public std::exception {
	public:
		const char*		what(void) const throw() { return "353"; };
};

class test366Exception : public std::exception {
	public:
		const char*		what(void) const throw() { return "366"; };
};

class Sender {
	public:
	static Udata good_thing_mssage(uintptr_t& sock) {
		std::string msg = "socket: 5 nickname :good";
		Udata ret = std::make_pair(sock, msg);
		return ret;
	}
	static Udata error_353_message(uintptr_t& sock) {
		std::string msg = "socket: 5 353 nickname :error NO.. bla bla";
		Udata ret = std::make_pair(sock, msg);
		return ret;
	};
	static Udata error_366_message(uintptr_t& sock) {
		std::string msg = "socket: 5 366 nickname : what?!!";
		Udata ret = std::make_pair(sock, msg);
		return ret;
	};
};


void	error_message(uintptr_t& sock, int error_code) {
	switch (error_code) {
		case 353:
			throw Sender::error_353_message(sock);
			break;
		case 366:
			throw Sender::error_366_message(sock);
			break;
	}
}

Udata	printException(uintptr_t& sock, int error_code) {
	Udata u;

	// select_channel과 같은 형태
	if (!error_code) {
		u = Sender::good_thing_mssage(sock);
		return u;
	}
	error_message(sock, error_code);
	return u;
}

int main()
{
    std::time_t result = std::time(NULL);
	std::stringstream	ss;
	ss << result;
    std::cout << ss.str() << " seconds since the Epoch\n";

	Udata_s Us;

	try {
		uintptr_t user_sock1 = 3;
		Udata u = printException(user_sock1, 0);
		std::cout << BOLDGREEN << "[Udata good]" << RESET << std::endl
				  << "> fd  = " << u.first << std::endl
				  << "> msg = " << u.second << std::endl << std::endl;
		Us.insert(u);

		std::cout << BOLDRED << "[Udata error]" << std::endl << RESET;
		uintptr_t user_sock2 = 5;
		printException(user_sock2, 366);
		std::cout << "here?" << std::endl;

	} catch (const Udata& u2){
		Us.insert(u2);
		std::cout << "> fd   = " << u2.first << std::endl
				  << "> msg  = " << u2.second << std::endl;
	}
	std::cout << BOLDRED << "=========Pair show=======" << std::endl << RESET;
	for (Udata_s::const_iterator it(Us.begin())
			; it != Us.end(); ++it) {
		std::cout << "pair" << "<\t" << it->first << "\t,\t"<< it->second << "\t>"<< std::endl;
	}

	return 0;
}


/* 대략적인 형태
while(std::getline...) {
	try{
		함수 포인터 부분
	}
	catch (Udata) {
	}
}
*/
