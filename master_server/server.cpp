#include "server.hpp"
#include "network_messages.hpp"

/*void master_server::cull_dead()
{
    for(int i=0; i<servers.size(); i++)
    {
        tcp_sock& fd = servers[i].sock;

        if(sock_readable(fd))
        {
            auto data = tcp_recv(fd);

            if(fd.invalid())
            {
                servers[i].sock.close();
            }
        }

        if(!servers[i].sock.valid())
        {
            printf("gameserver died\n");

            servers[i].sock.close();

            servers.erase(servers.begin() + i);
            i--;
        }
    }
}*/

/*void master_server::tick_all()
{
    for(int i=0; i<servers.size(); i++)
    {
        printf("test tick %i\n", i);
    }
}*/

/*game_server master_server::server_from_sock(tcp_sock& sock, uint32_t port)
{
    std::string ip = sock.get_peer_ip();
    std::string my_port_to_them = sock.get_peer_port();
    std::string their_host_port = std::to_string(port);

    printf("Ip %s:%s joined, they are hosted on port %s\n", ip.c_str(), my_port_to_them.c_str(), their_host_port.c_str());

    ///I need a constructor!
    game_server serv;

    serv.sock = sock;
    serv.address = ip;
    serv.my_port_to_them = my_port_to_them;
    serv.their_host_port = their_host_port;

    return serv;
}*/

/*void master_server::add_server(const game_server& serv)
{
    for(auto& i : servers)
    {
        if(i.address == serv.address && i.their_host_port == serv.their_host_port)
        {
            i = serv;

            printf("eliminating duplicate server\n");

            return;
        }
    }

    servers.push_back(serv);
}

std::vector<char> master_server::get_client_response()
{
    byte_vector vec;

    vec.push_back(canary_start);
    vec.push_back(message::CLIENTRESPONSE);

    int32_t server_nums = servers.size();

    vec.push_back(server_nums);

    for(int i=0; i<server_nums; i++)
    {
        std::string ip = servers[i].address;
        int32_t len = ip.length();

        vec.push_back(len);
        vec.push_string<std::string>(ip.c_str(), len);

        uint32_t port = atoi(servers[i].their_host_port.c_str());

        vec.push_back(port);
    }

    vec.push_back(canary_end);

    return vec.ptr;
}*/
