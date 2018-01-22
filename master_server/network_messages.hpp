#ifndef NETWORK_MESSAGES_HPP_INCLUDED
#define NETWORK_MESSAGES_HPP_INCLUDED

#include "../../serialise/serialise.hpp"

#define GAMESERVER_PORT "6950"
#define MASTER_PORT     "6850" ///for server functions
#define MASTER_CLIENT_PORT "6851" ///for client functions

//#define MASTER_IP "127.0.0.1"

/*#define MASTER_IP "88.\
                   150.\
                   175.\
                   30"*/

//#define MASTER_IP "enigma.simrai.com"

#define MASTER_IP "77.96.132.101"

#include <stdint.h>

#include <net/shared.hpp>

namespace message
{
    enum message : int32_t
    {
        DUMMY,
        GAMESERVER,
        CLIENT,
        GAMESERVERUPDATE,
        CLIENTRESPONSE,
        CLIENTJOINREQUEST,
        CLIENTJOINACK,
        FORWARDING,
        FORWARDING_RELIABLE, ///;_;
        FORWARDING_RELIABLE_ACK,
        FORWARDING_ORDERED_RELIABLE,
        FORWARDING_ORDERED_RELIABLE_ACK,
        FORWARDING_ORDERED_RELIABLE_REQUEST,
        REPORT, ///client has something to report to the server
        TEAMASSIGNMENT,
        GAMEMODEUPDATE, ///periodic broadcast of the relevant stats
        RESPAWNREQUEST,
        RESPAWNRESPONSE,
        RESPAWNINFO,
        PING,
        PING_RESPONSE,
        PING_DATA,
        PING_GAMESERVER,
        PING_GAMESERVER_RESPONSE,
        PLAYER_STATS_UPDATE_INDIVIDUAL, ///kills, deaths for a player
        KEEP_ALIVE,
        REQUEST,
        PAUSE_DATA,
        PACKET_ACK,
        PUNCHTHROUGH_FROM_CLIENT,
        PUNCHTHROUGH_TO_GAMESERVER,
        PUNCHTHROUGH_PACKET,
    };
}

namespace net_type
{
    using message_t = int32_t;
    using player_t = int16_t;
    using component_t = int16_t;
    using len_t = int8_t;
}

struct network_game_server : serialisable
{
    std::string ip;
    uint32_t port = 0;
    int32_t player_count = 0;

    void do_serialise(serialise& s, bool ser) override
    {
        s.handle_serialise(ip, ser);
        s.handle_serialise(port, ser);
        s.handle_serialise(player_count, ser);
    }
};

struct network_game_server_list : serialisable
{
    decltype(canary_start) can_start = canary_start;
    message::message type = message::CLIENTRESPONSE;
    std::vector<network_game_server> servers;
    decltype(canary_end) can_end = canary_end;

    void do_serialise(serialise& s, bool ser) override
    {
        s.handle_serialise(can_start, ser);
        s.handle_serialise(type, ser);
        s.handle_serialise(servers, ser);
        s.handle_serialise(can_end, ser);
    }
};

///canary_start
///message::REPORT
///TYPE
///PLAYERID ///optional?
///LEN
///DATA
///canary_end
namespace report
{
    enum report : int32_t
    {
        DEATH,
        COUNT
    };
}

struct punchthrough_to_server_data : serialisable
{
    decltype(canary_start) can_start = canary_start;
    message::message type = message::PUNCHTHROUGH_TO_GAMESERVER;
    std::string client_ip;
    std::string client_port;
    decltype(canary_end) can_end = canary_end;

    virtual void do_serialise(serialise& s, bool ser) override
    {
        s.handle_serialise(can_start, ser);
        s.handle_serialise(type, ser);
        s.handle_serialise(client_ip, ser);
        s.handle_serialise(client_port, ser);
        s.handle_serialise(can_end, ser);
    }
};

struct punchthrough_from_client_data : serialisable
{
    decltype(canary_start) can_start = canary_start;
    message::message type = message::PUNCHTHROUGH_FROM_CLIENT;

    std::string gserver_ip;
    std::string gserver_port;

    std::string my_port;

    decltype(canary_end) can_end = canary_end;

    virtual void do_serialise(serialise& s, bool ser) override
    {
        s.handle_serialise(can_start, ser);
        s.handle_serialise(type, ser);
        s.handle_serialise(gserver_ip, ser);
        s.handle_serialise(gserver_port, ser);
        s.handle_serialise(my_port, ser);
        s.handle_serialise(can_end, ser);
    }
};

struct punchthrough_packet : serialisable
{
    decltype(canary_start) can_start = canary_start;
    message::message type = message::PUNCHTHROUGH_PACKET;
    decltype(canary_end) can_end = canary_end;

    virtual void do_serialise(serialise& s, bool ser) override
    {
        s.handle_serialise(can_start, ser);
        s.handle_serialise(type, ser);
        s.handle_serialise(can_end, ser);
    }
};

typedef report::report report_t;

#endif // NETWORK_MESSAGES_HPP_INCLUDED
