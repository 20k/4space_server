#ifndef PACKET_CLUMPING_SHARED_HPP_INCLUDED
#define PACKET_CLUMPING_SHARED_HPP_INCLUDED

#include <net/shared.hpp>
#include <map>
#include <vector>

struct net_dest
{
    udp_sock sock;
    sockaddr_storage store;
    std::vector<char> data;
};

struct net_info
{
    udp_sock sock;
    sockaddr_storage store;
};


struct packet_clumper
{
    std::vector<net_dest> destination_to_senddata;
    std::vector<net_info> destinations;

    void add_send_data(udp_sock& sock, sockaddr_storage& store, const std::vector<char>& dat)
    {
        net_dest dest = {sock, store, dat};

        destination_to_senddata.push_back(dest);

        net_info info = {sock, store};

        for(auto& i : destinations)
        {
            if(i.store == info.store)
                return;
        }

        destinations.push_back(info);
    }

    int get_max_packet_data_size()
    {
        return 450;
    }

    void tick()
    {
        for(auto& i : destinations)
        {
            int cur_dest = 0;
            std::vector<std::vector<char>> data;
            data.resize(1);

            for(auto& nd : destination_to_senddata)
            {
                if(data[cur_dest].size() != 0 && (data[cur_dest].size() + nd.data.size()) >= get_max_packet_data_size())
                {
                    cur_dest++;
                    data.resize(cur_dest + 1);
                }

                if(i.store == nd.store)
                {
                    data[cur_dest].insert(data[cur_dest].end(), nd.data.begin(), nd.data.end());
                }
            }

            for(auto& dat : data)
            {
                //while(!sock_writable(i.sock)){}

                udp_send_to(i.sock, dat, (sockaddr*)&i.store);
            }
        }

        destination_to_senddata.clear();
        destinations.clear();
    }
};

#endif // PACKET_CLUMPING_SHARED_HPP_INCLUDED
