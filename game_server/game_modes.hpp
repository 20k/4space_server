#ifndef GAME_MODES_HPP_INCLUDED
#define GAME_MODES_HPP_INCLUDED

#include <vector>
#include <string>
#include <vec/vec.hpp>

///this file is more.. map and session data though ;_;
///ie a clusterfuck of unrelated things
namespace game_mode
{
    enum game_mode : int32_t
    {
        FIRST_TO_X,
        FFA,
        ///the man with the golden sword, and in fact outfit entirely
        COUNT
    };

    static std::vector<std::string> names
    {
        "Team Deathmatch",
        "Error"
    };
}

#define TEAM_NUMS 16

typedef game_mode::game_mode game_mode_t;

namespace game_data
{
    const float kills_to_win = 40;
    const float round_default = 3.5 * 60; ///10 seconds for testing
    const float game_over_time = 6; ///seconds
}

struct session_boundaries
{
    int32_t max_kills = game_data::kills_to_win;
    float max_time_ms = game_data::round_default * 1000.f;
};

///team session state
struct session_state
{
    int32_t team_killed[TEAM_NUMS] = {0}; ///how many died on this team
    int32_t team_kills[TEAM_NUMS] = {0}; ///how many this team has killed

    float time_elapsed = 0; ///if i make you a float, i can just pipe these structures directly

    ///implement gamemode functions here, with session_boundaries

    bool game_over(const session_boundaries& bounds);
    std::string get_game_over_string(const session_boundaries& bounds);
    std::string get_current_game_state_string(const session_boundaries& bounds);
};

namespace map_namespace
{
    enum spawn_defs : int
    {
        R = -1 << 16,
        B = -1 << 17//,
        //G = -1 << 18  //GOLD
    };

    static std::vector<spawn_defs> team_list
    {
        R,
        B//,
        //G
    };

    static std::vector<vec3f> map_cube_rotations =
    {
        {0,0,0},
        {M_PI,0,0},
        {M_PI/2,0,0},
        {0,0,M_PI/2},
        {0,0,-M_PI/2},
        {-M_PI/2,0,0},
    };

    ///rotations are defined from the reference plane of the bottom

    /*static std::vector<int>
    map_one
    {
        1,1,1,1,1,1,1,1,1,1,1,
        1,0,0,R,R,R,R,R,0,0,1,
        1,0,0,0,0,0,0,0,0,0,1,
        1,0,0,1,1,1,1,1,0,0,1,
        1,0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,1,0,1,0,0,0,1,
        1,0,0,0,1,0,1,0,0,0,1,
        1,0,0,0,0,0,0,0,0,0,1,
        1,0,0,1,1,1,1,1,0,0,1,
        1,0,0,0,0,0,0,0,0,0,1,
        1,0,0,B,B,B,B,B,0,0,1,
        1,1,1,1,1,1,1,1,1,1,1,
    };*/

    ///need to open up map so we can transfer
    static std::vector<int>
    map_one
    {
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,0,0,0,0,R,R,R,R,R,R,R,R,R,R,0,0,0,0,1,1,1,
        1,1,1,0,0,0,0,R,R,R,R,R,R,R,R,R,R,0,0,0,0,1,1,1,
        1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
        1,1,1,0,0,0,0,2,1,2,1,2,1,2,1,2,1,0,0,0,0,1,1,1,
        1,1,1,0,0,0,0,1,2,1,2,1,2,1,2,1,2,0,0,0,0,1,1,1,
        1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
        1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
        1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
        0,0,0,0,0,0,0,0,0,2,1,0,0,1,2,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,1,0,0,0,2,1,0,0,1,2,0,0,0,1,0,0,0,0,0,
        0,0,0,0,0,1,0,0,0,2,1,0,0,1,2,0,0,0,1,0,0,0,0,0,
        1,1,1,0,0,0,0,0,0,2,1,0,0,1,2,0,0,0,0,0,0,1,1,1,
        1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
        1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
        1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
        1,1,1,0,0,0,0,2,1,2,1,2,1,2,1,2,1,0,0,0,0,1,1,1,
        1,1,1,0,0,0,0,1,2,1,2,1,2,1,2,1,2,0,0,0,0,1,1,1,
        1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
        1,1,1,0,0,0,0,B,B,B,B,B,B,B,B,B,B,0,0,0,0,1,1,1,
        1,1,1,0,0,0,0,B,B,B,B,B,B,B,B,B,B,0,0,0,0,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    };

    static std::vector<std::vector<int>> map_cube_one
    {
        map_one,
        map_one,
        map_one,
        map_one,
        map_one,
        map_one
    };
}

///do not grid based
namespace game_map
{
    //static float scale = 1000.f;
    static float scale = 500.f;
    //static float floor_const =
    #define FLOOR_CONST (bodypart::default_position[bodypart::LFOOT].v[1] - bodypart::scale/3)

    /*static
    std::vector<std::vector<int>> map_list =
    {
        map_namespace::map_one
    };*/

    ///umm. Fix the types?
    static
    std::vector<std::vector<std::vector<int>>> cube_map_list =
    {
        map_namespace::map_cube_one
    };

    static std::vector<vec2i> map_dims =
    {
        {12*2, 12*2}
    };

    static vec2i world_to_point(vec2f world_pos, vec2f dim)
    {
        vec2f centre = dim/2.f;

        vec2f map_scale = world_pos / game_map::scale;

        map_scale = map_scale - 0.5f;

        map_scale = round(map_scale + centre);

        vec2i imap = {(int)map_scale.v[0], (int)map_scale.v[1]};

        imap = clamp(imap, (vec2i){0,0}, (vec2i){(int)dim.v[0], (int)dim.v[1]} - 1);

        return imap;
    }

    static vec2f point_to_world(vec2f local_pos, vec2f dim)
    {
        vec2f w2d = local_pos - dim/2.f;

        vec2f world_pos = w2d + (vec2f){0.5f, 0.5f};

        return world_pos * scale;
    }

    ///universally shared between client and server
    static std::vector<vec2f> get_spawn_positions(map_namespace::spawn_defs team, int map_id)
    {
        std::vector<vec2f> ret;

        ///FIXMEBAD
        const std::vector<int>& my_map = cube_map_list[map_id][0];

        vec2i dim = map_dims[map_id];

        for(int y=0; y<dim.v[1]; y++)
        {
            for(int x=0; x<dim.v[0]; x++)
            {
                int point = my_map[y*dim.v[0] + x];

                if(point != (int)team)
                    continue;

                vec2f wpos = point_to_world({x, y}, {dim.v[0], dim.v[1]});

                ret.push_back(wpos);
            }
        }

        return ret;
    }
}

#endif // GAME_MODES_HPP_INCLUDED
