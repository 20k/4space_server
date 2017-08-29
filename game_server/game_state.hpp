#ifndef GAME_STATE_HPP_INCLUDED
#define GAME_STATE_HPP_INCLUDED

#include <vec/vec.hpp>
#include <net/shared.hpp>

#include <set>
#include <map>

#include "game_modes.hpp"

#include "../reliability_shared.hpp"
#include "../packet_clumping_shared.hpp"
#include "../game_mode_shared.hpp"

struct player
{
    //int32_t player_slot = 0;
    int32_t id = -1; ///per game
    int32_t team = 0;

    float ping_ms = 150;
    sf::Clock clk;
    float max_ping_ms = 250;

    //vec<float, 10> moving_avg = {0};
    //int mov_c;

    ///I don't know if I do want to store these
    ///I could carry on using the existing broadcast system
    ///but just pipe traffic through the server instead
    ///vec3f pos;
    ///vec3f rot;

    udp_sock sock;
    sockaddr_storage store;
    sf::Clock time_since_last_message;
};

///modify this to have player_id_reported_as_killer
struct kill_count_timer
{
    std::set<int32_t> player_ids_reported_deaths; ///who reported the death ids
    std::map<int32_t, int> player_id_of_reported_killer; ///the id of who is reported to have killed the player
    sf::Clock elapsed_time_since_started;
    const float max_time = 1000.f; ///1s window
};

struct server_game_state;

///unify with gamemode_info in game_server_session_resources
///shared gamemode resources
///This is server gamemode wrapper
///rename?
struct game_mode_handler
{
    game_mode_handler_shared shared_game_state;

    void tick(server_game_state* state);

    bool game_over();
};

struct respawn_request
{
    int32_t player_id = 0;
    sf::Clock clk;
    float time_to_respawn_ms = 5000;
    float time_to_remove_ms = 10000; ///the client spams the server. Wait this much more time before they can spawn again
    bool respawned = false;
};

struct server_game_state;

struct server_reliability_manager
{
    std::map<int32_t, reliability_manager> player_reliability_handler;

    void tick(server_game_state* state);

    void add(byte_vector& vec, int32_t to_skip, uint32_t reliable_id);
    void add_packetid_to_ack(uint32_t id, int32_t to_whom);

    void add_player(int32_t id);
    void remove_player(int32_t id);

    void process_ack(byte_fetch& fetch);
};

///so, we want to pipe everyone's ping to everyone else

///so the client will send something like
///update component playerid componentenum value
///uuh. this is really a networking class?
struct server_game_state
{
    packet_clumper packet_clump;

    server_reliability_manager reliable;

    int max_players = 10;

    int map_num = 0; ///????
    std::vector<std::vector<vec2f>> respawn_positions;

    std::vector<respawn_request> respawn_requests;

    ///we really need to handle this all serverside
    ///which means the server is gunna have to keep track
    ///lets implement a simple kill counter
    game_mode_handler mode_handler;

    ///maps player id who died to kill count structure
    std::map<int32_t, kill_count_timer> kill_confirmer;

    int16_t gid = 0;

    float timeout_time_ms = 10000;
    float ping_interval_ms = 1000;
    sf::Clock ping_interval_clk;
    float last_time_ms = 0;
    sf::Clock running_time;

    ///THIS IS NOT A MAP
    ///PLAYER IDS ARE NOT POSITIONS IN THIS STRUCTURE
    std::vector<player> player_list;

    int number_of_team(int team_id);

    int32_t get_team_from_player_id(int32_t id);
    player get_player_from_player_id(int32_t id);
    int32_t get_pos_from_player_id(int32_t id);

    void broadcast(const std::vector<char>& dat, const int& to_skip);
    void broadcast(const std::vector<char>& dat, sockaddr_storage& to_skip);
    void broadcast_clump(const std::vector<char>& dat, sockaddr_storage& to_skip);

    void cull_disconnected_players();
    void add_player(udp_sock& sock, sockaddr_storage store);
    int16_t get_new_id();

    ///for the moment just suck at it

    int32_t sockaddr_to_playerid(sockaddr_storage& who);

    //void tick_all();

    void tick();

    void process_received_message(byte_fetch& fetch, sockaddr_storage& who);
    void process_reported_message(byte_fetch& fetch, sockaddr_storage& who);
    void process_join_request(udp_sock& sock, byte_fetch& fetch, sockaddr_storage& who);
    void process_respawn_request(udp_sock& sock, byte_fetch& fetch, sockaddr_storage& who);
    //void process_ping_and_forward(udp_sock& sock, byte_fetch& fetch, sockaddr_storage& who);
    void process_ping_response(udp_sock& sock, byte_fetch& fetch, sockaddr_storage& who);
    void process_ping_gameserver(udp_sock& sock, byte_fetch& fetch, sockaddr_storage& who);

    void ping();

    void broadcast_ping_data();

    void balance_teams();
    vec2f find_respawn_position(int team_id);
    void respawn_player(int32_t player_id);
    //void respawn_all();
    void ensure_player_info_entry();

    void periodic_team_broadcast();
    void periodic_gamemode_stats_broadcast();
    void periodic_respawn_info_update();
    void periodic_player_stats_update();

    void reset_player_disconnect_timer(sockaddr_storage& store);

    void set_map(int);
};


#endif // GAME_STATE_HPP_INCLUDED
