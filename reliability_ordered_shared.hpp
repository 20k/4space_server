#ifndef RELIABILITY_ORDERED_SHARED_HPP_INCLUDED
#define RELIABILITY_ORDERED_SHARED_HPP_INCLUDED

#include "../serialise/serialise.hpp"

inline
void send_join_game(udp_sock& sock)
{
    byte_vector vec;
    vec.push_back(canary_start);
    vec.push_back(message::CLIENTJOINREQUEST);
    vec.push_back(canary_end);

    udp_send(sock, vec.ptr);
}

inline
udp_sock join_game(const std::string& address, const std::string& port)
{
    udp_sock sock = udp_connect(address, port);

    send_join_game(sock);

    return sock;
}

inline
int get_max_packet_size_clientside()
{
    return 450;
}

inline
int get_packet_fragments(int data_size)
{
    int max_data_size = get_max_packet_size_clientside();

    int fragments = ceil((float)data_size / max_data_size);

    return fragments;
}

struct network_object
{
    ///who's sending the data
    serialise_owner_type owner_id = -1;
    serialise_data_type serialise_id = -1;
};

using packet_id_type = int32_t;
using sequence_data_type = int32_t;

struct packet_fragment
{
    sequence_data_type sequence_number = 0;
    serialise data;
};

#pragma pack(1)
struct packet_header
{
    uint32_t current_size;
    int32_t overall_size = 0;
    packet_id_type packet_id = 0;
    sequence_data_type sequence_number = 0;

    uint32_t calculate_size()
    {
        return sizeof(overall_size) + sizeof(packet_id) + sizeof(sequence_number);
    }
};

inline
byte_vector get_fragment(int id, const network_object& no, const std::vector<char>& data, packet_id_type use_packet_id)
{
    int fragments = get_packet_fragments(data.size());

    sequence_data_type sequence_number = 0;

    packet_header header;
    header.current_size = data.size() + header.calculate_size() + sizeof(no);
    header.overall_size = header.current_size;
    header.packet_id = use_packet_id;

    if(fragments == 1)
    {
        byte_vector vec;

        vec.push_back(canary_start);
        vec.push_back(message::FORWARDING_ORDERED_RELIABLE);
        vec.push_back(header);
        vec.push_back<network_object>(no);

        for(auto& i : data)
        {
            vec.push_back(i);
        }

        vec.push_back(canary_end);

        return vec;
    }

    int real_data_per_packet = ceil((float)data.size() / fragments);

    int sent = id * real_data_per_packet;
    int to_send = data.size();

    if(sent >= to_send)
        return byte_vector();

    byte_vector vec;

    int to_send_size = std::min(real_data_per_packet, to_send - sent);

    header.current_size = to_send_size + header.calculate_size() + sizeof(no);
    header.sequence_number = id;

    vec.push_back(canary_start);
    vec.push_back(message::FORWARDING_ORDERED_RELIABLE);

    vec.push_back(header);
    vec.push_back<network_object>(no);

    for(int kk = 0; kk < real_data_per_packet && sent < to_send; kk++)
    {
        vec.push_back(data[sent]);

        sent++;
    }

    vec.push_back(canary_end);

    return vec;
}

struct network_data
{
    network_object object;
    serialise data;
    packet_id_type packet_id;
    bool should_cleanup = false;
    bool processed = false;

    //sf::Clock clk;

    void set_complete()
    {
        processed = true;
        //clk.restart();
    }
};

#pragma pack(1)
struct forward_packet
{
    int32_t canary_first;
    message::message type;
    packet_header header;
    network_object no;
    byte_fetch fetch;
    int32_t canary_second;
};

///packet acks were purely used for rate limiting
///which is extremely wasteful
///maybe just ack every 10th packet or something
///or, ack range every 1 second
struct packet_ack
{
    serialise_owner_type owner_id = 0;
    sequence_data_type sequence_id = 0;
    packet_id_type packet_id = 0;
};

#pragma pack(1)
struct packet_request_range
{
    serialise_owner_type owner_id = 0;
    sequence_data_type sequence_id_start = 0;
    sequence_data_type sequence_id_end = 0;

    packet_id_type packet_id = 0;
};


inline
void move_forward_packet_to_network_data(forward_packet& packet, network_data& out)
{
    out.data = serialise();

    out.object = packet.no;
    out.data.data = std::move(packet.fetch.ptr);
    out.packet_id = packet.header.packet_id;

    /*packet_ack ack;
    ack.owner_id = packet.no.owner_id;
    ack.packet_id = packet.header.packet_id;
    ack.sequence_id = packet.header.sequence_number;*/
}

struct network_packet_fragment_info_send
{
    byte_vector data;

    bool fragment_ack = false;
};

struct network_packet_info_send
{
    ///guaranteed to be empty or complete
    std::map<sequence_data_type, network_packet_fragment_info_send> fragment_info;

    bool full_packet_ack = false;

    std::vector<byte_vector> get_fragments(int start, int fin)
    {
        std::vector<byte_vector> ret;

        for(int i=start; i < fin && i < fragment_info.size(); i++)
        {
            ret.push_back(fragment_info[i].data);
        }

        return ret;
    }
};

struct network_owner_info_send
{
    std::map<packet_id_type, network_packet_info_send> packet_info;

    void store_packet_fragment(packet_id_type pid, sequence_data_type sid, byte_vector& vec)
    {
        packet_info[pid].fragment_info[sid].data = vec;
    }

    std::vector<byte_vector> get_fragments(packet_id_type pid, int start, int fin)
    {
        return packet_info[pid].get_fragments(start, fin);
    }
};

struct network_packet_fragment_info_recv
{
    byte_vector data;
};

struct network_packet_info_recv
{
   // std::map<sequence_data_type, network_packet_fragment_info_recv> fragment_info;

    serialise_data_type serialise_id = 0;
    int packet_fragments_num = 1;

    std::vector<packet_fragment> fragments;
    std::map<sequence_data_type, bool> has_fragment;

    bool has_full_packet = false;

    int from = -1;

    //sf::Clock last_requested_at;

    void sort_fragments()
    {
        std::sort(fragments.begin(), fragments.end(),
                  [](packet_fragment& p1, packet_fragment& p2)
                  {
                      return p1.sequence_number < p2.sequence_number;
                  });
    }

    /*bool could_request()
    {
        float max_time_s = 0.2f;

        return (last_requested_at.getElapsedTime().asMicroseconds() / 1000. / 1000.) > max_time_s;
    }*/

    bool has_all_fragments()
    {
        return fragments.size() == packet_fragments_num;
    }

    std::vector<packet_request_range> get_requests(serialise_owner_type sid, packet_id_type pid)
    {
        std::vector<packet_request_range> ret;

        packet_request_range next_to_send;
        next_to_send.sequence_id_start = -1;

        next_to_send.owner_id = sid;
        next_to_send.packet_id = pid;

        for(int i=0; i < packet_fragments_num; i++)
        {
            if(!has_fragment[i])
            {
                ///we haven't received this fragment
                ///and we haven't started a range
                if(next_to_send.sequence_id_start == -1)
                {
                    ///start range
                    next_to_send.sequence_id_start = i;
                    continue;
                }

                ///we haven't received this fragment and we've started a range
                next_to_send.sequence_id_end = i;
            }
            else
            {
                ///we've started a range and we have received this fragment
                ///terminmation condition for range
                ///push new range, then reset range to uninitialised
                if(next_to_send.sequence_id_start != -1)
                {
                    ///one past end
                    next_to_send.sequence_id_end += 1;

                    ret.push_back(next_to_send);

                    next_to_send.sequence_id_start = -1;
                }
            }
        }

        next_to_send.sequence_id_end = packet_fragments_num;

        ///we started a range that was unfinished, finish manually
        if(next_to_send.sequence_id_start != -1)
        {
            ret.push_back(next_to_send);
        }

        return ret;
    }

    /*void reset_request()
    {
        last_requested_at.restart();
    }*/
};

struct network_owner_info_recv
{
    std::map<packet_id_type, network_packet_info_recv> packet_info;
    std::map<packet_id_type, sf::Clock> request_timers;

    std::deque<forward_packet> full_packets;
    std::map<packet_id_type, bool> made_available;

    packet_id_type last_received = -1;

    void set_from(packet_id_type pid, int tf)
    {
        packet_info[pid].from = tf;
    }

    int get_from(packet_id_type pid)
    {
        return packet_info[pid].from;
    }

    void store_serialise_id(packet_id_type pid, serialise_data_type sid)
    {
        packet_info[pid].serialise_id = sid;
    }

    void store_expected_packet_fragments_num(packet_id_type pid, int num)
    {
        packet_info[pid].packet_fragments_num = num;
    }

    void try_add_packet_fragment(packet_id_type pid, packet_fragment& frag)
    {
        if(packet_info[pid].has_fragment[frag.sequence_number])
            return;

        packet_info[pid].fragments.push_back(frag);

        packet_info[pid].has_fragment[frag.sequence_number] = true;
    }

    int get_current_packet_fragments_num(packet_id_type pid)
    {
        return packet_info[pid].fragments.size();
    }

    void sort_fragments(packet_id_type pid)
    {
        packet_info[pid].sort_fragments();
    }

    void sort_received_packets()
    {
        std::sort(full_packets.begin(), full_packets.end(),
                  [](forward_packet& p1, forward_packet& p2){return p1.header.packet_id < p2.header.packet_id;});
    }

    ///So. We need to sort received packets, request any that we dont have

    std::vector<packet_fragment>& get_fragments(packet_id_type pid)
    {
        return packet_info[pid].fragments;
    }

    bool has_full_packet(packet_id_type pid)
    {
        return packet_info[pid].has_full_packet;
    }

    void add_full_packet(forward_packet& pack)
    {
        if(has_full_packet(pack.header.packet_id))
            return;

        if(last_received == -1)
        {
            last_received = pack.header.packet_id-1;
        }

        full_packets.push_back(pack);

        packet_info[pack.header.packet_id].has_full_packet = true;
    }

    void make_full_packets_available_into(std::vector<network_data>& into)
    {
        sort_received_packets();

        for(int i=0; i < full_packets.size(); i++)
        {
            forward_packet& packet = full_packets[i];

            if(made_available[packet.header.packet_id])
                continue;

            bool add = false;

            if(packet.header.packet_id <= last_received)
            {
                add = true;

                std::cout << "warning mixed packets" << std::endl;
            }

            if(packet.header.packet_id == last_received + 1)
            {
                add = true;

                last_received = packet.header.packet_id;
            }

            if(add)
            {
                made_available[packet.header.packet_id] = true;

                network_data out;
                move_forward_packet_to_network_data(packet, out);

                into.push_back(out);

                request_timers.erase(packet.header.packet_id);
                full_packets.erase(full_packets.begin() + i);
                i--;
                continue;
            }
        }
    }

    bool received_any_fragments(packet_id_type pid)
    {
        return packet_info.find(pid) != packet_info.end();
    }

    bool should_request_packet(packet_id_type pid)
    {
        float min_time_s = 0.2f;

        return (request_timers[pid].getElapsedTime().asMicroseconds() / 1000. / 1000.) > min_time_s;
    }

    void request_packet(packet_id_type pid)
    {
        request_timers[pid].restart();
    }

    std::vector<packet_request_range> request_incomplete_packets(serialise_owner_type oid)
    {
        sort_received_packets();

        std::vector<packet_request_range> ret;

        ///not maximum number of requests
        ///that'll be throttled on sending i guess?
        int max_request_packets = 20;

        int last_fragment = last_received + 1;

        if(full_packets.size() != 0)
        {
            last_fragment = full_packets.back().header.packet_id;
        }

        int num = 0;

        for(packet_id_type i=last_received+1; i < last_fragment; i++)
        {
            if(should_request_packet(i))
            {
                if(received_any_fragments(i))
                {
                    std::vector<packet_request_range> ranges = packet_info[i].get_requests(oid, i);

                    num += ranges.size();

                    for(auto& kk : ranges)
                    {
                        ret.push_back(kk);
                    }
                }
                else
                {
                    packet_request_range prr;
                    prr.owner_id = oid;
                    prr.packet_id = i;
                    prr.sequence_id_end = 1;
                    prr.sequence_id_start = 0;

                    ret.push_back(prr);

                    num++;
                }

                request_packet(i);

                if(num >= max_request_packets)
                    break;
            }
        }

        return ret;
    }
};

inline
forward_packet decode_forward(byte_fetch& fetch)
{
    forward_packet ret;
    ret.canary_first = canary_start;
    ret.type = message::FORWARDING_ORDERED_RELIABLE;
    ret.header = fetch.get<packet_header>();
    ret.no = fetch.get<network_object>();

    for(int i=0; i<ret.header.current_size - ret.header.calculate_size() - sizeof(network_object); i++)
    {
        ret.fetch.ptr.push_back(fetch.get<uint8_t>());
    }

    ret.canary_second = fetch.get<decltype(canary_end)>();

    return ret;
}

/*#pragma pack(1)
struct packet_request
{
    serialise_owner_type owner_id = 0;
    sequence_data_type sequence_id = 0;
    packet_id_type packet_id = 0;
    serialise_data_type serialise_id;
};*/

struct network_reliable_ordered
{
    bool serv = false;
    packet_id_type next_packet_id = 0;
    packet_id_type last_confirmed_packet_id = 0;

    std::map<serialise_owner_type, network_owner_info_send> sending_owner_to_packet_info;
    std::map<serialise_owner_type, network_owner_info_recv> receiving_owner_to_packet_info;

public:

    int get_owner_id_from_packet(network_object& no, packet_id_type pid)
    {
        return receiving_owner_to_packet_info[no.owner_id].get_from(pid);
    }

    void init_server(){serv = true;}
    void init_client(){serv = false;}

    bool is_server(){return serv;}
    bool is_client(){return !serv;}

    void make_packet_request(udp_sock& sock, const sockaddr* store, packet_request_range& request)
    {
        byte_vector vec;
        vec.push_back(canary_start);
        vec.push_back(message::FORWARDING_ORDERED_RELIABLE_REQUEST);
        vec.push_back(request);
        vec.push_back(canary_end);

        //while(!sock_writable(sock)) {}

        portable_send(sock, store, vec.ptr);
    }

    void request_all_packets_client(udp_sock& sock, const sockaddr* store)
    {
        for(auto& i : receiving_owner_to_packet_info)
        {
            std::vector<packet_request_range> range = i.second.request_incomplete_packets(i.first);

            for(packet_request_range& ran : range)
            {
                make_packet_request(sock, store, ran);
            }
        }
    }

    void portable_send(udp_sock& sock, const sockaddr* store, const std::vector<char>& data)
    {
        if(is_client())
            udp_send(sock, data);

        if(is_server())
            udp_send_to(sock, data, store);
    }

    void handle_packet_request(udp_sock& sock, const sockaddr* store, byte_fetch& fetch)
    {
        packet_request_range range = fetch.get<packet_request_range>();

        int32_t found_canary_end = fetch.get<decltype(canary_end)>();

        if(found_canary_end != canary_end)
        {
            printf("bad canary in handle packet request");
            return;
        }

        int max_send = 20;

        std::vector<byte_vector> dat = sending_owner_to_packet_info[range.owner_id].get_fragments(range.packet_id, range.sequence_id_start, range.sequence_id_end);

        if(dat.size() > max_send)
        {
            dat.resize(max_send);
        }

        for(byte_vector& f2 : dat)
        {
            portable_send(sock, store, f2.ptr);
        }
    }

    ///Hmm. This should work for sending to clients as well?
    ///We probably don't want to use broadcasting
    void forward_data_to(udp_sock& sock, const sockaddr* store, const network_object& no, serialise& s)
    {
        int max_to_send = 20;

        int fragments = get_packet_fragments(s.data.size());

        bool should_slowdown = false;

        /*for(auto& i : last_unconfirmed_packet)
        {
            if(disconnected(i.first))
                continue;

            if(i.second < (int)packet_id - 2000)
                should_slowdown = true;
        }

        if(should_slowdown)
            max_to_send = 1;*/

        for(int i=0; i<fragments; i++)
        {
            byte_vector frag = get_fragment(i, no, s.data, next_packet_id);

            ///no.owner_id is.. always me here?
            sending_owner_to_packet_info[no.owner_id].store_packet_fragment(next_packet_id, i, frag);

            if(i < max_to_send)
            {
                //while(!sock_writable(sock)) {}

                //udp_send_to(sock, frag.ptr, store);

                portable_send(sock, store, frag.ptr);
            }
        }

        next_packet_id = next_packet_id + 1;
    }

    void handle_forwarding_ordered_reliable(byte_fetch& fetch, int from_id)
    {
        forward_packet packet = decode_forward(fetch);

        packet_header header = packet.header;

        network_object no = packet.no;

        int real_overall_data_length = header.overall_size - header.calculate_size() - sizeof(no);

        int packet_fragments = get_packet_fragments(real_overall_data_length);

        network_owner_info_recv& receiving_data = receiving_owner_to_packet_info[no.owner_id];

        receiving_data.set_from(header.packet_id, from_id);

        receiving_data.store_serialise_id(header.packet_id, no.serialise_id);

        //disconnection_timer[no.owner_id].restart();

        if(packet_fragments > 1)
        {
            receiving_data.store_expected_packet_fragments_num(header.packet_id, packet_fragments);

            ///INSERT PACKET INTO PACKETS
            packet_fragment next;
            next.sequence_number = header.sequence_number;

            //if((header.sequence_number % 100) == 0)
            if(header.sequence_number > 400 && (header.sequence_number % 1000) == 0)
            {
                std::cout << header.sequence_number << " ";
                //std::cout << " " << packets.size() << std::endl;
                std::cout << receiving_data.get_current_packet_fragments_num(header.packet_id) << std::endl;
                std::cout << packet_fragments << std::endl;
            }

            next.data.data = packet.fetch.ptr;

            receiving_data.try_add_packet_fragment(header.packet_id, next);

            int current_received_fragments = receiving_data.get_current_packet_fragments_num(header.packet_id);

            if(current_received_fragments == packet_fragments)
            {
                receiving_data.sort_fragments(header.packet_id);

                std::vector<packet_fragment>& packets = receiving_data.get_fragments(header.packet_id);

                serialise s;

                for(packet_fragment& packet : packets)
                {
                    s.data.insert(std::end(s.data), std::begin(packet.data.data), std::end(packet.data.data));

                    packet.data.data.clear();
                }

                ///pipe back a response?
                if(s.data.size() > 0)
                {
                    if(!receiving_data.has_full_packet(packet.header.packet_id))
                    {
                        packet.fetch = byte_fetch();
                        packet.fetch.ptr = std::move(s.data);

                        forward_packet full_forward = packet;

                        receiving_data.add_full_packet(full_forward);
                    }
                }
            }
        }
        else
        {
            if(!receiving_data.has_full_packet(header.packet_id))
            {
                forward_packet full_forward = packet;

                receiving_data.add_full_packet(full_forward);
            }
        }

        auto found_end = packet.canary_second;

        if(found_end != canary_end)
        {
            printf("forwarding error\n");
        }
    }

    void make_packets_available_into(std::vector<network_data>& into)
    {
        for(auto& i : receiving_owner_to_packet_info)
        {
            i.second.make_full_packets_available_into(into);
        }
    }

    ///so
    ///can currently non reliably send packets to server
    ///then decode received packets correctly, including fragments
    ///next, we need to make received packets 'available' in an ordered way
    ///and set up piping through the server
    ///and then... set up reliability
};


#endif // RELIABILITY_ORDERED_SHARED_HPP_INCLUDED
