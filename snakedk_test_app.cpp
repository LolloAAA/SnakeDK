#include <iostream>
#include <conio.h>

#include "snake_dk.h"

void polling_for_keyboard_input()
{
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

int main()
{
    const int refresh_time_in_ms = 200;
    snake_dk_api::start_game(20, 25, refresh_time_in_ms);

    std::thread keyboard_thread(polling_for_keyboard_input);

    while (true)
    {
        std::vector<int> map;
        int rows;
        int cols;
        snake_dk_api::get_game_field_vector(map, cols, rows);

        int idx = 0;
        system("cls");
        for (auto row = 0; row < rows; ++row)
        {
            for (auto col = 0; col < cols; ++col)
            {
                std::cout << (map[idx] == 0 ? "_" : std::to_string(map[idx])) << " ";
                ++idx;
            }
            std::cout << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(refresh_time_in_ms));
    }

    if (keyboard_thread.joinable()) keyboard_thread.join();
}
