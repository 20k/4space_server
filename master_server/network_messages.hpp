#ifndef NETWORK_MESSAGES_HPP_INCLUDED
#define NETWORK_MESSAGES_HPP_INCLUDED

#define GAMESERVER_PORT "6950"
#define MASTER_PORT     "6850" ///for server functions
#define MASTER_CLIENT_PORT "6851" ///for client functions

//#define MASTER_IP "127.0.0.1"

/*#define MASTER_IP "88.\
                   150.\
                   175.\
                   30"*/

//#define MASTER_IP "enigma.simrai.com"

#define MASTER_IP "62.31.131.55"


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
    };
}

namespace net_type
{
    using message_t = int32_t;
    using player_t = int16_t;
    using component_t = int16_t;
    using len_t = int8_t;
}

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

typedef report::report report_t;

///this is the data for the forwarding specific section of the packet
struct forwarding_data
{
    ///unique to the client, client auto increments, server discards multiple packets with same id
    ///then, when we pipe it back to the client, they ignore multiple placets with the same id
    ///winrar
    ///server may need to change the packet id to be its own packet id, rather than the other clients id
    int32_t packet_id = -1;
};

///this is a descriptor of the above for clients/servers to keep
struct forwarding_info
{
    bool sent_ack = false;
    bool skip_send = false;
    uint32_t id = -1;
    float time_elapsed = 0.f;

    byte_vector data;
};

#endif // NETWORK_MESSAGES_HPP_INCLUDED
