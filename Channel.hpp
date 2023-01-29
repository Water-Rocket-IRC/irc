#pragma once

#include "Chan.hpp"

class Channels
{
	private:
		std::vector<Chan> Channels_;

	public:
		bool		is_channel(std::string& chan_name);
		void 		create_channel(user& joiner, std::string& chan_name);
		void 		delete_channel(std::string& chan_name);
		void 		join_channel(user& joiner, std::string& chan_name);
		void		leave_channel(user&leaver, std::string& chan_name);

		Chan&		select_channel(std::string& chan_name);

		std::vector<Chan>& get_channels() { return	Channels_; };
};



bool		Channels::is_channel(std::string& chan_name)
{
	std::vector<Chan>::iterator it;

	for (it = Channels_.begin(); it != Channels_.end(); ++it)
	{
		if (it->get_name() == chan_name)
		{
			return true;
		}
	}
	return false;
}

void Channels::create_channel(user& joiner, std::string& chan_name)
{
	Chan	tmp;
	
	tmp.set_channel_name(chan_name);
	tmp.add_user(joiner);
	tmp.set_host();
	Channels_.push_back(tmp);
}

/* 
 * operator 가 없을 때 (0 index가 비어있다면 삭제)
 */
void Channels::delete_channel(std::string& chan_name)
{
	Chan tmp;

	tmp.set_channel_name(chan_name);
	std::vector<Chan>::iterator it = std::find(Channels_.begin(), \
	Channels_.end(), tmp);
	std::size_t size = std::distance(this->Channels_.begin(), it);
	this->Channels_.erase(this->Channels_.begin() + size);
}


Chan&	Channels::select_channel(std::string& chan_name)
{
	Chan tmp;

	tmp.set_channel_name(chan_name);
	std::vector<Chan>::iterator result = std::find(Channels_.begin(), \
	Channels_.end(), tmp);

	return *result;
}

void Channels::join_channel(user& joiner, std::string& chan_name)
{
	if (is_channel(chan_name) == false)
	{
		this->create_channel(joiner, chan_name);
	}
	else
	{
		Chan& chan = select_channel(chan_name);
		if (chan.is_user(joiner) == true)
		{
			std::cerr << "ERROR" << std::endl;
		}
		else
		{
			chan.add_user(joiner);
		}
	}
}

void	Channels::leave_channel(user&leaver, std::string& chan_name)
{
	if (is_channel(chan_name) == false)
	{
		//채널이 없을 경우, 오류 메시지 유저에게 전송
	}
	else
	{
		Chan& chan = select_channel(chan_name);
		std::vector<user> users = chan.get_users();

		//Msg전송 : PART 내용에 따라 전송

		chan.delete_user(leaver);
		if (users.size() == 0)
		{
			delete_channel(chan_name);
		}
		else
		{
			if (leaver == chan.get_host())
			{
				chan.set_host();
			}
		}

	}
	
}
