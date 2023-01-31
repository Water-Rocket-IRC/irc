#pragma once

#include "Sender.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <sys/event.h>
#include <sys/time.h>

struct user;

/// @brief 유저들을 관리하고, sender에게 적절한 응답을 요청한다.
class Users
{
	private:
		std::vector<struct user>	user_list_;
	public:
		Udata											command_nick(std::stringstream &line_ss, struct kevent event);
		Udata											command_user(std::stringstream &line_ss, uintptr_t sock);
		Udata											command_quit(std::stringstream &line_ss, uintptr_t sock);
		Udata											command_privmsg(std::stringstream &line_ss, uintptr_t sock);
		Udata											command_Notice(std::stringstream &line_ss, uintptr_t sock);

		std::vector<user>::iterator						search_user_by_ident(uintptr_t sock);
		user&											search_user_by_ident2(uintptr_t sock);

		std::vector<user>::iterator						search_user_by_nick(std::string nickname);
		std::pair<std::vector<user>::iterator, bool>	check_dup_nick(std::string nickname);

		void print_all_user(); //debug
};

// ident 즉 socket을 이용해 지금 명령어 친 user가 누군 지 알아낸다. 
std::vector<user>::iterator	Users::search_user_by_ident(uintptr_t sock)
{
	std::vector<user>::iterator	it;

	for (it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (it->event.ident == sock) // 닉네임 바꿔야할 유저를 찾을 상태 !!! 
		{
			return (it);
		}
	}
	return (it);
}

user&	Users::search_user_by_ident2(uintptr_t sock)
{
	std::vector<user>::iterator	it;

	for (it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (it->event.ident == sock) // 닉네임 바꿔야할 유저를 찾을 상태 !!! 
		{
			return (*it);
		}
	}
	return (*it);
}

// nick 이용해 user가 누군 지 알아낸다. 
std::vector<user>::iterator	Users::search_user_by_nick(std::string nickname)
{
	std::vector<user>::iterator	it;

	for (it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (it->nickname_ == nickname) // 닉네임 바꿔야할 유저를 찾을 상태 !!! 
		{
			return (it);
		}
	}
	return (it);
}

// nick이 중복되는 지 체크하는 함수
// pair을 쓴 이유
// 중복되면 it는 중복된 user에서의 it와 bool값은 true를 가지고 나간다.
// 중복되지 않으면 it는 it는 end()와 bool값은 false를 가지고 나간다. 
std::pair<std::vector<user>::iterator, bool>	Users::check_dup_nick(std::string nickname)
{
    std::pair<std::vector<user>::iterator, bool> pair;

	for (pair.first = user_list_.begin(); pair.first != user_list_.end(); pair.first++)
	{
		if (pair.first->nickname_ == nickname) // 닉네임 바꿔야할 유저를 찾을 상태 !!! 
		{
			pair.second = true;
			return (pair);
		}
	}
	pair.second = false;
	return (pair);
}

// nick을 실행하는 함수
Udata	Users::command_nick(std::stringstream &line_ss, struct kevent event)
{
	std::vector<Udata>	ret;
	std::string			nickname;
	Udata				empty;
	line_ss >> nickname;

	// 경우의 수 2 가지 
	// 1. 중복 O -> 안된다고 적어줌
	// 2. 중복 X -> 된다고 적어줌
	std::pair<std::vector<user>::iterator, bool>	pair;
	std::vector<user>::iterator						my_user;

	// if (nickname.size() > 1 && nickname.at(0) == '#')
	// {
	// 	return (Sender::nick_wrong_message(*my_user, nickname));
	// }
	pair = check_dup_nick(nickname);
	my_user = search_user_by_ident(event.ident);
	if (pair.second) // 중복 O
	{
		return (Sender::nick_error_message(*my_user, nickname));
	}
	else // 중복 X, 바꾸면 된다. 
	{
		if (my_user == user_list_.end()) // 처음 들어온 경우
		{
			struct user			tmp_usr;
			tmp_usr.nickname_ = nickname;
			tmp_usr.event = event;
			user_list_.push_back(tmp_usr);
			bzero(&empty, sizeof(empty));
			return (empty);
		}
		my_user->nickname_ = nickname;
		my_user->event = event;
		return (Sender::nick_well_message(*my_user, *my_user, nickname));
	}
}

// quit을 실행하는 함수
Udata	Users::command_quit(std::stringstream &line_ss, uintptr_t sock)
{
	std::vector<user>::iterator	my_user;
	std::string					leave_message;

	line_ss >> leave_message;
	my_user = search_user_by_ident(sock);
	// static Udata quit_channel_message(struct user sender, std::string leave_message);

	for (std::vector<user>::iterator it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (my_user->nickname_ == it->nickname_)
		{
			user_list_.erase(it);
		}
	}
	return (Sender::quit_channel_message(*my_user, *my_user, leave_message));
}

// user를 실행하는 함수 
Udata	Users::command_user(std::stringstream &line_ss, uintptr_t sock)
{
	Udata	tmp;
	//std::vector<user>::iterator my_user = search_user_by_ident(sock);
	user& 	my_user = search_user_by_ident2(sock);
	std::string username, hostname, servername, realname;
	bzero(&tmp, sizeof(tmp));
	
	line_ss >> username >> hostname >> servername >> realname;
	
	if (my_user.nickname_.empty()) // nick을 안달고 온 것! 오류사항
	{
		std::cout << my_user.nickname_ << " socket is " << my_user.event.ident << std::endl;
		return (tmp);
	}
	if (my_user.username_.empty()) // 넣기 전에 비어있는 지 검사를 해야한다.
	{
		realname.erase(0, 1); //prefix 제거 ':' <- 이거 제거
		my_user.username_ = username;
		my_user.hostname_ = hostname;
		my_user.servername_ = servername;
		my_user.realname_ = realname;
		return(Sender::welcome_message_connect(my_user));
	}
	realname.erase(0, 1); //prefix 제거 ':' <- 이거 제거
	my_user.username_ = username;
	my_user.hostname_ = hostname;
	my_user.servername_ = servername;
	my_user.realname_ = realname;
	return (tmp);
}

Udata	Users::command_privmsg(std::stringstream &line_ss, uintptr_t sock)
{
	std::string					to_who, message;
	std::vector<user>::iterator	user_who_get;
	std::vector<user>::iterator	user_who_sent;

	line_ss >> to_who >> message;
	user_who_sent = search_user_by_ident(sock);

	// 사람에게 보냄
	user_who_get = search_user_by_nick(to_who);
	if (user_who_get == user_list_.end())
	{
		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		return (Sender::privmsg_p2p_message(*user_who_sent, *user_who_get, message)); // @@@@@@@@@ Sender랑 이야기해야함 노션에 포맷없음 @@@@@@@@@
		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	}
	return (Sender::privmsg_p2p_message(*user_who_sent, *user_who_get, message)); // 정상 작동! 
}

// Udata	Users::command_Notice(std::stringstream &line_ss, uintptr_t sock)
// {
// 	std::string					to_who, message;
// 	std::vector<user>::iterator	user_who_get;
// 	std::vector<user>::iterator	user_who_sent;

// 	line_ss >> to_who >> message;
// 	user_who_sent = search_user_by_ident(sock);

// 	// 사람에게 보냄
// 	user_who_get = search_user_by_nick(to_who);
// 	if (user_who_get == user_list_.end())
// 	{
// 		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// 		return (Sender::Notice_message(*user_who_sent, *user_who_get, message)); // @@@@@@@@@ Sender랑 이야기해야함 노션에 포맷없음 @@@@@@@@@
// 		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// 	}
// 	return (Sender::Notice_message(*user_who_sent, *user_who_get, message)); // 정상 작동! 
// }

//debug 함수
void Users::print_all_user()
{
	std::vector<user>::iterator it;

	for (it = user_list_.begin(); it != user_list_.end(); ++it)
	{
		std::cout << "user " << it->nickname_ << std::endl; 
		std::cout << "socket " << it->client_sock_ << std::endl;
	}
}