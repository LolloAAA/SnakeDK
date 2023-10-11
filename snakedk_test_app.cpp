#include <iostream>
#include <conio.h>

#include "snake_dk.h"

void test_callback(int* t_ret_map, int t_ret_map_w, int t_ret_map_h)
{
    system("cls");

    const int rows = t_ret_map_w;
    const int cols = t_ret_map_h;
    
    const std::vector<int> map(t_ret_map, t_ret_map + (rows * cols));

    int idx = 0;
    for (auto row = 0; row < rows; ++row)
    {
        for (auto col = 0; col < cols; ++col)
        {
            std::cout << (map[idx] == 0 ? "_" : std::to_string(map[idx])) << " ";
            ++idx;
        }
        std::cout << std::endl;
    }
}

int main()
{
    snake_dk_api::set_game_field_callback(test_callback);

    const int refresh_time_in_ms = 200;
    snake_dk_api::start_game(5, 2, refresh_time_in_ms);

    while (true)
    {
        const char key = _getch();
        
        snake_dk_api::eSNAKE_DIRECTION new_direction;

        switch (key)
        {
        case 'w': { new_direction = snake_dk_api::eSNAKE_DIRECTION::TOP;      break; }
        case 's': { new_direction = snake_dk_api::eSNAKE_DIRECTION::DOWN;     break; }
        case 'a': { new_direction = snake_dk_api::eSNAKE_DIRECTION::RIGHT;    break; }
        case 'd': { new_direction = snake_dk_api::eSNAKE_DIRECTION::LEFT;     break; }
        default:  {                                                        continue; }
        }

        snake_dk_api::change_snake_direction(new_direction);
    }
}
