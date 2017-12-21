#ifndef NETWORKING_HPP_INCLUDED
#define NETWORKING_HPP_INCLUDED

#include <net/shared.hpp>
#include "reliability_ordered_shared.hpp"
#include "../serialise/serialise.hpp"
#include <thread>
#include <mutex>

///intent: Make this file increasingly more general so eventually we can port it between projects

///for the moment, do poor quality networking
///where its fully determined in advanced what will be networked out of systems
///per network tick
///granularity is on the level of functions that have a do_serialise method
///get full state transfer working first
///we can probably fix 'bad' networking with a combination of lzma, interpolation, updating state when necessary
///(ie on ship hit update its stats to ensure it doesn't take 1 second for it to update)
///very infrequently networking stuff the clients aren't looking at

struct network_state
{
    int my_id = -1;
    udp_sock sock;
    sockaddr_storage store;
    bool have_sock = false;
    bool try_join = false;

    float timeout = 5;
    float timeout_max = 5;

    network_reliable_ordered reliable_ordered;

    void tick_join_game(float dt_s);

public:
    //unused
    void leave_game();

///THIS IS THE WHOLE EXPOSED API
    bool connected();

    bool owns(serialisable* s);

    void claim_for(serialisable* s, serialise_host_type new_host);
    void claim_for(serialisable& s, serialise_host_type new_host);

    std::vector<network_data> available_data;

    serialisable* get_serialisable(serialise_host_type& host_id, serialise_data_type& serialise_id);

    void forward_data(const network_object& no, serialise& s);

    void tick(double dt_s);
};

struct update_strategy
{
    int num_updated = 0;

    double time_elapsed_s = 0;

    template<typename T>
    void update(T* t, network_state& net_state, int send_mode)
    {
        serialise_data_helper::send_mode = send_mode;
        serialise_data_helper::ref_mode = 0;

        serialise ser;
        ///because o is a pointer, we allow the stream to force decode the pointer
        ///if o were a non ptr with a do_serialise method, this would have to be false
        //ser.allow_force = true;

        ser.default_owner = net_state.my_id;

        serialise_host_type host_id = net_state.my_id;

        if(t->host_id != -1)
        {
            host_id = t->host_id;
        }

        ser.handle_serialise(serialise_data_helper::send_mode, true);
        ser.handle_serialise(host_id, true);
        //ser.force_serialise(t, true);

        serialise ser_temp;
        ser_temp.default_owner = net_state.my_id;
        ser_temp.force_serialise(t, true);

        if(ser_temp.data.size() == 0)
        {
            t->force_send = false;
            return;
        }

        ser.data.insert(ser.data.end(), ser_temp.data.begin(), ser_temp.data.end());

        network_object no;

        no.owner_id = net_state.my_id;
        no.serialise_id = t->serialise_id;

        net_state.forward_data(no, ser);

        t->force_send = false;
    }

    template<typename T>
    void enforce_cleanups_sent(const std::vector<T*>& to_manage, network_state& net_state, int send_mode, const std::vector<int>& sent)
    {
        for(int i=0; i<to_manage.size(); i++)
        {
            if((to_manage[i]->cleanup || to_manage[i]->dirty || to_manage[i]->force_send) && sent[i] == 0)
            {
                update(to_manage[i], net_state, send_mode);
            }
        }
    }

    template<typename T>
    void do_update_strategy(float dt_s, float time_between_full_updates, const std::vector<T*>& to_manage, network_state& net_state, int send_mode)
    {
        if(to_manage.size() == 0)
            return;

        std::vector<int> updated;
        updated.resize(to_manage.size());

        if(time_elapsed_s > time_between_full_updates)
        {
            for(int i=num_updated; i<to_manage.size(); i++)
            {
                T* t = to_manage[i];

                update(t, net_state, send_mode);

                updated[i] = 1;
            }

            time_elapsed_s = 0.;
            num_updated = 0;

            enforce_cleanups_sent(to_manage, net_state, send_mode, updated);

            return;
        }

        float frac = time_elapsed_s / time_between_full_updates;

        frac = clamp(frac, 0.f, 1.f);

        int num_to_update_to = frac * (float)to_manage.size();

        num_to_update_to = clamp(num_to_update_to, 0, (int)to_manage.size());

        for(; num_updated < num_to_update_to; num_updated++)
        {
            T* t = to_manage[num_updated];

            update(t, net_state, send_mode);

            updated[num_updated] = 1;
        }

        enforce_cleanups_sent(to_manage, net_state, send_mode, updated);

        time_elapsed_s += dt_s;
    }
};


#endif // NETWORKING_HPP_INCLUDED
