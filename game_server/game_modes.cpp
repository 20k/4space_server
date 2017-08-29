#include "game_modes.hpp"

bool session_state::game_over(const session_boundaries& bounds)
{
    for(int i=0; i<TEAM_NUMS; i++)
    {
        if(team_kills[i] >= bounds.max_kills)
            return true;
    }

    if(time_elapsed >= bounds.max_time_ms)
        return true;

    return false;
}

std::string session_state::get_current_game_state_string(const session_boundaries& bounds)
{
    std::string str;

    for(int i=0; i<TEAM_NUMS; i++)
    {
        if(team_kills[i] != 0)
            str = str + "Team " + std::to_string(i) + " kills: " + std::to_string(team_kills[i]) + " out of " + std::to_string(bounds.max_kills) + "\n";
    }

    str = str + std::to_string((int)time_elapsed / 1000.f) + " of " + std::to_string((int)bounds.max_time_ms / 1000.f) + " remaining!";

    return str;
}

std::string session_state::get_game_over_string(const session_boundaries& bounds)
{
    for(int i=0; i<TEAM_NUMS; i++)
    {
        if(team_kills[i] >= bounds.max_kills)
            return "Team " + std::to_string(i) + " wins!";
    }

    return "Its a draw!";
}
