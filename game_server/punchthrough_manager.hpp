#ifndef PUNCHTHROUGH_MANAGER_HPP_INCLUDED
#define PUNCHTHROUGH_MANAGER_HPP_INCLUDED

#include <SFML/System.hpp>
#include <net/shared.hpp>

struct punchthrough
{
    sf::Clock clk;
    float max_time_s = 5.f;

    sf::Clock send_clock;
    float send_every_s = 0.025f;

    std::string client_ip;
    std::string client_port;

    void construct_from_serialise(serialise& s)
    {
        punchthrough_to_server_data data;

        s.handle_serialise(data, false);

        client_ip = data.client_ip;
        client_port = data.client_port;
    }

    bool expired()
    {
        return (clk.getElapsedTime().asMilliseconds() / 1000.f) > max_time_s;
    }

    bool should_send()
    {
        if((send_clock.getElapsedTime().asMilliseconds() / 1000.f) > send_every_s)
        {
            send_clock.restart();

            return true;
        }

        return false;
    }
};

struct punchthrough_manager
{
    std::vector<punchthrough> to_punch;

    void tick(udp_sock& host)
    {
        for(int i=0; i < to_punch.size(); i++)
        {
            punchthrough& punch = to_punch[i];

            if(punch.expired())
            {
                to_punch.erase(to_punch.begin() + i);
                i--;
                continue;
            }

            if(punch.should_send())
            {
                sockaddr_storage store = get_sockaddr_from(punch.client_ip, punch.client_port);

                std::cout << "sending to " << get_addr_ip(store) << " " << get_addr_port(store) << std::endl;

                punchthrough_packet packet;
                serialise s;
                s.handle_serialise(packet, true);

                udp_send_to(host, s.data, (const sockaddr*)&store);
            }
        }
    }
};

#endif // PUNCHTHROUGH_MANAGER_HPP_INCLUDED
