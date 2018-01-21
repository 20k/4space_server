#include "networking.hpp"

#define GAMESERVER_IP "77.96.132.101"
//#define GAMESERVER_IP "192.168.0.54"

//#define GAMESERVER_IP "127.0.0.1"

void network_state::tick_join_game(float dt_s)
{
    if(my_id != -1)
        return;

    if(!try_join)
        return;

    if(!sock.valid())
    {
        if(try_join_ip == "" || try_join_port == "")
        {
            sock = udp_connect(GAMESERVER_IP, GAMESERVER_PORT);
        }
        else
        {
            sock = udp_connect(try_join_ip, try_join_port);
        }

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
    if(sock.valid())
        sock.close();

    sock.make_invalid();

    my_id = -1;

    try_join = false;
    timeout = timeout_max;
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

///TODO: Convert over to using new serialisation scheme, share data structure with master server
void network_state::handle_master_response()
{
    if(!to_master_sock.valid())
        return;

    sockaddr_storage tstore;

    bool any_read = true;

    while(any_read && sock_readable(to_master_sock))
    {
        auto data = udp_receive_from(to_master_sock, &tstore);

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
                break;


            //while(!sock_writable(my_server)){}

            int32_t type = fetch.get<int32_t>();

            if(type == message::CLIENTRESPONSE)
            {
                ///reset game servers
                game_servers = decltype(game_servers)();

                int32_t server_nums = fetch.get<int32_t>();

                for(int i=0; i < server_nums && i < 255; i++)
                {
                    int32_t string_length = fetch.get<int32_t>();

                    std::vector<char> chars = fetch.get_buf(string_length);

                    uint32_t port = fetch.get<uint32_t>();

                    game_server_info inf;
                    inf.ip = std::string(chars.begin(), chars.end());
                    inf.port = std::to_string(port);

                    game_servers.push_back(inf);
                }

                auto canary = fetch.get<decltype(canary_end)>();
            }
        }
    }
}

struct master_request_gameservers_info : serialisable
{
    decltype(canary_start) can_start = canary_start;
    message::message type = message::CLIENT;
    decltype(canary_end) can_end = canary_end;

    void do_serialise(serialise& s, bool ser)
    {
        s.handle_serialise(can_start, ser);
        s.handle_serialise(type, ser);
        s.handle_serialise(can_end, ser);
    }
};

void network_state::ping_master_for_gameservers()
{
    if(!to_master_sock.valid())
        return;

    master_request_gameservers_info info;

    serialise s;
    s.handle_serialise(info, true);

    udp_send(to_master_sock, s.data);
}

void network_state::tick_ping_master_for_gameservers()
{
    static sf::Clock elapsed;
    static bool once = false;

    if(!once || elapsed.getElapsedTime().asSeconds() > 5)
    {
        elapsed.restart();

        ping_master_for_gameservers();
    }

    once = true;
}

///server should only deal in compressed packets
///compress when we forward in the above function, decompress when we receive after make packets available into
void network_state::tick(double dt_s)
{
    tick_join_game(dt_s);

    handle_master_response();

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
                break;


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


    if(use_keepalive)
    {
        std::vector<keepalive_info*> keepalive_hack{&keepalive};

        static update_strategy keepalive_update;
        keepalive_update.do_update_strategy(dt_s, 1.f, keepalive_hack, *this, 0);
    }
}

void network_state::register_keepalive()
{
    keepalive.explicit_register();
    use_keepalive = true;
}

void network_state::open_socket_to_master_server(const std::string& master_ip, const std::string& master_port)
{
    to_master_sock = udp_connect(master_ip, master_port);
}

std::vector<std::string> network_state::get_gameserver_strings()
{
    std::vector<std::string> ret;

    for(game_server_info& inf : game_servers)
    {
        std::string name;

        name = inf.ip + ":" + inf.port;

        ret.push_back(name);
    }

    return ret;
}

void network_state::try_join_server(int offset)
{
    if(offset < 0 || offset >= game_servers.size())
        return;

    game_server_info& info = game_servers[offset];

    leave_game();

    try_join = true;
    try_join_ip = info.ip;
    try_join_port = info.port;
}

bool network_state::connected_to(int offset)
{
    if(offset < 0 || offset >= game_servers.size())
        return false;

    if(!sock.valid())
        return false;

    if(my_id == -1)
        return false;

    game_server_info& info = game_servers[offset];

    std::string addr = sock.get_peer_ip();
    std::string port = sock.get_peer_port();

    return addr == info.ip && port == info.port;
}
