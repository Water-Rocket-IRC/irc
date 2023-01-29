#pragma once

#include <vector>
#include "Users.hpp"
#include <algorithm>

class Chan
{
	private:
		std::string			name_;
		std::string			topic_;
		int					error_;
		user				host_;				// operator
		std::vector<user>	connectors_;		// connectors.at(0) => operator

	public:
		std::string		get_info(void);

	//채널을 처음 만든 사람 혹은,
		std::string	get_name();
		user get_host();
		void 		set_host();
		bool 		is_user(user& usr);
		void 		add_user(user joiner);
		void 		set_channel_name(std::string& chan_name);
		void		delete_user(user& user_name);
		
		std::vector<user>& get_users();

		bool operator==(const Chan& t) const;	
		void seterror();
		int geterror();
};

void Chan::seterror()
{
	error_ = 1;
}
int Chan::geterror()
{
	return this->error_;
}
std::string Chan::get_name()
{
	return this->name_;
}

bool Chan::is_user(user& usr)
{
	std::vector<user>::iterator it;

	for (it = connectors_.begin(); it != connectors_.end(); it++)
	{
		if (it->nickname_ == usr.nickname_)
			return 1;
	}
	return 0;
}

void Chan::add_user(user joiner)
{
	connectors_.push_back(joiner);
}

void Chan::set_host()
{
	this->host_ = connectors_[0];
}

void Chan::set_channel_name(std::string& chan_name)
{
	this->name_ = chan_name;
}

std::vector<user>& Chan::get_users()
{
	return this->connectors_;
}

void	Chan::delete_user(user& user_name)
{
	std::vector<user>::iterator it = std::find(this->connectors_.begin(), \
	this->connectors_.end(), user_name);
	std::size_t size = std::distance(this->connectors_.begin(), it);
	this->connectors_.erase(this->connectors_.begin() + size);
}

bool Chan::operator==(const Chan& t) const
{
	return (this->name_ == t.name_);
}

user Chan::get_host()
{
	return this->host_;
}

std::string Chan::get_info()
{
	std::string ret;
	std::stringstream index_ss;
	index_ss << connectors_.size();

	ret =	"Channel's name  : " + name_ + "\n" +
			"Channel's topic : " + topic_ + "\n" +
			"Connected users : " + index_ss.str() + "\n" +
			"Channel's host  : " + host_.nickname_;
	return ret;
} 