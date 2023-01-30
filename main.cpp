#include "Server.hpp"

#include "Channel.hpp"
#include <iostream>

void	showUsers(std::vector<user>& target) {
	std::cout << YELLOW << "(begin) " << RESET << " ➜ ";
	for (std::vector<user>::iterator it = target.begin(); it != target.end(); ++it) {
		std::cout << (*it).nickname_ << " ➜ ";
	}
	std::cout << YELLOW << "(end)" << RESET << std::endl;
}

void	showChannels(std::vector<Chan>& target) {
	std::cout << YELLOW << "(begin) " << RESET << " ➜ ";
	for (std::vector<Chan>::iterator it = target.begin(); it != target.end(); ++it) {
		std::cout << (*it).get_name() << " ➜ ";
	}
	std::cout << YELLOW << "(end)" << RESET << std::endl;
}

int main(int argc, char **argv)
{
	Channels	channels;
	user		user, user2;
	std::string name1, name2;

	name1 = "chan1";
	name2 = "chan2";

	user.nickname_ = "lee";

	channels.join_channel(user, name1);
	

	std::cout << BOLDCYAN << " channels name  : " << RESET;
	std::cout << channels.get_channels().at(0).get_name() << std::endl;

	user2.nickname_ = "token";
	std::cout << BOLDCYAN << " channels user2  : " << RESET;
	channels.join_channel(user2, name1);
	std::cout << std::endl; 
	showUsers(channels.get_channels().at(0).get_users());

	channels.join_channel(user, name2);
	showUsers(channels.get_channels().at(1).get_users());

	std::cout << BOLDWHITE << channels.get_channels().at(0).get_info() << RESET << std::endl << std::endl;
	
	channels.leave_channel(user, name1);

	std::cout << BOLDWHITE << channels.get_channels().at(0).get_info() << RESET << std::endl << std::endl;
	std::cout << BOLDWHITE << channels.get_channels().at(1).get_info() << RESET << std::endl;

	showChannels(channels.get_channels());
	channels.delete_channel(name2);
	showChannels(channels.get_channels());


	// if (argc != 3)
	// {
	// 	//에러
	// 	system("clear");
	// 	std::cerr << RED << "err: Wrong Arguments" << RESET << std::endl;
	// 	exit(EXIT_FAILURE);
	// }
	// std::string	port(argv[1]);
	// std::string password(argv[2]);
	// Server		server(port, password);

	// server.start();
}