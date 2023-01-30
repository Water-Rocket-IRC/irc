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
	std::vector<Udata> join1, join2, join3, join4, kick1, quit1, priv1, notice1, wall1;

	Channels	channels;
	user		user1, user2, user3, user4;
	std::string chan1, chan2;

	chan1 = "chan1";
	chan2 = "chan2";
	user1.nickname_ = "lee"; user1.username_ = "userlee"; user1.realname_ = "reallee";
	user2.nickname_ = "token"; user2.realname_ = "kim token";
	user3.nickname_ = "three"; user3.realname_ = "lee three";
	user4.nickname_ = "whatever"; user4.realname_ = "park whatever";

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

	std::string priv = "This is MSG man";
	std::cout << BOLDCYAN << " --- PRIV --- " << RESET << std::endl;
	std::memset(&priv1, 0, sizeof(priv1));
	priv1 = channels.channel_msg(user1, chan1, priv);
	std::cout << priv1[0].msg << std::endl;
	std::cout << priv1[1].msg << std::endl;

	std::string notice = "This is Wall man";
	std::cout << BOLDCYAN << " --- Wall --- " << RESET << std::endl;
	// wall1.push_back(channels.channel_wall(user1, chan1, notice));
	// std::cout << wall1[0].msg << std::endl;
	wall1.push_back(channels.channel_wall(user3, chan1, notice));
	std::cout << wall1[0].msg << std::endl;
	
 	std::cout << BOLDWHITE << channels.get_channels().at(0).get_info() << RESET << std::endl;

	notice = "Crazzy topic";
	std::memset(&priv1, 0, sizeof(priv1));
	priv1 = channels.set_topic(user1, chan1, notice);
	std::cout << BOLDWHITE << " ------------------- " << RESET << std::endl;
 	std::cout << BOLDWHITE << channels.get_channels().at(0).get_info() << RESET << std::endl;
	std::cout << priv1[0].msg << std::endl;
	std::cout << priv1[1].msg << std::endl;
	std::cout << priv1[2].msg << std::endl;

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