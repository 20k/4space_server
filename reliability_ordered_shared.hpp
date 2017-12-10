#ifndef RELIABILITY_ORDERED_SHARED_HPP_INCLUDED
#define RELIABILITY_ORDERED_SHARED_HPP_INCLUDED

#include "../serialise/serialise.hpp"

inline
void send_join_game(udp_sock& sock)
{
    byte_vector vec;
    vec.push_back(canary_start);
    vec.push_back(message::CLIENTJOINREQUEST);
    vec.push_back(canary_end);

    udp_send(sock, vec.ptr);
}

inline
udp_sock join_game(const std::string& address, const std::string& port)
{
    udp_sock sock = udp_connect(address, port);

    send_join_game(sock);

    return sock;
}

inline
int get_max_packet_size_clientside()
{
    return 450;
}

inline
int get_packet_fragments(int data_size)
{
    int max_data_size = get_max_packet_size_clientside();

    int fragments = ceil((float)data_size / max_data_size);

    return fragments;
}

struct network_reliable_ordered
{
    bool serv = false;

    void init_server(){serv = true;}
    void init_client(){serv = false;}

    bool is_server(){return serv;}
    bool is_client(){return !serv;}
};

#endif // RELIABILITY_ORDERED_SHARED_HPP_INCLUDED
