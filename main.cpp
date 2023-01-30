#include "Channel.hpp"
#include "color.hpp"
//#include "Channel.hpp"
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

// static void _test1(Channel& channels, user& user1, user& user2) {
// 	channels.join_channel(user, name1);
	
// 	std::cout << BOLDCYAN << " channels name  : " << RESET;

// 	std::cout << channels.get_channels().at(0).get_name() << std::endl;

// 	std::cout << BOLDCYAN << " channels user2  : " << RESET;
// 	channels.join_channel(user2, name1);
// 	std::cout << std::endl; 
// 	showUsers(channels.get_channels().at(0).get_users());

// 	channels.join_channel(user, name2);
// 	showUsers(channels.get_channels().at(1).get_users());

// 	std::cout << BOLDWHITE << channels.get_channels().at(0).get_info() << RESET << std::endl << std::endl;
	
// 	channels.leave_channel(user, name1);

// 	std::cout << BOLDWHITE << channels.get_channels().at(0).get_info() << RESET << std::endl << std::endl;
// 	std::cout << BOLDWHITE << channels.get_channels().at(1).get_info() << RESET << std::endl;

// 	showChannels(channels.get_channels());
// 	channels.delete_channel(name2);
// 	showChannels(channels.get_channels());
// }

int main(int argc, char **argv)
{
	std::vector<Udata> events;
	std::vector<Udata> join1;
	std::vector<Udata> join2;
	std::vector<Udata> join3;
	std::vector<Udata> join4;
	std::vector<Udata> kick1;
	std::vector<Udata> quit1;


	
	Channels	channels;
	user		user1, user2, user3, user4;
	std::string chan1, chan2;

	chan1 = "chan1";
	chan2 = "chan2";
	user1.nickname_ = "lee";
	user2.nickname_ = "token";
	user3.nickname_ = "three";
	user4.nickname_ = "whatever";

	join1 = channels.join_channel(user1, chan1);
	join2 = channels.join_channel(user2, chan1);
	join3 = channels.join_channel(user3, chan1);
	join4 = channels.join_channel(user4, chan1);
	showUsers(channels.select_channel(chan1).get_users());

	std::cout << BOLDCYAN << " --- JOIN --- " << RESET << std::endl;

	std::cout << join1[0].msg << std::endl;
	std::cout << join2[0].msg << std::endl;
	std::cout << join3[0].msg << std::endl;
	std::cout << join4[0].msg << std::endl;


	std::cout << BOLDCYAN << " --- KICK --- " << RESET << std::endl;
	showUsers(channels.select_channel(chan1).get_users());
	kick1 = channels.kick_channel(user1, user2, chan1);
	showUsers(channels.select_channel(chan1).get_users());

	std::cout << kick1[0].msg << std::endl;
	std::cout << kick1[1].msg << std::endl;
	std::cout << kick1[2].msg << std::endl;
	std::cout << kick1[3].msg << std::endl;

	std::cout << BOLDCYAN << " --- QUIT --- " << RESET << std::endl;
	showUsers(channels.select_channel(chan1).get_users());
	quit1 = channels.quit_channel(user4);
	showUsers(channels.select_channel(chan1).get_users());

	std::cout << quit1[0].msg << std::endl;
	std::cout << quit1[1].msg << std::endl;
	std::cout << quit1[2].msg << std::endl;
	std::cout << quit1[3].msg << std::endl;


	

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