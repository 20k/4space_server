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

struct network_object
{
    ///who's sending the data
    serialise_owner_type owner_id = -1;
    serialise_data_type serialise_id = -1;
};

using packet_id_type = int32_t;
using sequence_data_type = int32_t;

#pragma pack(1)
struct packet_header
{
    uint32_t current_size;
    int32_t overall_size = 0;
    packet_id_type packet_id = 0;
    sequence_data_type sequence_number = 0;

    uint32_t calculate_size()
    {
        return sizeof(overall_size) + sizeof(packet_id) + sizeof(sequence_number);
    }
};

inline
byte_vector get_fragment(int id, const network_object& no, const std::vector<char>& data, packet_id_type use_packet_id)
{
    int fragments = get_packet_fragments(data.size());

    sequence_data_type sequence_number = 0;

    packet_header header;
    header.current_size = data.size() + header.calculate_size() + sizeof(no);
    header.overall_size = header.current_size;
    header.packet_id = use_packet_id;

    if(fragments == 1)
    {
        byte_vector vec;

        vec.push_back(canary_start);
        vec.push_back(message::FORWARDING_ORDERED_RELIABLE);
        vec.push_back(header);
        vec.push_back<network_object>(no);

        for(auto& i : data)
        {
            vec.push_back(i);
        }

        vec.push_back(canary_end);

        return vec;
    }

    int real_data_per_packet = ceil((float)data.size() / fragments);

    int sent = id * real_data_per_packet;
    int to_send = data.size();

    if(sent >= to_send)
        return byte_vector();

    byte_vector vec;

    int to_send_size = std::min(real_data_per_packet, to_send - sent);

    header.current_size = to_send_size + header.calculate_size() + sizeof(no);
    header.sequence_number = id;

    vec.push_back(canary_start);
    vec.push_back(message::FORWARDING_ORDERED_RELIABLE);

    vec.push_back(header);
    vec.push_back<network_object>(no);

    for(int kk = 0; kk < real_data_per_packet && sent < to_send; kk++)
    {
        vec.push_back(data[sent]);

        sent++;
    }

    vec.push_back(canary_end);

    return vec;
}

struct network_data
{
    network_object object;
    serialise data;
    packet_id_type packet_id;
    bool should_cleanup = false;
    bool processed = false;

    //sf::Clock clk;

    void set_complete()
    {
        processed = true;
        //clk.restart();
    }
};

struct network_packet_fragment_info
{
    byte_vector data;
};

struct network_packet_info
{
    std::map<sequence_data_type, network_packet_fragment_info> fragment_info;
};

struct network_owner_info
{
    std::map<packet_id_type, network_packet_info> packet_info;

    void store_packet_fragment(packet_id_type pid, sequence_data_type sid, byte_vector& vec)
    {
        packet_info[pid].fragment_info[sid].data = vec;
    }
};

struct network_reliable_ordered
{
    bool serv = false;
    packet_id_type next_packet_id = 0;

    //owner_to_packet_id_to_sequence_number_to_data

    std::map<serialise_owner_type, network_owner_info> owner_to_packet_info;

public:

    void init_server(){serv = true;}
    void init_client(){serv = false;}

    bool is_server(){return serv;}
    bool is_client(){return !serv;}

    void forward_data_to_server(udp_sock& sock, const sockaddr* store, const network_object& no, serialise& s)
    {
        int max_to_send = 20;

        int fragments = get_packet_fragments(s.data.size());

        bool should_slowdown = false;

        /*for(auto& i : last_unconfirmed_packet)
        {
            if(disconnected(i.first))
                continue;

            if(i.second < (int)packet_id - 2000)
                should_slowdown = true;
        }

        if(should_slowdown)
            max_to_send = 1;*/

        for(int i=0; i<fragments; i++)
        {
            byte_vector frag = get_fragment(i, no, s.data, next_packet_id);

            ///no.owner_id is.. always me here?
            owner_to_packet_info[no.owner_id].store_packet_fragment(next_packet_id, i, frag);

            if(i < max_to_send)
            {
                //while(!sock_writable(sock)) {}

                udp_send_to(sock, frag.ptr, store);
            }
        }

        next_packet_id = next_packet_id + 1;
    }
};


#endif // RELIABILITY_ORDERED_SHARED_HPP_INCLUDED
