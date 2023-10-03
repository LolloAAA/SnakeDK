#ifndef SNAKE_DK_H
#define SNAKE_DK_H

#include <list>
#include <memory>
#include <thread>
#include <vector>
#include <iostream>
#include <random>
#include <mutex>
#include <string>

#define EMPTY_FILED_TOKEN   0
#define SNAKE_HEAD_TOKEN    2
#define SNAKE_BODY_TOKEN    1
#define FOOD_TOKEN          3

namespace snake_dk_api
{
    enum class eSNAKE_DIRECTION
    {
        TOP,
        DOWN,
        LEFT,
        RIGHT
    };
}

namespace snake_dk_details
{
    class FieldBlock
    {
        int m_x = 0;
        int m_y = 0;
    public:
        FieldBlock() {}
        FieldBlock(const int t_x, const int t_y)
        {
            m_x = t_x;
            m_y = t_y;
        }
    
        int get_x() { return m_x; }
        int get_y() { return m_y; }
    
        void update_position(const int t_new_x, const int t_new_y)
        {
            m_x = t_new_x;
            m_y = t_new_y;
        }
    };
    
    typedef FieldBlock Food;
    typedef FieldBlock SnakeBlock;
    
    class Snake
    {
        std::list<SnakeBlock>           m_body              = {};
        snake_dk_api::eSNAKE_DIRECTION  m_last_direction    = snake_dk_api::eSNAKE_DIRECTION::RIGHT;
        std::mutex                      m_dir_mutex;
    
    public:
        Snake(){ }
    
        std::list<SnakeBlock>   get_all_snake()     { return m_body;            }
        SnakeBlock              get_snake_head()    { return (*m_body.begin()); }
    
        void add_token_to_snake_body(const SnakeBlock t_token_to_add = SnakeBlock()) { m_body.push_back(t_token_to_add); }
    
        void update_snake_head(SnakeBlock& t_snake_head, const int t_field_w, const int t_field_h)
        {
            const std::lock_guard<std::mutex> lock(m_dir_mutex);
    
            const auto curr_x = t_snake_head.get_x();
            const auto curr_y = t_snake_head.get_y();
    
            int new_x = curr_x;
            int new_y = curr_y;
    
            switch (m_last_direction)
            {
                case snake_dk_api::eSNAKE_DIRECTION::TOP:   { new_y = curr_y - 1; break; }
                case snake_dk_api::eSNAKE_DIRECTION::DOWN:  { new_y = curr_y + 1; break; }
                case snake_dk_api::eSNAKE_DIRECTION::LEFT:  { new_x = curr_x + 1; break; }
                case snake_dk_api::eSNAKE_DIRECTION::RIGHT: { new_x = curr_x - 1; break; }
                default:                                    {                     break; }
            }
            
            // Handle map mirroring
                 if (new_x < 0)          { new_x = t_field_w - 1;   }
            else if (new_x >= t_field_w) { new_x = 0;               }
    
                 if (new_y < 0)          { new_y = t_field_h - 1;   }
            else if (new_y >= t_field_h) { new_y = 0;               }
    
            t_snake_head.update_position(new_x, new_y);
        }
    
        void update_snake_body(const int t_field_w, const int t_field_h)
        {
            const auto head             = m_body.begin();
            const auto last_body_token  = std::prev(m_body.end());
    
            std::list<SnakeBlock>::iterator it;
            for (it = last_body_token; it != head; --it)
            {
                const auto previous_body_token = std::prev(it);
                it->update_position(previous_body_token->get_x(), previous_body_token->get_y());
            }
    
            // Update head according to current direction
            update_snake_head(*head, t_field_w, t_field_h);
        }
    
        bool are_opposite_directions(const snake_dk_api::eSNAKE_DIRECTION t_first_direction, const snake_dk_api::eSNAKE_DIRECTION t_second_direction)
        {
            if (t_first_direction == snake_dk_api::eSNAKE_DIRECTION::DOWN  && t_second_direction == snake_dk_api::eSNAKE_DIRECTION::TOP)   return true;
            if (t_first_direction == snake_dk_api::eSNAKE_DIRECTION::TOP   && t_second_direction == snake_dk_api::eSNAKE_DIRECTION::DOWN)  return true;
            if (t_first_direction == snake_dk_api::eSNAKE_DIRECTION::LEFT  && t_second_direction == snake_dk_api::eSNAKE_DIRECTION::RIGHT) return true;
            if (t_first_direction == snake_dk_api::eSNAKE_DIRECTION::RIGHT && t_second_direction == snake_dk_api::eSNAKE_DIRECTION::LEFT)  return true;

            return false;
        }

        void update_snake_direction(const snake_dk_api::eSNAKE_DIRECTION t_new_direction)
        {
            const std::lock_guard<std::mutex> lock(m_dir_mutex);

            if (m_body.size() > 1 && are_opposite_directions(m_last_direction, t_new_direction)) return;

            m_last_direction = t_new_direction;
        }
    };
    
    class GameField
    {
        int                             m_w               = 0;
        int                             m_h               = 0;
        int                             m_ms_refresh_rate = 0;
    
        std::vector<std::vector<int>>   m_field_matrix;
        std::mutex                      m_field_mutex;
        std::thread                     m_refresh_field_thread;
    
        Snake                           m_snake;
        Food                            m_food;
    
        bool m_end_refresh = false;
    
    public:
        GameField() {}
        GameField(const int t_w, const int t_h, const int t_ms_refresh_rate)
        {
            m_w                 = t_w;
            m_h                 = t_h;
            m_ms_refresh_rate   = t_ms_refresh_rate;
            m_field_matrix      = std::vector<std::vector<int>>(m_h, std::vector<int>(m_w));
    
            // Set the snake-head and the foor block
            {
                int food_x, food_y;
                get_empty_field_position(food_x, food_y);
    
                m_food.update_position(food_x, food_y);
    
                const int half_field_w = get_field_w() / 2;
                const int half_field_h = get_field_h() / 2;
                const SnakeBlock snake_head = SnakeBlock(half_field_w, half_field_h);
                m_snake.add_token_to_snake_body(snake_head);
            }
    
            m_refresh_field_thread = std::thread(&GameField::refresh_field_thread, this);
        }
    
        ~GameField()
        {
            if (m_refresh_field_thread.joinable())
            {
                m_refresh_field_thread.join();
            }
        }
    
        int get_field_w() { return m_w; }
        int get_field_h() { return m_h; }
    
        void get_game_field_as_vector(std::vector<int>& t_ret_map, int& t_ret_map_w, int& t_ret_map_h)
        {
            const std::lock_guard<std::mutex> lock(m_field_mutex);
    
            const int map_width     = get_field_w();
            const int map_height    = get_field_h();
            const int vector_size   = map_width * map_height;
    
            std::vector<int> matrix_in_vector_form(vector_size);
    
            int idx = 0;
            for (auto row = 0; row < map_height; ++row)
            {
                for (auto col = 0; col < map_width; ++col)
                {
                    matrix_in_vector_form[idx] = m_field_matrix[row][col];
                    ++idx;
                }
            }
    
            t_ret_map   = matrix_in_vector_form;
            t_ret_map_w = map_width;
            t_ret_map_h = map_height;
        }
    
        void update_game_field()
        {
            const std::lock_guard<std::mutex> lock(m_field_mutex);
    
            for (auto row = 0; row < get_field_h(); ++row)
            {
                for (auto col = 0; col < get_field_w(); ++col)
                {
                    m_field_matrix[row][col] = EMPTY_FILED_TOKEN;
                }
            }
    
            m_field_matrix[m_food.get_y()][m_food.get_x()] = FOOD_TOKEN;
    
            for (SnakeBlock& snake : m_snake.get_all_snake())
            {
                m_field_matrix[snake.get_y()][snake.get_x()] = SNAKE_BODY_TOKEN;
            }
    
            m_field_matrix[m_snake.get_snake_head().get_y()][m_snake.get_snake_head().get_x()] = SNAKE_HEAD_TOKEN;
        }
    
        void get_empty_field_position(int& t_ret_x, int& t_ret_y)
        {
            std::random_device rd;
            const auto seed = rd();
            std::mt19937 rng(seed);
    
            std::uniform_int_distribution<uint32_t> distribuition_x(0, this->get_field_w() - 1);
            std::uniform_int_distribution<uint32_t> distribuition_y(0, this->get_field_h() - 1);
    
            int tmp_x = 0;
            int tmp_y = 0;
    
            bool position_occupied_by_snake = false;
    
            do
            {
                tmp_x = distribuition_x(rd);
                tmp_y = distribuition_y(rd);
    
                for (FieldBlock& snake : m_snake.get_all_snake())
                {
                    position_occupied_by_snake = (tmp_x == snake.get_x() && tmp_y == snake.get_y());
                }
            }
            while (position_occupied_by_snake);
    
            t_ret_x = tmp_x;
            t_ret_y = tmp_y;
        }
    
        bool check_for_collision()
        {
            SnakeBlock snake_head = m_snake.get_snake_head();
    
            if (snake_head.get_x() != m_food.get_x()) return false;
            if (snake_head.get_y() != m_food.get_y()) return false;
    
            return true;
        }
        
        void change_snake_direction(const snake_dk_api::eSNAKE_DIRECTION t_new_direction) { m_snake.update_snake_direction(t_new_direction); }

        void increase_snake() { m_snake.add_token_to_snake_body(); }
    
        void refresh_field_thread()
        {
            while (!m_end_refresh)
            {
                const bool collision_occurred = check_for_collision();
                if (collision_occurred)
                {
                    increase_snake();
    
                    int new_food_x, new_food_y;
                    get_empty_field_position(new_food_x, new_food_y);
    
                    m_food.update_position(new_food_x, new_food_y);
                }
    
                m_snake.update_snake_body(m_w, m_h);
    
                update_game_field();
    
                std::this_thread::sleep_for(std::chrono::milliseconds(m_ms_refresh_rate));
            }
        }
    
        void close_game()
        {
            m_end_refresh = true;
    
            if (m_refresh_field_thread.joinable())
            {
                m_refresh_field_thread.join();
            }
        }
    };
    
    std::unique_ptr<GameField> g_game_field = nullptr;
}


namespace snake_dk_api
{
    void start_game(const int t_field_width, const int t_field_height, const int t_ms_refresh_rate)
    {
        if (snake_dk_details::g_game_field != nullptr) { return; }
        snake_dk_details::g_game_field = std::make_unique<snake_dk_details::GameField>(t_field_width, t_field_height, t_ms_refresh_rate);
    }
    
    // API
    // 0 -> for empty cell
    // 1 -> for snake head
    // 2 -> for snake body
    // 3 -> food
    void get_game_field_vector(std::vector<int>& t_ret_map, int& t_ret_map_w, int& t_ret_map_h)
    {
        if (snake_dk_details::g_game_field == nullptr) { return; }
    
        std::vector<int> aux_tmp;
        int aux_w = 0;
        int aux_h = 0;
        snake_dk_details::g_game_field->get_game_field_as_vector(aux_tmp, aux_w, aux_h);
    
        t_ret_map   = aux_tmp;
        t_ret_map_w = aux_w;
        t_ret_map_h = aux_h;
    }
    
    void change_snake_direction(const snake_dk_api::eSNAKE_DIRECTION t_new_direction)
    {
        if (snake_dk_details::g_game_field == nullptr) { return; }
        snake_dk_details::g_game_field->change_snake_direction(t_new_direction);
    }
    
    void end_game()
    {
        if (snake_dk_details::g_game_field == nullptr) { return; }
        snake_dk_details::g_game_field->close_game();
        snake_dk_details::g_game_field.reset();
        snake_dk_details::g_game_field = nullptr;
    }
}

#endif // SNAKE_DK_H
