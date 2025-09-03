// 2025-05-08  tetris_ai_v3_c_version.c


//////////////// 包含


#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Windows.h>


//////////////// 宏


#define TETRIS_MAX_ANGLE    4
#define TETRIS_SHAPE_I_LIM  4
#define TETRIS_SHAPE_J_LIM  4
#define TETRIS_GRID_I_LIM   20
#define TETRIS_GRID_J_LIM   10

#define HOLE_WEIGHT (-4)
#define WELL_WEIGHT (-1)
#define ROW_TRANSITION_WEIGHT (-1)
#define COL_TRANSITION_WEIGHT (-1)
#define LANDING_HEIGHT_WEIGHT (-1)
#define ERODED_CELLS_WEIGHT 1

//#define DEBUGGING_THE_EVALUATOR
//#define DRAW_DETAIL


//////////////// 类声明


typedef struct {
    int  content[TETRIS_SHAPE_I_LIM][TETRIS_SHAPE_J_LIM];
    int  i_lim;
    int  j_lim;
} shape_s;

int shape_get_i_lim(const shape_s *shape);
int shape_get_j_lim(const shape_s *shape);
int shape_get_cell_not_hitbox_check(const shape_s *shape, int i, int j);
int shape_get_cell_hitbox_check(const shape_s *shape, int i, int j);


typedef struct {
    int  placed_blocks;
    int  score;
    int  total_lines_cleared;
    int  lines_cleared[5];
} statistics_s;

statistics_s statistics_make_blank();  // 构造函数
void statistics_print_out(const statistics_s *statistics);


typedef struct {
    int  indices[4];
    int  size;
} full_rows_index_container_s;

full_rows_index_container_s full_rows_index_container_make_blank();  // 构造函数
full_rows_index_container_s full_rows_index_container_with_a_row_index_appended(const full_rows_index_container_s *container, int index);
bool full_rows_index_container_contains(const full_rows_index_container_s *container, int index);


typedef struct {
    int content[TETRIS_GRID_I_LIM][TETRIS_GRID_J_LIM];
} grid_s;

grid_s grid_make_blank();  // 构造函数
grid_s grid_with_a_tetris_placed(const grid_s *grid, char tetris, int rotation, int j_pos, int i_pos);
full_rows_index_container_s grid_all_full_rows(const grid_s *grid);
void grid_print_out(const grid_s *grid);
bool grid_is_deadline_touched(const grid_s *grid);
int grid_get_with_default(const grid_s *grid, int i, int j, int default_value);


typedef struct {
    int  rotation;
    int  j_pos;
} operation_s;

void operation_print_out(const operation_s *operation);


typedef struct {
    grid_s        grid;
    char          falling_tetris;
    char          next_tetris;
    bool          deadline_touched;
    statistics_s  statistics;
} game_state_s;


game_state_s game_state_make(grid_s grid, char falling_tetris, char next_tetris, bool deadline_touched, statistics_s statistics);  // 构造函数
game_state_s game_state_with_next_tetris_filled_in(const game_state_s *game_state, char next_tetris);
void game_state_print_grid(const game_state_s *game_state);
void game_state_print_statistics(const game_state_s *game_state);
bool game_state_is_deadline_touched(const game_state_s *game_state);
operation_s game_state_make_decision(const game_state_s *game_state);
game_state_s game_state_the_next_state_with_no_next_tetris(const game_state_s *game_state, operation_s operation);
int game_state__calculate_i_pos(const game_state_s *game_state, operation_s operation);
operation_s game_state__calculate_best_move(const game_state_s *game_state);
double game_state__calculate_evaluate_score(const game_state_s *game_state, int rotation, int j_pos, int i_pos);
void game_state_draw_the_falling_tetris(const game_state_s *game_state);
void game_state_static_test_evaluator(void);


//////////////// 不变的数据


const shape_s tetris_shapes[128][TETRIS_MAX_ANGLE] = {
    ['I'] = {
        {
            {
                {1, 1, 1, 1},
                {0, 0, 0, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0},
            },
            1,
            4,
        },
        {
            {
                {1, 0, 0, 0},
                {1, 0, 0, 0},
                {1, 0, 0, 0},
                {1, 0, 0, 0},
            },
            4,
            1,
        },
        {
            {
                {1, 1, 1, 1},
                {0, 0, 0, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0},
            },
            1,
            4,
        },
        {
            {
                {1, 0, 0, 0},
                {1, 0, 0, 0},
                {1, 0, 0, 0},
                {1, 0, 0, 0},
            },
            4,
            1,
        },
    },
    ['O'] = {
        {
            {
                {1, 1, 0, 0},
                {1, 1, 0, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0},
            },
            2,
            2,
        },
        {
            {
                {1, 1, 0, 0},
                {1, 1, 0, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0},
            },
            2,
            2,
        },
        {
            {
                {1, 1, 0, 0},
                {1, 1, 0, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0},
            },
            2,
            2,
        },
        {
            {
                {1, 1, 0, 0},
                {1, 1, 0, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0},
            },
            2,
            2,
        },
    },
    ['T'] = {
        {
            {
                {0, 1, 0, 0},
                {1, 1, 1, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0},
            },
            2,
            3,
        },
        {
            {
                {1, 0, 0, 0},
                {1, 1, 0, 0},
                {1, 0, 0, 0},
                {0, 0, 0, 0},
            },
            3,
            2,
        },
        {
            {
                {1, 1, 1, 0},
                {0, 1, 0, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0},
            },
            2,
            3,
        },
        {
            {
                {0, 1, 0, 0},
                {1, 1, 0, 0},
                {0, 1, 0, 0},
                {0, 0, 0, 0},
            },
            3,
            2,
        },
    },
    ['S'] = {
        {
            {
                {0, 1, 1, 0},
                {1, 1, 0, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0},
            },
            2,
            3,
        },
        {
            {
                {1, 0, 0, 0},
                {1, 1, 0, 0},
                {0, 1, 0, 0},
                {0, 0, 0, 0},
            },
            3,
            2,
        },
        {
            {
                {0, 1, 1, 0},
                {1, 1, 0, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0},
            },
            2,
            3,
        },
        {
            {
                {1, 0, 0, 0},
                {1, 1, 0, 0},
                {0, 1, 0, 0},
                {0, 0, 0, 0},
            },
            3,
            2,
        },
    },
    ['Z'] = {
        {
            {
                {1, 1, 0, 0},
                {0, 1, 1, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0},
            },
            2,
            3,
        },
        {
            {
                {0, 1, 0, 0},
                {1, 1, 0, 0},
                {1, 0, 0, 0},
                {0, 0, 0, 0},
            },
            3,
            2,
        },
        {
            {
                {1, 1, 0, 0},
                {0, 1, 1, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0},
            },
            2,
            3,
        },
        {
            {
                {0, 1, 0, 0},
                {1, 1, 0, 0},
                {1, 0, 0, 0},
                {0, 0, 0, 0},
            },
            3,
            2,
        },
    },
    ['L'] = {
        {
            {
                {0, 0, 1, 0},
                {1, 1, 1, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0},
            },
            2,
            3,
        },
        {
            {
                {1, 0, 0, 0},
                {1, 0, 0, 0},
                {1, 1, 0, 0},
                {0, 0, 0, 0},
            },
            3,
            2,
        },
        {
            {
                {1, 1, 1, 0},
                {1, 0, 0, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0},
            },
            2,
            3,
        },
        {
            {
                {1, 1, 0, 0},
                {0, 1, 0, 0},
                {0, 1, 0, 0},
                {0, 0, 0, 0},
            },
            3,
            2,
        },
    },
    ['J'] = {
        {
            {
                {1, 0, 0, 0},
                {1, 1, 1, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0},
            },
            2,
            3,
        },
        {
            {
                {1, 1, 0, 0},
                {1, 0, 0, 0},
                {1, 0, 0, 0},
                {0, 0, 0, 0},
            },
            3,
            2,
        },
        {
            {
                {1, 1, 1, 0},
                {0, 0, 1, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0},
            },
            2,
            3,
        },
        {
            {
                {0, 1, 0, 0},
                {0, 1, 0, 0},
                {1, 1, 0, 0},
                {0, 0, 0, 0},
            },
            3,
            2,
        },
    }
};

const int scores_of_line_cleared[5] = {0, 100, 300, 500, 800};


//////////////// 自由函数声明

// 没有 main 函数的声明。

int new_main(void);
int raw_main(void);
void run_ai_1(void);
operation_s run_game_step(game_state_s *game, char next_tetris);


//////////////// 自由函数定义


operation_s run_game_step(game_state_s *game, char next_tetris)
{
    game_state_s obj = *game;

    const operation_s operation = game_state_make_decision(&obj);
    obj = game_state_the_next_state_with_no_next_tetris(&obj, operation);


#ifdef DRAW_DETAIL
    operation_print_out(&operation);
    printf("\n");
    game_state_print_grid(&obj);
    printf("//=================\\\\\n");
    game_state_print_statistics(&obj);
    printf("\\\\=================//\n");
    game_state_draw_the_falling_tetris(&obj);
    fflush(stdout);
#endif /* DRAW_DETAIL */

    // 打印操作与分数
    //printf("%d %d\n", operation.rotation, operation.j_pos);
    //fflush(stdout);
    //printf("%d\n", obj.statistics.score);
    //fflush(stdout);

    // 这个函数要检查 X 和 E 吗？是的话，确实应该在这里检查吗？
    //if (!(next_tetris == 'X' || next_tetris == 'E')) {
    //    obj = game_state_with_next_tetris_filled_in(&obj, next_tetris);
    //}

    obj = game_state_with_next_tetris_filled_in(&obj, next_tetris);

    *game = obj;  // 修改出参……
    return operation;
}

// TODO labs 是在干什么？

void print_operation(const game_state_s *game, const operation_s *operation)
{
    printf("%d %d\n%d\n", operation->rotation, operation->j_pos, game->statistics.score);
    fflush(stdout);
}


int main(void)
{
    return raw_main();
}


int new_main(void)
{
    // 主程序，包含了有 IO 的主循环

    char block1, block2;

    scanf("%c%c\n", &block1, &block2);
    game_state_s game = game_state_make(grid_make_blank(), block1, block2, false, statistics_make_blank());

    // 这次调用重复给了 block2，没关系。game_state_make 里给的 block2 才是被忽略的。
    operation_s operation = run_game_step(&game, block2);
    print_operation(&game, &operation);

    while (true) {
        scanf("%c\n", &block1);

        if (block1 == 'E') {
            break;
        }

        // 实际上碰到文件尾是 scanf 本身返回 EOF
        if (block1 == EOF) {
            break;
        }

        operation = run_game_step(&game, block1);
        print_operation(&game, &operation);

        // 不用自己判断触线了

        if (block1 == 'X') {
            break;
        }
    }


    return 0;
}


int raw_main(void)
{
#ifdef DEBUGGING_THE_EVALUATOR
    game_state_static_test_evaluator();
#else
    run_ai_1();
#endif
    return 0;
}


void run_ai_1(void)
{
    char first_line[10] = {0};
    fgets(first_line, 10, stdin);
    assert(first_line[2] == '\n');

    char first = first_line[0];
    char second = first_line[1];

    game_state_s game = game_state_make(grid_make_blank(), first, second, false, statistics_make_blank());


    while (true) {
        //Sleep(400);
        operation_s operation = game_state_make_decision(&game);
        game = game_state_the_next_state_with_no_next_tetris(&game, operation);


#ifdef DRAW_DETAIL
        // draw things
        operation_print_out(&operation);
        printf("\n");
        game_state_print_grid(&game);
        printf("//=================\\\\\n");
        game_state_print_statistics(&game);
        printf("\\\\=================//\n");
        game_state_draw_the_falling_tetris(&game);
        fflush(stdout);
#endif /* DRAW_DETAIL */

        // 应该在这里打印操作与分数
        printf("%d %d\n", operation.rotation, operation.j_pos);
        fflush(stdout);
        printf("%d\n", game.statistics.score);
        fflush(stdout);

        first = second;

        if (first == 'X') {
            break;
        }

        char next_line[10] = {0};
        fgets(next_line, 10, stdin);
        assert(next_line[1] == '\n');
        second = next_line[0];

        if (second == 'E') {
            break;
        }

        //if (game_state_is_deadline_touched(&game)) {
        //    printf("Game Over because deadline touched.\n");
        //    return;
        //}

        //if (second == 'X') {
        //    printf("Game Over because next_tetris is X.\n");
        //    return;
        //}


        game = game_state_with_next_tetris_filled_in(&game, second);
    }
}



//////////////// 类成员函数实现


int shape_get_i_lim(const shape_s *shape)
{
    return shape->i_lim;
}


int shape_get_j_lim(const shape_s *shape)
{
    return shape->j_lim;
}


int shape_get_cell_hitbox_check(const shape_s *shape, int i, int j)
{
    assert(0 <= i && i < shape->i_lim);
    assert(0 <= j && j < shape->j_lim);
    return shape->content[i][j];
}


int shape_get_cell_not_hitbox_check(const shape_s *shape, int i, int j)
{
    assert(0 <= i && i < TETRIS_SHAPE_I_LIM);
    assert(0 <= j && j < TETRIS_SHAPE_J_LIM);
    return shape->content[i][j];
}


statistics_s statistics_make_blank()
{
    return (statistics_s) {
        .placed_blocks       = 0,
        .score               = 0,
        .total_lines_cleared = 0,
        .lines_cleared       = {0, 0, 0, 0, 0},
    };
}


void statistics_print_out(const statistics_s *statistics)
{
    printf("statistics:\n");
    printf("- score: %d\n", statistics->score);
    printf("- placed_blocks: %d\n", statistics->placed_blocks);
    printf("- cleared_lines:\n");

    for (int i = 1; i < 5; ++i) {
        printf("  - %d lines: %d\n", i, statistics->lines_cleared[i]);
    }
}


grid_s grid_make_blank()
{
    grid_s grid;

    for (int i = 0; i < TETRIS_GRID_I_LIM; ++i) {

        for (int j = 0; j < TETRIS_GRID_J_LIM; ++j) {
            grid.content[i][j] = 0;
        }
    }

    return grid;
}


grid_s grid_with_a_tetris_placed(const grid_s *grid, char tetris, int rotation, int j_pos, int i_pos)
{
    grid_s new_grid = *grid;
    const shape_s shape = tetris_shapes[(unsigned char) tetris][rotation];

    for (int i = 0; i < shape_get_i_lim(&shape); ++i) {

        for (int j = 0; j < shape_get_j_lim(&shape); ++j) {
            const int abs_i = i_pos + i;
            const int abs_j = j_pos + j;

            assert(0 <= abs_i && abs_i < TETRIS_GRID_I_LIM);
            assert(0 <= abs_j && abs_j < TETRIS_GRID_J_LIM);

            if (shape_get_cell_hitbox_check(&shape, i, j) == 0) {
                continue;
            }

            assert(new_grid.content[abs_i][abs_j] == 0);
            new_grid.content[abs_i][abs_j] = 1;
        }
    }

    return new_grid;
}


full_rows_index_container_s grid_all_full_rows(const grid_s *grid)
{
    full_rows_index_container_s container = full_rows_index_container_make_blank();

    for (int i = 0; i < TETRIS_GRID_I_LIM; ++i) {
        bool all_value = true;

        for (int j = 0; j < TETRIS_GRID_J_LIM; ++j) {

            if (grid->content[i][j] == 0) {
                all_value = false;
                break;
            }
        }

        if (all_value) {
            container = full_rows_index_container_with_a_row_index_appended(&container, i);
        }
    }

    return container;
}


void grid_print_out(const grid_s *grid)
{
    printf("     0 1 2 3 4 5 6 7 8 9\n");
    printf("   +--------------------+\n");

    for (int i = 0; i < TETRIS_GRID_I_LIM; ++i) {
        printf("%2d |", i);

        for (int j = 0; j < TETRIS_GRID_J_LIM; ++j) {
            printf("%s", grid->content[i][j] == 1 ? "[]" : "  ");
        }
        printf("|\n");
    }
    printf("   +--------------------+\n");
}


bool grid_is_deadline_touched(const grid_s *grid)
{
    for (int j = 0; j < TETRIS_GRID_J_LIM; ++j) {

        // 4 是死线
        if (grid->content[4][j] == 1) {
            return true;
        }
    }

    return false;
}


int grid_get_with_default(const grid_s *grid, int i, int j, int default_value)
{
    if (0 <= i && i < TETRIS_GRID_I_LIM && 0 <= j && j < TETRIS_GRID_J_LIM) {
        return grid->content[i][j];
    }

    return default_value;
}


void operation_print_out(const operation_s *operation)
{
    printf("operation: rotation=%d, j_pos=%d\n", operation->rotation, operation->j_pos);
}


game_state_s game_state_make(grid_s grid, char falling_tetris, char next_tetris, bool deadline_touched, statistics_s statistics)
{
    return (game_state_s) {
        .grid             = grid,
        .falling_tetris   = falling_tetris,
        .next_tetris      = next_tetris,
        .deadline_touched = deadline_touched,
        .statistics       = statistics
    };
}


game_state_s game_state_with_next_tetris_filled_in(const game_state_s *game_state, char next_tetris)
{
    assert(game_state->next_tetris == '?');
    game_state_s return_value = *game_state;
    return_value.next_tetris = next_tetris;
    return return_value;
}


void game_state_print_grid(const game_state_s *game_state)
{
    grid_print_out(&game_state->grid);
}


void game_state_print_statistics(const game_state_s *game_state)
{
    statistics_print_out(&game_state->statistics);
}


bool game_state_is_deadline_touched(const game_state_s *game_state)
{
    return game_state->deadline_touched;
}


operation_s game_state_make_decision(const game_state_s *game_state)
{
    return game_state__calculate_best_move(game_state);
}


game_state_s game_state_the_next_state_with_no_next_tetris(const game_state_s *game_state, operation_s operation)
{
    const int rotation = operation.rotation;
    const int j_pos = operation.j_pos;

    // 0. 创建一个副本，并开始修改它
    game_state_s return_value = *game_state;
    const int i_pos = game_state__calculate_i_pos(&return_value, operation);

    // 1. 更新网格（将下落的方块放入格子中）
    return_value.grid = grid_with_a_tetris_placed(&return_value.grid, game_state->falling_tetris, rotation, j_pos, i_pos);

    // 2. (有可能) deadline_touched = True
    if (grid_is_deadline_touched(&game_state->grid)) {
        return_value.deadline_touched = true;
    }

    // debug
    //printf("######################\n");
    //grid_print_out(&return_value.grid);

    // 3. 更新网格（清除已满的行）
    // 如果不想再去更改已满行的索引，那么清除满行时就要从上到下。
    const full_rows_index_container_s container = grid_all_full_rows(&return_value.grid);

    for (int i = 0; i < container.size; ++i) {
        memmove(
            &return_value.grid.content[1][0],
            &return_value.grid.content[0][0],
            container.indices[i] * sizeof return_value.grid.content[0]
        );
        memset(&return_value.grid.content[0][0], 0, sizeof return_value.grid.content[0]);
    }

    // debug
    //printf("!!!!!!!!!!!!!!!!!!!!!!!\n");
    //grid_print_out(&return_value.grid);

    // 4. 更新数据
    return_value.statistics.placed_blocks++;

    if (container.size > 0) {
        return_value.statistics.score += scores_of_line_cleared[container.size];
        return_value.statistics.lines_cleared[container.size]++;
        return_value.statistics.total_lines_cleared += container.size;
    }

    // 5. falling_tetris = next_tetris
    return_value.falling_tetris = game_state->next_tetris;

    // 6. next_tetris = ""
    // 假定下次调用此方法之前会调用 fill_in_the_next_tetris() 方法
    return_value.next_tetris = '?';

    // 7. 返回已修改的副本
    return return_value;
}


int game_state__calculate_i_pos(const game_state_s *game_state, operation_s operation)
{
    const int rotation = operation.rotation;
    const int j_pos = operation.j_pos;

    const shape_s shape = tetris_shapes[(unsigned char) game_state->falling_tetris][rotation];

    for (int i_pos = TETRIS_GRID_I_LIM - 1; i_pos >= 0; --i_pos) {
        // 尝试一个坐标 ((i_pos, j_pos) 组合)
        bool can_place = true;

        for (int rel_i = 0; rel_i < shape_get_i_lim(&shape); ++rel_i) {

            for (int rel_j = 0; rel_j < shape_get_j_lim(&shape); ++rel_j) {
                const int cell = shape_get_cell_hitbox_check(&shape, rel_i, rel_j);

                // 在这里修改了算法
                if (cell == 0) {
                    continue;
                }

                const int abs_i = i_pos + rel_i;
                const int abs_j = j_pos + rel_j;

                // 出界
                if (!(0 <= abs_i && abs_i < TETRIS_GRID_I_LIM && 0 <= abs_j && abs_j < TETRIS_GRID_J_LIM)) {
                    can_place = false;
                    goto end;
                }

                // 位置被占
                if (game_state->grid.content[abs_i][abs_j] == 1) {
                    can_place = false;
                    goto end;
                }

                // 上方被挡
                bool any_value = false;

                for (int abs_i_scan = 0; abs_i_scan < abs_i; ++abs_i_scan) {

                    if (game_state->grid.content[abs_i_scan][abs_j] == 1) {
                        any_value = true;
                        break;
                    }
                }

                if (any_value) {
                    can_place = false;
                    goto end;
                }
            }
        }
    end:
        // 现在就知道这个位置是否有效了
        if (can_place) {
            // 返回最矮的 i_pos
            return i_pos;
        }
    }

    // 所有位置都无效
    return -1;
}


operation_s game_state__calculate_best_move(const game_state_s *game_state)
{
    operation_s best_moves[40];
    int best_moves_size = 0;
    double best_evaluate_score = -INFINITY;

    for (int rotation = 0; rotation < 4; ++rotation) {

        for (int j_pos = 0; j_pos < 10; ++j_pos) {
            const operation_s operation = {.rotation = rotation, .j_pos = j_pos};

            const int i_pos = game_state__calculate_i_pos(game_state, operation);

            if (i_pos == -1) {
                continue;
            }

            const double evaluate_score = game_state__calculate_evaluate_score(game_state, rotation, j_pos, i_pos);

            if (evaluate_score > best_evaluate_score) {
                best_moves_size = 0;
                best_moves[best_moves_size++] = operation;
                best_evaluate_score = evaluate_score;

            } else if (evaluate_score == best_evaluate_score) {
                best_moves[best_moves_size++] = operation;
            }
        }
    }

    assert(best_moves_size > 0);

    // 如果不同摆法存在相同的最高评价值，就再按优先级（priority）分出高低。
    // 第一档：优先靠墙。目标位置的横坐标偏离入场位置的程度越大，就越优先，每格记 100 分。
    // 第二档：优先向左。如果第一档同分，就取向左移的摆法，“居左”这一状态记 10 分。
    // 第三档：优先少转。如果前两档同分，就取旋转次数最少的摆法，每次旋转多扣 1 分。
    // 第三档意义不明，所以直接忽略。
    double best_score_for_priority = -INFINITY;
    operation_s best_operation_by_priority;

    for (int i = 0; i < best_moves_size; ++i) {
        const operation_s operation = best_moves[i];
        //const int rotation = operation.rotation;
        const int j_pos = operation.j_pos;
        //const shape_s shape = tetris_shapes[(unsigned char) game_state->falling_tetris][rotation];
        // 这里更改了公式
        const int priority = 100 * fabs((j_pos) - 4.5) + 10 * (9 - j_pos);

        if (priority > best_score_for_priority) {
            best_score_for_priority = priority;
            best_operation_by_priority = operation;
        }
    }

    assert(best_score_for_priority > -INFINITY);
    return best_operation_by_priority;
}


double game_state__calculate_evaluate_score(const game_state_s *game_state, int rotation, int j_pos, int i_pos)
{
    // 评估得分。

    // 公式：评价 = −4∗洞数 − 累计井数 − 行转变数 − 列转变数 − 方块着陆高度 + 侵蚀格数
    // 洞（Hole）：洞是正上方存在砖格的空格
    // 井（Well）：左右两侧都是砖格或墙壁的空格
    // 行转变数（Row Transition）：一行中砖格和空格交替出现，交替了几次。墙和砖格等效。
    // 列转变数（Column Transition）：一列中砖格和空格交替出现，交替了几次。墙和砖格等效。
    // 方块着陆高度（Landing Height）：列高 + 块高/2。但如果触发消行，则是取消行后的结果。
    // 侵蚀格数（Number of Eroded Cells）：当前方块的消行数乘以填入被消行的方格数

    // 参考资料：
    // 方块 AI 算法历史 (1996–2013) https://tetris.huijiwiki.com/wiki/%E6%96%B9%E5%9D%97_AI_%E7%AE%97%E6%B3%95%E5%8E%86%E5%8F%B2_(1996%E2%80%932013)
    // Tetris AI (单块, Pierre Dellacherie, 2003) https://tetris.huijiwiki.com/wiki/Tetris_AI_(%E5%8D%95%E5%9D%97,_Pierre_Dellacherie,_2003)

    const shape_s shape = tetris_shapes[(unsigned char) game_state->falling_tetris][rotation];

    const grid_s new_grid = grid_with_a_tetris_placed(&game_state->grid, game_state->falling_tetris, rotation, j_pos, i_pos);
    const full_rows_index_container_s container = grid_all_full_rows(&new_grid);
    grid_s new_grid_with_full_rows_cleared = new_grid;

    // 清除已满的行。
    // 如果不想再去更改已满行的索引，清除满行时应从上到下。

    for (int i = 0; i < container.size; ++i) {
        memmove(
            &new_grid_with_full_rows_cleared.content[1][0],
            &new_grid_with_full_rows_cleared.content[0][0],
            container.indices[i] * sizeof new_grid_with_full_rows_cleared.content[0]
        );
        memset(&new_grid_with_full_rows_cleared.content[0][0], 0, sizeof new_grid_with_full_rows_cleared.content[0]);
    }

    // 洞
    int hole = 0;
    for (int i = 0; i < TETRIS_GRID_I_LIM; ++i) {

        for (int j = 0; j < TETRIS_GRID_J_LIM; ++j) {

            if (new_grid_with_full_rows_cleared.content[i][j] == 1) {
                continue;
            }
            bool any_value = false;

            for (int i_scan = 0; i_scan < i; ++i_scan) {

                if (new_grid_with_full_rows_cleared.content[i_scan][j] == 1) {
                    any_value = true;
                    break;
                }
            }

            if (any_value) {
                ++hole;
            }
        }
    }
#ifdef DEBUGGING_THE_EVALUATOR
    printf("hole: %d\n", hole);
#endif

    // 井
    int well = 0;
    for (int i = 0; i < TETRIS_GRID_I_LIM; ++i) {

        for (int j = 0; j < TETRIS_GRID_J_LIM; ++j) {

            if (new_grid_with_full_rows_cleared.content[i][j] == 1) {
                continue;
            }

            if (grid_get_with_default(&new_grid_with_full_rows_cleared, i, j-1, 1) == 1
                && grid_get_with_default(&new_grid_with_full_rows_cleared, i, j+1, 1) == 1)
            {
                ++well;
            }
        }
    }
#ifdef DEBUGGING_THE_EVALUATOR
    printf("well: %d\n", well);
#endif

    // 行转变数
    int row_transition = 0;

    for (int i = 0; i < TETRIS_GRID_I_LIM; ++i) {

        for (int j = -1; j < TETRIS_GRID_J_LIM; ++j) {

            if (grid_get_with_default(&new_grid_with_full_rows_cleared, i, j, 1) != grid_get_with_default(&new_grid_with_full_rows_cleared, i, j+1, 1)) {
                ++row_transition;
            }
        }
    }
#ifdef DEBUGGING_THE_EVALUATOR
    printf("row_transition: %d\n", row_transition);
#endif

    // 列转变数
    int col_transition = 0;

    for (int i = -1; i < TETRIS_GRID_I_LIM; ++i) {

        for (int j = 0; j < TETRIS_GRID_J_LIM; ++j) {

            if (grid_get_with_default(&new_grid_with_full_rows_cleared, i, j, 1) != grid_get_with_default(&new_grid_with_full_rows_cleared, i+1, j, 1)) {
                ++col_transition;
            }
        }
    }
#ifdef DEBUGGING_THE_EVALUATOR
    printf("col_transition: %d\n", col_transition);
#endif


    // 着陆高度
    double landing_height = 20 - (i_pos + shape_get_i_lim(&shape) / 2.0);

#ifdef DEBUGGING_THE_EVALUATOR
    printf("landing_height: %lf\n", landing_height);
#endif

    // 侵蚀格数
    int eroded_cells = 0;

    for (int rel_i = 0; rel_i < shape_get_i_lim(&shape); ++rel_i) {

        for (int rel_j = 0; rel_j < shape_get_j_lim(&shape); ++rel_j) {
            const int abs_i = i_pos + rel_i;

            if (full_rows_index_container_contains(&container, abs_i)) {
                ++eroded_cells;
            }
        }
    }
    eroded_cells *= container.size;
#ifdef DEBUGGING_THE_EVALUATOR
    printf("eroded_cells: %d\n", eroded_cells);
#endif

    const double res =
        HOLE_WEIGHT * hole
        + WELL_WEIGHT * well
        + ROW_TRANSITION_WEIGHT * row_transition
        + COL_TRANSITION_WEIGHT * col_transition
        + LANDING_HEIGHT_WEIGHT * landing_height
        + ERODED_CELLS_WEIGHT * eroded_cells;

#ifdef DEBUGGING_THE_EVALUATOR
    printf("result of the evaluator: %lf\n", res);
#endif

    return res;
}


void game_state_draw_the_falling_tetris(const game_state_s *game_state)
{
    const shape_s shape = tetris_shapes[(unsigned char) game_state->falling_tetris][0];
    printf("       falling tetris\n");
    printf("       +------------+\n");
    printf("       |            |\n");

    for (int i = 0; i < TETRIS_SHAPE_I_LIM; ++i) {

        if (i < shape_get_i_lim(&shape)) {
            // 打印一行
            printf("       |  ");

            for (int j = 0; j < TETRIS_SHAPE_J_LIM; ++j) {
                printf("%s", shape_get_cell_not_hitbox_check(&shape, i, j) == 1 ? "[]" : "  ");
            }
            printf("  |\n");

        } else {
            // 打印空行
            printf("       |            |\n");
        }
    }

    printf("       |            |\n");
    printf("       +------------+\n");
}


void game_state_static_test_evaluator(void)
{
    /*
    const game_state_s game = {
        .grid = {
            .content = {
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {1, 0, 0, 0, 0, 0, 0, 0, 1, 0},
                {1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
                {1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
            },
        },
        .falling_tetris   = 'I',
        .next_tetris      = 'Z',
        .deadline_touched = false,
        .statistics       = {
            .placed_blocks       = 5,
            .score               = 0,
            .total_lines_cleared = 0,
            .lines_cleared       = {0, 0, 0, 0, 0},
        },
    };
    */

    // 造环境然后测试

    const game_state_s game = {
        .grid = {
            .content = {
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                {1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
                {1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
                {1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
                {1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
            },
        },
        .falling_tetris   = 'I',
        .next_tetris      = 'J',
        .deadline_touched = false,
        .statistics       = {
            .placed_blocks       = 233,
            .score               = 0,
            .total_lines_cleared = 0,
            .lines_cleared       = {0, 0, 0, 0, 0},
        },
    };

    const int j_pos = 0;
    const int i_pos = 16;
    const int rotation = 1;

    game_state__calculate_evaluate_score(&game, rotation, j_pos, i_pos);

    grid_s grid = game.grid;
    grid = grid_with_a_tetris_placed(&grid, game.falling_tetris, rotation, j_pos, i_pos);
    grid_print_out(&grid);

    return;
}


full_rows_index_container_s full_rows_index_container_make_blank()
{
    return (full_rows_index_container_s) { .indices = {0, 0, 0, 0}, .size = 0 };
}


full_rows_index_container_s full_rows_index_container_with_a_row_index_appended(const full_rows_index_container_s *container, int index)
{
    assert(0 <= index && index < TETRIS_GRID_I_LIM);
    assert(0 <= container->size && container->size <= 3);

    full_rows_index_container_s new_container = *container;

    new_container.indices[new_container.size++] = index;

    return new_container;
}


bool full_rows_index_container_contains(const full_rows_index_container_s *container, int index)
{
    for (int i = 0; i < container->size; ++i) {

        if (container->indices[i] == index) {
            return true;
        }
    }

    return false;
}
