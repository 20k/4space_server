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

#define MASTER_IP "77.96.132.101"

//#define MASTER_IP "192.168.0.54"

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

#endif // NETWORK_MESSAGES_HPP_INCLUDED
