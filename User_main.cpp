#include "Users.hpp"
#include "color.hpp"
#include <stdint.h> // cstdint c++11

// user 함수
// Udata    command_nick(std::string& nick_name, const uintptr_t& ident);
// Event    command_user(const std::string input[4], const uintptr_t& ident);
// Udata    command_quit(user& leaver, std::string& leave_msg);
// Udata    command_privmsg(std::string &target_name, std::string &line, const uintptr_t& ident);

// Udata 데이터 구조
// typedef std::map<uintptr_t, std::string>             Udata;
// typedef std::pair<uintptr_t, std::string>            Event;
// typedef std::map<uintptr_t, std::string>::iterator   Udata_iter;



int main(void)
{
    Users test;
    Udata u;
    const uintptr_t ident = 3;
    std::string nick_name = "hello";
    u = test.command_nick(nick_name, ident);
    for (Udata_iter it = u.begin(); it != u.end(); ++it)
    {
        std::cout << "socket : " << it->first << std::endl;
        std::cout << "message : " << it->second << std::endl;
    }
    // nick_name = "hi";
    try // <- receiver
    {
        u = test.command_nick(nick_name, ident);
    }
    catch(Event& e)
    {
        u.insert(e);
    }
    
    for (Udata_iter it = u.begin(); it != u.end(); ++it)
    {
        std::cout << "socket ) " << it->first << std::endl;
        std::cout << "message ) " << it->second << std::endl;
    }
    return 0;
}