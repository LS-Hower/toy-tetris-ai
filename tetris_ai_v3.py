# 2025-04-30  tetris_ai_v3.py

import itertools
import time
from tetris_shape import TETRIS_SHAPES, shape_width, shape_height

from dataclasses import dataclass

import copy


SCORES_OF_LINE_CLEARED = {
    1: 100,
    2: 300,
    3: 500,
    4: 800,
}



type list2d[T] = list[list[T]]

@dataclass
class Statistics(object):
    """
    携带了游戏右侧面板的所有统计信息，包括：
    - 当前得分
    - 已经放置了几个方块
    - 当前已消除的行数（1 行，2 行，3 行，4 行 分别消了几次）
    """
    placed_blocks:       int
    score:               int
    total_cleared_lines: int
    cleared_lines:       dict[int, int]

    def print_out(self) -> None:
        print(f"statistics:")
        print(f"- score: {self.score}")
        print(f"- placed_blocks: {self.placed_blocks}")
        print(f"- cleared_lines:")
        for line, count in self.cleared_lines.items():
            print(f"  - {line} lines: {count}")



class GameState(object):
    """
    对外显现为不可变对象。
    游戏当前的一个状态，应该包含一个截图中的信息，即：

    - 当前网格已有方块内容
    - 需要下降的方块
    - 下一个方块
    - 当前游戏是否结束
    - 统计信息
    """

    def __init__(self, grid: list2d[int], falling_tetris: str, next_tetris: str, deadline_touched: bool, statistics: Statistics) -> None:
        self.grid             = grid
        self.falling_tetris   = falling_tetris
        self.next_tetris      = next_tetris
        self.deadline_touched = deadline_touched
        self.statistics       = statistics
        return None


    def with_next_tetris_filled_in(self, next_tetris: str) -> 'GameState':
        assert self.next_tetris == ""
        new_state = copy.deepcopy(self)
        new_state.next_tetris = next_tetris
        return new_state


    def print_grid(self) -> None:
        print("     0 1 2 3 4 5 6 7 8 9")
        print("   +--------------------+")
        for i, row in enumerate(self.grid):
            print(f"{i:2d} |{''.join("[]" if cell == 1 else "  " for cell in row)}|")
        print("   +--------------------+")
        return None


    def print_statistics(self) -> None:
        return self.statistics.print_out()


    def is_deadline_touched(self) -> bool:
        return self.deadline_touched


    def make_decision(self) -> tuple[int, int]:
        return self._calculate_best_move()


    @staticmethod
    def _place_tetris_into_a_grid(grid: list2d[int], tetris: str, rotation: int, j_pos: int, i_pos: int) -> list2d[int]:
        new_grid = copy.deepcopy(grid)
        for abs_i, row in enumerate(TETRIS_SHAPES[tetris][rotation], i_pos):
            for abs_j, cell in enumerate(row, j_pos):
                if cell == 1:
                    assert new_grid[abs_i][abs_j] == 0, "cannot place the tetris here"
                    new_grid[abs_i][abs_j] = 1
        return new_grid


    def the_next_state(self, rotation: int, j_pos: int) -> 'GameState':
        # 0. 创建一个副本，并开始修改它
        new_state = copy.deepcopy(self)
        i_pos = new_state._calculate_i_pos(rotation, j_pos)

        # 1. 更新网格（将下落的方块放入格子中）
        new_state.grid = GameState._place_tetris_into_a_grid(new_state.grid, new_state.falling_tetris, rotation, j_pos, i_pos)

        # 2. (有可能) deadline_touched = True
        if any(new_state.grid[4]):
            new_state.deadline_touched = True

        # 3. 更新网格（清除已满的行）
        # 如果不想再去更改已满行的索引，那么清除满行时就要从上到下。
        full_rows = sorted(GameState._all_full_rows(new_state.grid))
        for i in full_rows:
            new_state.grid.pop(i)
            new_state.grid.insert(0, [0 for _ in range(10)])

        # 4. 更新数据
        new_state.statistics.placed_blocks += 1
        if len(full_rows) > 0:
            new_state.statistics.score += SCORES_OF_LINE_CLEARED[len(full_rows)]
            new_state.statistics.cleared_lines[len(full_rows)] += 1
            new_state.statistics.total_cleared_lines += len(full_rows)

        # 5. falling_tetris = next_tetris
        new_state.falling_tetris = new_state.next_tetris

        # 6. next_tetris = ""
        # 假定下次调用此方法之前会调用 fill_in_the_next_tetris() 方法
        new_state.next_tetris = ""

        # 7. 返回已修改的副本
        return new_state

    @staticmethod
    def _all_full_rows(grid: list2d[int]) -> list[int]:
        return [i for i, row in enumerate(grid) if all(row)]


    def _calculate_i_pos(self, rotation: int, j_pos: int) -> int:
        for i_pos in range(19, -1, -1):
            can_place = True
            for abs_i, row in enumerate(TETRIS_SHAPES[self.falling_tetris][rotation], i_pos):
                for abs_j, cell in enumerate(row, j_pos):

                    # 出界
                    if abs_i not in range(20) or abs_j not in range(10):
                        can_place = False

                    # 位置被占
                    elif cell == self.grid[abs_i][abs_j] == 1:
                        can_place = False

                    # 上方被挡
                    elif any(self.grid[i_][abs_j] for i_ in range(abs_i)):
                        can_place = False
            if can_place:
                return i_pos
        return -1


    def _calculate_best_move(self) -> tuple[int, int]:

        best_moves: set[tuple[int, int]] = set()
        best_score = -float('inf')

        for rotation, j_pos in itertools.product(range(4), range(10)):
            i_pos = self._calculate_i_pos(rotation, j_pos)
            if i_pos == -1:
                continue

            evaluate_score = self._calculate_evaluate_score(rotation, j_pos, i_pos)
            if evaluate_score >= best_score:
                #print(f"(rotation, j_pos, i_pos) = ({rotation}, {j_pos}, {i_pos}) got the new best score: {evaluate_score}")
                # 发现 bug，合着这个集合里只能有一个元素？
                best_score = evaluate_score
                best_moves = set()
                best_moves.add((rotation, j_pos))

        assert len(best_moves) != 0
        assert best_score != -float('inf')

        # 如果不同摆法存在相同的最高评价值，就再按优先级（priority）分出高低。
        # 第一档：优先靠墙。目标位置的横坐标偏离入场位置的程度越大，就越优先，每格记 100 分。
        # 第二档：优先向左。如果第一档同分，就取向左移的摆法，“居左”这一状态记 10 分。
        # 第三档：优先少转。如果前两档同分，就取旋转次数最少的摆法，每次旋转多扣 1 分。

        # 第三档意义不明，所以直接忽略。

        best_move = (-1, -1)
        best_move_priority = -float('inf')

        for rotation, j_pos in best_moves:
            priority = 100 * abs((j_pos + shape_width(TETRIS_SHAPES[self.falling_tetris][rotation]) / 2) - 4.5) + 10 * (9 - j_pos)

            if priority > best_move_priority:
                best_move = (rotation, j_pos)
                best_move_priority = priority


        assert best_move != (-1, -1)
        assert best_move_priority != -float('inf')

        return best_move




    def _calculate_evaluate_score(self, rotation: int, j_pos: int, i_pos: int) -> float:
        # 评估得分。

        # 公式：评价 = −4∗洞数 − 累计井数 − 行转变数 − 列转变数 − 方块着陆高度 + 侵蚀格数
        # 洞（Hole）：洞是正上方存在砖格的空格
        # 井（Well）：左右两侧都是砖格或墙壁的空格
        # 行转变数（Row Transition）：一行中砖格和空格交替出现，交替了几次。墙和砖格等效。
        # 列转变数（Column Transition）：一列中砖格和空格交替出现，交替了几次。墙和砖格等效。
        # 方块着陆高度（Landing Height）：列高 + 块高/2。但如果触发消行，则是取消行后的结果。
        # 侵蚀格数（Number of Eroded Cells）：当前方块的消行数乘以填入被消行的方格数

        # 参考资料：
        # 方块 AI 算法历史 (1996–2013) https://tetris.huijiwiki.com/wiki/%E6%96%B9%E5%9D%97_AI_%E7%AE%97%E6%B3%95%E5%8E%86%E5%8F%B2_(1996%E2%80%932013)
        # Tetris AI (单块, Pierre Dellacherie, 2003) https://tetris.huijiwiki.com/wiki/Tetris_AI_(%E5%8D%95%E5%9D%97,_Pierre_Dellacherie,_2003)



        tetris_shape = TETRIS_SHAPES[self.falling_tetris][rotation]

        new_grid = GameState._place_tetris_into_a_grid(self.grid, self.falling_tetris, rotation, j_pos, i_pos)

        def list_get[T](ls: list[T], ind: int, default: T) -> T:
            return ls[ind] if ind in range(len(ls)) else default

        hole = sum(1 for i in range(20) for j in range(10) if new_grid[i][j] == 0 and any(new_grid[i_][j] for i_ in range(i)))
        hole_depth = sum(sum(new_grid[i_][j] for i_ in range(i)) for i in range(20) for j in range(10) if new_grid[i][j] == 0)
        well = sum(1 for i in range(20) for j in range(10) if new_grid[i][j] == 0 and list_get(new_grid[i], j - 1, 1) == list_get(new_grid[i], j + 1, 1) == 1)
        row_transitions = sum(1 for i in range(20) for j in range(-1, 10) if list_get(new_grid[i], j, 1) != list_get(new_grid[i], j+1, 1))
        col_transitions = sum(1 for i in range(-1, 20) for j in range(10) if list_get(new_grid, i, [1 for _ in range(10)])[j] != list_get(new_grid, i+1, [1 for _ in range(10)])[j])

        full_rows = GameState._all_full_rows(new_grid)
        landing_height = 20 - (i_pos + shape_height(tetris_shape) / 2)
        #landing_height -= sum(1 for r in full_rows if r > landing_height)

        # 又发现一个 bug
        eroded_cells = len(full_rows) * sum(1 for i in range(20) for j in range(10) if new_grid[i][j] == 1 and (i in full_rows))

        evaluate_result = -4 * hole - 1 * well - 1 * row_transitions - col_transitions - max(0, landing_height - 6) - landing_height + eroded_cells

        #print("evaluate (tetris, rotation, j_pos, i_pos) = ({}, {}, {}, {}) -> {}".format(self.falling_tetris, rotation, j_pos, i_pos, evaluate_result))


        return evaluate_result



if __name__ == '__main__':

    first_line = input()
    assert len(first_line) == 2
    first, second = first_line

    game = GameState([[0 for _ in range(10)] for _ in range(20)], first, second, False, Statistics(0, 0, 0, {1: 0, 2: 0, 3: 0, 4: 0}))
    next_tetris = ""

    while True:
        #time.sleep(0.4)
        rotation, j_pos = game.make_decision()
        game = game.the_next_state(rotation, j_pos)

        # 绘制一下正在下落的俄罗斯方块
        shape = TETRIS_SHAPES[game.falling_tetris][0]
        print("       falling tetris")
        print("       +------------+")
        print("       |            |")
        for i in range(4):
            if i in range(shape_height(shape)):
                print("       |  {:8s}  |".format("".join("[]" if cell else "  " for cell in shape[i])))
            else:
                print("       |            |")
        print("       |            |")
        print("       +------------+")
        print(f"    applied action: {rotation} {j_pos}")
        print()
        game.print_grid()
        game.print_statistics()
        print()
        print()

        if game.is_deadline_touched():
            print(f"game over because of deadline touched")
            break

        if next_tetris == 'X':
            print(f"game over because read of 'X'")
            break

        next_tetris = input()
        assert len(next_tetris) == 1

        game = game.with_next_tetris_filled_in(next_tetris)





