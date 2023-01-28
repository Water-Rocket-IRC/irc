#pragma once

#include "Sender.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

enum mod
{
	NORMAL, 
	ADMIN
};

/// @brief 개별 클라이언트에서 접속한 유저의 정보
struct user
{
	std::string	nickname_;
	std::string username_;
	std::string hostname_;
	std::string servername_;
	std::string realname_;

	struct kevent event;
	int			client_sock_; //legacy variable, 곧 kevent로 대체해야 함
	int			mod;
	//그 외 기타등등
};

/// @brief 유저들을 관리하고, sender에게 적절한 응답을 요청한다.
class Users
{
	private:
		std::vector<user> user_list_;
	public:
		Users();
		void addnick(std::stringstream &line_ss, struct kevent event);
		void adduser(std::stringstream &line_ss, uintptr_t sock);

		user search_user_event(struct kevent event);
		user search_user_nick(std::string nick);


		void print_all_user(); //debug

};

Users::Users()
{
	
}

void	Users::addnick(std::stringstream &line_ss, struct kevent event)
{
	user tmp_usr;
	std::string nickname;
	std::vector<user>::iterator it;
	bool flag = true;

	line_ss >> nickname;

	for (it = user_list_.begin(); it != user_list_.end(); ++it)
	{
		if (it->nickname_ == nickname)
		{
			//sender의 에러메시지 메소드 호출
			flag = false;
		}
	}
	if (flag == true)
	{
		tmp_usr.nickname_ = nickname;
		tmp_usr.event = event;
		user_list_.push_back(tmp_usr);
	}
}

/*
https://datatracker.ietf.org/doc/html/rfc1459#section-4.1.3
에 따르면, 이건 서버에 처음 접속할때 사용되는 명령어다. NICK과 USER 모두 접수되어야, 서버에 레지스터 된 것이다.
중간에 유저 명령어 전송이 가능한지 확인해야 하고, 그에 따라 처리해야 한다.
*/
void Users::adduser(std::stringstream &line_ss, uintptr_t sock)
{
	std::vector<user>::iterator it;
	user tmp_user;

	 for (it = user_list_.begin(); it != user_list_.end(); ++it)
	 {
		//접속한 소켓을 찾아 정보를 추가한다
		if (it->event.ident == sock)
		{
			tmp_user = *it;

			if (tmp_user.nickname_.empty())
				break;
			std::string username, hostname, servername, realname;
			line_ss >> username >> hostname >> servername >> realname;
			realname.erase(0, 1); //prefix 제거

			tmp_user.username_ = username;
			tmp_user.hostname_ = hostname;
			tmp_user.servername_ = servername;
			tmp_user.realname_ = realname;

			//for debug
			std::cout << "user " << username << std::endl; 
			std::cout << "host " << hostname << std::endl;
			std::cout << "server " << servername << std::endl;
			std::cout << "real " << realname << std::endl;

			Sender::welcome_message(tmp_user.client_sock_, tmp_user.servername_, tmp_user.nickname_, tmp_user.hostname_);

			break;
		}
		// 예외처리 할 부분

		// nick없이 user만 들어왔으면 sender로 에러 메시지 출력? 실제 클라이언트와 서버가 어떻게 행동하는지 살펴보고 행동 결정해야함
		// USER의 매개변수가 부족할때 들어오면? nc로 쌩으로 보내면 그럴 수 있다.
	 }
}


/// @brief kqueue의 event를 통해 서버에 메시지를 전송한 유저를 식별하는 함수
/// @param event 서버가 listen한 event
/// @return user
user Users::search_user_event(struct kevent event)
{
	std::vector<user>::iterator it;
	user usr;

	for (it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (it->event.ident == event.ident)
			return *it;
	}
	std::cout << "Error : Unknown User accessed to the server" << std::endl;
	return usr;
}

user Users::search_user_nick(std::string nick)
{
	std::vector<user>::iterator it;
	user usr;

	for (it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (it->nickname_ == nick)
			return *it;
	}
	usr.client_sock_ = -433;
	return usr;
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
