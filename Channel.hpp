#include "Users.hpp"

class Chan
{
	private:
		std::vector<user> connectors;
		std::string topic;
		std::string host_nick;

	public:
	//채널을 처음 만든 사람 혹은, 
		void set_host();
		void set_host(std::string nickname);

};

class Channels
{
	private:
		std::vector<Chan> Channels;

	public:
		void create_channel(std::string chan_name);
		void delete_channel(std::string chan_name);
};

void Channels::create_channel(std::string chan_name)
{

}

void Channels::delete_channel(std::string chan_name)
{
	
}