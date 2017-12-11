#include <iostream>
#include <net/shared.hpp>

#include <vector>
#include "../master_server/network_messages.hpp"
#include <vec/vec.hpp>
#include "game_state.hpp"
#include "../reliability_ordered_shared.hpp"

#include <cl/cl.h>

void ping_master(server_game_state& my_state, int32_t port, udp_sock& to_master)
{
    static sf::Clock clk;
    static bool init = false;

    if(clk.getElapsedTime().asSeconds() < 1 && init)
        return;

    init = true;

    if(!to_master.valid())
    {
        if(to_master.udp_connected)
        {
            to_master.close();
            printf("Local sock error (this is not caused by master server termination), reopening socket");
        }

        to_master = udp_connect(MASTER_IP, MASTER_PORT);

        printf("Creating socket to master server\n");
    }

    byte_vector vec;

    vec.push_back<int32_t>(my_state.player_list.size());
    vec.push_back<int32_t>(port);

    udp_send(to_master, vec.ptr);

    clk.restart();
}

using namespace std;

///so as it turns out, you must use canaries with tcp as its a stream protocol
///that will by why the map client doesn't work
///:[
int main(int argc, char* argv[])
{
    //sock_info inf = try_tcp_connect(MASTER_IP, MASTER_PORT);
    //tcp_sock to_server;

    std::string host_port = GAMESERVER_PORT;

    for(int i=1; i<argc; i++)
    {
        if(strncmp(argv[i], "-port", strlen("-port")) == 0)
        {
            if(i + 1 < argc)
            {
                host_port = argv[i+1];
            }
        }
    }

    uint32_t pnum = atoi(host_port.c_str());

    udp_sock my_server = udp_host(host_port);
    sock_set_non_blocking(my_server, 1);

    printf("Registered on port %s\n", my_server.get_host_port().c_str());

    server_game_state my_state;
    my_state.mode_handler.shared_game_state.current_game_mode = game_mode::FFA;
    my_state.reliable_ordered.init_server();

    //my_state.set_map(0);

    udp_sock to_master;


    bool once = false;

    bool going = true;

    while(going)
    {
        ping_master(my_state, pnum, to_master);

        /*if(sock_readable(to_server))
        {
            auto dat = tcp_recv(to_server);
        }

        if(!to_server.valid())
        {
            if(!inf.within_timeout())
            {
                ///resets sock info
                inf.retry();
                inf = try_tcp_connect(MASTER_IP, MASTER_PORT);

                printf("trying\n");
            }

            if(inf.valid())
            {
                to_server = tcp_sock(inf.get());

                byte_vector vec;
                vec.push_back(canary_start);
                vec.push_back(message::GAMESERVER);
                vec.push_back<uint32_t>(pnum);
                vec.push_back(canary_end);

                tcp_send(to_server, vec.ptr);

                printf("found master server\n");
            }

            //continue;
        }*/

        bool any_read = true;

        while(any_read && sock_readable(my_server))
        {
            sockaddr_storage store;

            auto data = udp_receive_from(my_server, &store);

            any_read = data.size() > 0;

            byte_fetch fetch;
            fetch.ptr.swap(data);

            while(!fetch.finished() && any_read)
            {
                int32_t found_canary = fetch.get<int32_t>();

                while(found_canary != canary_start && !fetch.finished())
                {
                    found_canary = fetch.get<int32_t>();
                }

                if(fetch.finished())
                    continue;

                //while(!sock_writable(my_server)){}

                int32_t type = fetch.get<int32_t>();


                if(type == message::CLIENTJOINREQUEST)
                {
                    my_state.process_join_request(my_server, fetch, store);
                }

                else if(type == message::FORWARDING)
                {
                    my_state.process_received_message(fetch, store);
                }

                else if(type == message::REPORT)
                {
                    my_state.process_reported_message(fetch, store);
                }
                else if(type == message::RESPAWNREQUEST)
                {
                    my_state.process_respawn_request(my_server, fetch, store);
                }
                else if(type == message::FORWARDING_RELIABLE)
                {
                    int32_t player_id = my_state.sockaddr_to_playerid(store);

                    uint32_t reliable_id = -1;

                    byte_vector vec = reliability_manager::strip_data_from_forwarding_reliable(fetch, reliable_id);

                    ///don't want to replicate it back to the player, do we!
                    ///we're calling add, but add sticks on canaries
                    ///really we want to strip down the message

                    ///so uuh. this is instructing the server to just repeatdly forward
                    ///the message to the client
                    ///whereas obviously we want to check if they've already got it
                    ///I'm not smart
                    my_state.reliable.add(vec, player_id, reliable_id);
                    my_state.reliable.add_packetid_to_ack(reliable_id, player_id);
                }
                else if(type == message::FORWARDING_RELIABLE_ACK)
                {
                    my_state.reliable.process_ack(fetch);
                }
                else if(type == message::PING_RESPONSE)
                {
                    my_state.process_ping_response(my_server, fetch, store);
                }
                else if(type == message::PING_GAMESERVER)
                {
                    my_state.process_ping_gameserver(my_server, fetch, store);
                }
                else if(type == message::KEEP_ALIVE)
                {
                    fetch.get<decltype(canary_end)>();
                }
                else if(type == message::REQUEST)
                {
                    my_state.process_request(my_server, fetch, store);
                }
                else if(type == message::PACKET_ACK)
                {
                    my_state.process_generic_message(my_server, fetch, store, message::PACKET_ACK);
                }
                else if(type == message::FORWARDING_ORDERED_RELIABLE)
                {
                    int32_t player_id = my_state.sockaddr_to_playerid(store);

                    my_state.reset_player_disconnect_timer(store);

                    my_state.reliable_ordered.handle_forwarding_ordered_reliable(fetch, player_id);
                }
                else if(type == message::FORWARDING_ORDERED_RELIABLE_REQUEST)
                {
                    my_state.reliable_ordered.handle_packet_request(my_server, (const sockaddr*)&store, fetch);
                }
                else if(type == message::FORWARDING_ORDERED_RELIABLE_ACK)
                {
                    my_state.reliable_ordered.handle_ack(fetch);
                }
                else
                {
                    printf("err %i ", type);
                }

                /*if(!once && my_state.gid == 3)
                {
                    printf("pied piping\n");

                    //my_state.reliable.add(test, -1);

                    once = true;
                }*/

                //printf("client %s:%s\n", get_addr_ip(store).c_str(), get_addr_port(store).c_str());

                my_state.reset_player_disconnect_timer(store);
            }
                ///client pushing data to other clients
        }

        std::vector<network_data> reliable_data;
        my_state.reliable_ordered.make_packets_available_into(reliable_data);

        for(network_data& dat : reliable_data)
        {
            std::cout << "dat av " << dat.packet_id << std::endl;

            for(player& p : my_state.player_list)
            {
                if(p.id == my_state.reliable_ordered.get_owner_id_from_packet(dat.object, dat.packet_id))
                    continue;

                my_state.reliable_ordered.forward_data_to(p.sock, (const sockaddr*)&p.store, dat.object, dat.data, p.id);
            }
        }

        reliable_data.clear();

        for(auto& i : my_state.reliable_ordered.receiving_owner_to_packet_info)
        {
            std::vector<packet_request_range> range = i.second.request_incomplete_packets(i.first);

            /*player p = my_state.get_player_from_player_id(i.first);

            for(packet_request_range& ran : range)
            {
                my_state.reliable_ordered.make_packet_request(p.sock, (const sockaddr*)&p.store, ran);
            }*/

            for(packet_request_range& ran : range)
            {
                ///well... this is unfortunate
                for(player& p : my_state.player_list)
                {
                    my_state.reliable_ordered.make_packet_request(p.sock, (const sockaddr*)&p.store, ran);
                }
            }
        }

        for(packet_ack& ack : my_state.reliable_ordered.unacked)
        {
            player p = my_state.get_player_from_player_id(ack.host_player_id);

            my_state.reliable_ordered.make_packet_ack(p.sock, (const sockaddr*)&p.store, ack);

            std::cout << "send_ack\n" << ack.host_player_id << std::endl;
        }

        my_state.reliable_ordered.unacked.clear();

        sf::sleep(sf::milliseconds(1));


        //my_state.tick();

        //my_state.balance_teams();

        //my_state.periodic_team_broadcast();

        /*my_state.periodic_gamemode_stats_broadcast();

        my_state.periodic_respawn_info_update();

        my_state.periodic_player_stats_update();*/

        my_state.cull_disconnected_players();

        my_state.reliable.tick(&my_state);

        ///should do tick ping
        if(my_state.ping_interval_clk.getElapsedTime().asMicroseconds() / 1000.f >= my_state.ping_interval_ms)
        {
            my_state.ping();

            my_state.ping_interval_clk.restart();
        }

        my_state.broadcast_ping_data();

        my_state.packet_clump.tick();
    }
}
