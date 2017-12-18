#include "networking.hpp"

#define GAMESERVER_IP "77.96.132.101"
//#define GAMESERVER_IP "192.168.0.54"

void network_state::tick_join_game(float dt_s)
{
    if(my_id != -1)
        return;

    if(!try_join)
        return;

    if(!sock.valid())
    {
        //sock = udp_connect("77.96.132.101", GAMESERVER_PORT);
        sock = udp_connect(GAMESERVER_IP, GAMESERVER_PORT);
        //sock_set_non_blocking(sock, 1);
    }

    timeout += dt_s;

    if(timeout > timeout_max)
    {
        send_join_game(sock);

        timeout = 0;
    }
}

void network_state::leave_game()
{
    sock.close();

    my_id = -1;

    try_join = false;
}

serialisable* network_state::get_serialisable(serialise_host_type& host_id, serialise_data_type& serialise_id)
{
    return serialise_data_helper::host_to_id_to_pointer[host_id][serialise_id];
}

bool network_state::connected()
{
    return (my_id != -1) && (sock.valid());
}

bool network_state::owns(serialisable* s)
{
    return s->owned_by_host;
}

void network_state::claim_for(serialisable* s, serialise_host_type new_host)
{
    if(s == nullptr)
        return;

    if(new_host == my_id)
    {
        s->owned_by_host = true;
    }
    else
    {
        s->owned_by_host = false;
    }
}

void network_state::claim_for(serialisable& s, serialise_host_type new_host)
{
    if(new_host == my_id)
    {
        s.owned_by_host = true;
    }
    else
    {
        s.owned_by_host = false;
    }
}

void network_state::forward_data(const network_object& no, serialise& s)
{
    if(!connected())
        return;

    reliable_ordered.forward_data_to(sock, (const sockaddr*)&store, no, s);
}

///server should only deal in compressed packets
///compress when we forward in the above function, decompress when we receive after make packets available into
void network_state::tick(double dt_s)
{
    tick_join_game(dt_s);

    if(!sock.valid())
        return;

    reliable_ordered.make_packets_available_into(available_data);

    bool any_read = true;

    while(any_read && sock_readable(sock))
    {
        sockaddr_storage tstore;

        auto data = udp_receive_from(sock, &tstore);

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

            if(type == message::CLIENTJOINACK)
            {
                int32_t recv_id = fetch.get<int32_t>();

                int32_t canary_found = fetch.get<int32_t>();

                if(canary_found == canary_end)
                    my_id = recv_id;
                else
                {
                    printf("err in CLIENTJOINACK\n");
                }

                std::cout << recv_id << std::endl;
            }

            if(type == message::PING_DATA)
            {
                int num = fetch.get<int32_t>();

                for(int i=0; i<num; i++)
                {
                    int pid = fetch.get<int32_t>();
                    float ping = fetch.get<float>();
                }

                int32_t found_end = fetch.get<decltype(canary_end)>();

                if(found_end != canary_end)
                {
                    printf("err in PING_DATA\n");
                }
            }

            if(type == message::PING)
            {
                fetch.get<decltype(canary_end)>();
            }

            ///so the server does seem to respond to packet requests
            ///so investigate this next
            ///check owner_ids?
            if(type == message::FORWARDING_ORDERED_RELIABLE)
            {
                //std::cout << "did\n";

                reliable_ordered.handle_forwarding_ordered_reliable(fetch);
            }

            if(type == message::FORWARDING_ORDERED_RELIABLE_REQUEST)
            {
                reliable_ordered.handle_packet_request(sock, (const sockaddr*)&store, fetch);

                //printf("greq ");
            }

            if(type == message::FORWARDING_ORDERED_RELIABLE_ACK)
            {
                //std::cout << "got ack\n";

                reliable_ordered.handle_ack(fetch);
            }
        }
    }

    reliable_ordered.request_all_packets(sock, (const sockaddr*)&store);
    reliable_ordered.process_acks(sock, (const sockaddr*)&store);
}
