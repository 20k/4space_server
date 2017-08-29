#ifndef SERVER_HPP_INCLUDED
#define SERVER_HPP_INCLUDED

#include <net/shared.hpp>

struct tcp_sock;

struct game_server
{
    //tcp_sock sock;

    std::string address = "127.0.0.1";
    std::string my_port_to_them = SERVERPORT;
    std::string their_host_port = SERVERPORT;

    int32_t max_players = -1;
    int32_t current_players = -1;
    float ping = -1;
    bool pinged = false;
};

///me!
/*struct master_server
{
    std::vector<game_server> servers;

    //void cull_dead();
    void tick_all();

    //game_server server_from_sock(tcp_sock& sock, uint32_t port);

    void add_server(const game_server& serv);

    std::vector<char> get_client_response();
};*/

#endif // SERVER_HPP_INCLUDED
