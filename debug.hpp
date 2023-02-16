#pragma once

#include "User.hpp"
#include "Channel.hpp"

namespace debug {
	// static void	showUserInfo(std::vector<User>& target) {
	// 	std::cout << YELLOW << "< User list >  " << RESET << std::endl;
	// 	int i(0);
	// 	for (std::vector<User>::iterator it = target.begin(); it != target.end(); ++it, ++i) {
	// 		std::cout << YELLOW << " ㄴ " << i + 1 << ". " << RESET
	// 				  << BOLDWHITE << (*it).nickname_ << RESET << std::endl;
	// 	}
	// 	std::cout << YELLOW << "------------------" << std::endl
	// 						<< "Total : " << i << " (users)"
	// 			  << RESET << std::endl << std::endl;
	// }
	static void	showUsers(std::vector<User>& target) {
		std::cout << YELLOW << "< User list >  " << RESET << std::endl;
		int i(0);
		for (std::vector<User>::iterator it = target.begin(); it != target.end(); ++it, ++i) {
			std::cout << YELLOW << " ㄴ " << i + 1 << ". " << RESET
					  << BOLDWHITE << (*it).nickname_ << "(" << it->username_ << ", " << it->mode_ << ", " << it->unused_ << ", " << it->realname_ << ")" << RESET << std::endl;
		}
		std::cout << YELLOW << "------------------" << std::endl
							<< "Total : " << i << " (users)"
				  << RESET << std::endl << std::endl;
	}
	// static void	showChannels(std::vector<Channel>& target) {
	// 	std::cout << YELLOW << "< Channel list >" << RESET << std::endl;
	// 	int i(0);
	// 	for (std::vector<Channel>::iterator it = target.begin(); it != target.end(); ++it, ++i) {
	// 		std::cout << YELLOW << " ㄴ " << i + 1 << ". " << RESET
	// 				  << BOLDWHITE << (*it).get_name() << RESET << std::endl;
	// 	}
	// 	std::cout << YELLOW << "------------------" << std::endl
	// 						<< "Total : " << i << " (channels)"
	// 			  << RESET << std::endl << std::endl;
	// }
}
