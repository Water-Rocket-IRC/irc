#pragma once

#include "Udata.hpp"
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
		std::vector<struct user> user_list_;
	public:

        // Udata*  
		Udata											command_nick(std::stringstream &line_ss, struct kevent event);
		Udata											command_user(std::stringstream &line_ss, uintptr_t sock);
		Udata											command_quit(std::stringstream &line_ss, uintptr_t sock);

		std::vector<user>::iterator						Users::search_user_by_ident(uintptr_t sock);
		std::pair<std::vector<user>::iterator, bool>	Users::check_dup_nick(std::string nickname);

        bool    check_dup_username(std::string username);

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
	line_ss >> nickname;

	// 경우의 수 2 가지 
	// 1. 중복 O -> 안된다고 적어줌
	// 2. 중복 X -> 된다고 적어줌
	std::pair<std::vector<user>::iterator, bool> pair;
	std::vector<user>::iterator my_user;

	pair = check_dup_nick(nickname);
	my_user = search_user_by_ident(event.ident);
	if (pair.second) // 중복 O
	{
		return (Sender::nick_error_message(*my_user, nickname))
	}
	else // 중복 X, 바꾸면 된다. 
	{
		if (my_user == user_list_.end()) // 처음 들어온 경우
		{
			struct user			tmp_usr;
			tmp_usr.nickname_ = nickname;
			tmp_usr.event = event;
			user_list_.push_back(tmp_usr);
			return (Sender::nick_well_message(tmp_usr, nickname));
		}
		my_user->nickname_ = nickname;
		my_user->event = event;
		return (Sender::nick_well_message(*my_user, nickname));
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

	return (Sender::quit_channel_message(*my_user, leave_message));
}

// user를 실행하는 함수 
Udata	Users::command_user(std::stringstream &line_ss, uintptr_t sock)
{
	std::vector<user>::iterator my_user = search_user_by_ident(sock);
	std::string username, username, hostname, realname;
	
	line_ss >> username >> hostname >> hostname >> realname;
	
	if (my_user->nickname_.empty()) // nick을 안달고 온 것! 오류사항
		return (Udata());
	if (my_user->username_.empty())
	{
		realname.erase(0, 1); //prefix 제거 ':' <- 이거 제거
		my_user->username_ = username;
		my_user->hostname_ = hostname;
		my_user->hostname_ = hostname;
		my_user->realname_ = realname;
		return(Sender::welcome_message(*my_user));
	}
	realname.erase(0, 1); //prefix 제거 ':' <- 이거 제거
	my_user->username_ = username;
	my_user->hostname_ = hostname;
	my_user->hostname_ = hostname;
	my_user->realname_ = realname;
	return (Udata());
}

bool    Users::check_dup_username(std::string username)
{
	std::vector<user>::iterator it;

	for (it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (it->username_ == username)
            return (true);    
	}
	return (false);
}

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