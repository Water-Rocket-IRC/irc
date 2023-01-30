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

Udata	Users::command_nick(std::stringstream &line_ss, struct kevent event)
{
	struct user			tmp_usr;
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
		return (Sender::nick_error_message(my_user, nickname))
	}
	else
	{
		my_user->nickname_ = nickname;
		my_user->event = event;
		return (Sender::nick_well_message(my_user, nickname));
	}
}

// user Users::search_user_nick(std::string nick)
// {
// 	std::vector<user>::iterator it;
// 	user usr;

// 	for (it = user_list_.begin(); it != user_list_.end(); it++)
// 	{
// 		if (it->nickname_ == nick)
// 			return *it;
// 	}
// 	usr.client_sock_ = -433;
// 	return usr;
// }




























/*
https://datatracker.ietf.org/doc/html/rfc1459#section-4.1.3
에 따르면, 이건 서버에 처음 접속할때 사용되는 명령어다. NICK과 USER 모두 접수되어야, 서버에 레지스터 된 것이다.
중간에 유저 명령어 전송이 가능한지 확인해야 하고, 그에 따라 처리해야 한다.
*/
Udata	Users::command_user(std::stringstream &line_ss, uintptr_t sock)
{
	std::vector<user>::iterator it;
	user tmp_user;
	
    std::vector<Udata> ret;
    
    // Udata *ret = new Udata;

    for (it = user_list_.begin(); it != user_list_.end(); ++it)
    {
        //접속한 소켓을 찾아 정보를 추가한다
        if (it->event.ident == sock)
        {
            tmp_user = *it;

            if (tmp_user.nickname_.empty())
                break;
            std::string username, username, hostname, realname;
            line_ss >> username >> hostname >> hostname >> realname;
            realname.erase(0, 1); //prefix 제거 ':' <- 이거 제거

            tmp_user.username_ = username;
            tmp_user.hostname_ = hostname;
            tmp_user.hostname_ = hostname;
            tmp_user.realname_ = realname;

            ret = Sender::welcome_message(tmp_user.event.ident, tmp_user.hostname_, tmp_user.nickname_, tmp_user.hostname_);
            return (ret);
}
    // 예외처리 할 부분

    // nick없이 user만 들어왔으면 sender로 에러 메시지 출력? 실제 클라이언트와 서버가 어떻게 행동하는지 살펴보고 행동 결정해야함
    // USER의 매개변수가 부족할때 들어오면? nc로 쌩으로 보내면 그럴 수 있다.
    }
    return (ret);
}


/// @brief kqueue의 event를 통해 서버에 메시지를 전송한 유저를 식별하는 함수
/// @param event 서버가 listen한 event
/// @return user
// user Users::search_user_event(struct kevent event)
// {
// 	std::vector<user>::iterator it;
// 	user usr;

// 	for (it = user_list_.begin(); it != user_list_.end(); it++)
// 	{
// 		if (it->event.ident == event.ident)
// 			return *it;
// 	}
// 	std::cout << "Error : Unknown User accessed to the server" << std::endl;
// 	return usr;
// }















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

	return (Sender::quit_channel_message(my_user, leave_message));
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