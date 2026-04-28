#include "bot.h"

#include <QHash>
#include <QRandomGenerator>
#include <algorithm>

namespace {
    const int size = 10;
    uint64_t zobrist[size][size][2];
    bool zobristInitialized = false;

    void initZobrist() {
        if (zobristInitialized)
            return;

        for (int i = 0; i < size; i++)
            for (int j = 0; j < size; j++) {
                zobrist[i][j][0] = QRandomGenerator::global()->generate64();
                zobrist[i][j][1] = QRandomGenerator::global()->generate64();
            }

        zobristInitialized = true;
    }

    uint64_t computeHash(char board[size][size]) {
        uint64_t h = 0;

        for (int i = 0; i < size; i++)
            for (int j = 0; j < size; j++) {
                if (board[i][j] == 'O')
                    h ^= zobrist[i][j][0];
                else if (board[i][j] == 'X')
                    h ^= zobrist[i][j][1];
            }

        return h;
    }

    QHash<uint64_t, int> transpositionTable;

    bool quickCheckWin(char board[size][size], int x, int y, char symbol) {
        const int dirs[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

        for (auto &d: dirs) {
            int count = 1;

            for (int s = -1; s <= 1; s += 2) {
                int dx = d[0] * s;
                int dy = d[1] * s;

                int nx = x + dx;
                int ny = y + dy;

                while (nx >= 0 && ny >= 0 && nx < size && ny < size && board[nx][ny] == symbol) {
                    count++;
                    nx += dx;
                    ny += dy;
                }
            }

            if (count >= 5)
                return true;
        }

        return false;
    }

    int evaluateMove(char board[10][10], int x, int y, char symbol) {
        const int dirs[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
        int total = 0;

        for (auto &d: dirs) {
            int count = 1;
            int openEnds = 0;

            int nx = x + d[0];
            int ny = y + d[1];

            while (nx >= 0 && ny >= 0 && nx < 10 && ny < 10 && board[nx][ny] == symbol) {
                count++;
                nx += d[0];
                ny += d[1];
            }

            if (nx >= 0 && ny >= 0 && nx < 10 && ny < 10 && board[nx][ny] == ' ')
                openEnds++;

            nx = x - d[0];
            ny = y - d[1];

            while (nx >= 0 && ny >= 0 && nx < 10 && ny < 10 && board[nx][ny] == symbol) {
                count++;
                nx -= d[0];
                ny -= d[1];
            }

            if (nx >= 0 && ny >= 0 && nx < 10 && ny < 10 && board[nx][ny] == ' ')
                openEnds++;

            if (count >= 5)
                total += 100000000;

            else if (count == 4) {
                if (openEnds == 2)
                    total += 100000000;
                else if (openEnds == 1)
                    total += 100000;
            } else if (count == 3) {
                if (openEnds == 2)
                    total += 10000;
                else if (openEnds == 1)
                    total += 500;
            } else if (count == 2) {
                if (openEnds == 2)
                    total += 200;
                else if (openEnds == 1)
                    total += 50;
            }
        }

        return total;
    }

    int evaluateBoard(char board[10][10]) {
        int score = 0;

        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 10; j++) {
                if (board[i][j] == 'O')
                    score += evaluateMove(board, i, j, 'O');

                else if (board[i][j] == 'X')
                    score -= static_cast<int>(static_cast<double>(evaluateMove(board, i, j, 'X')) * 2);
            }

        return score;
    }

    QPair<int, int> findWinningMove(char board[10][10], char symbol) {
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 10; j++) {
                if (board[i][j] != ' ')
                    continue;

                board[i][j] = symbol;

                if (quickCheckWin(board, i, j, symbol)) {
                    board[i][j] = ' ';
                    return {i, j};
                }

                board[i][j] = ' ';
            }

        return {-1, -1};
    }

    bool createsThreat(char board[10][10], int x, int y, char symbol) {
        const int dirs[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

        for (auto &d: dirs) {
            int count = 1;
            int openEnds = 0;

            for (int s = -1; s <= 1; s += 2) {
                int dx = d[0] * s;
                int dy = d[1] * s;

                int nx = x + dx;
                int ny = y + dy;

                while (nx >= 0 && ny >= 0 && nx < 10 && ny < 10 && board[nx][ny] == symbol) {
                    count++;
                    nx += dx;
                    ny += dy;
                }

                if (nx >= 0 && ny >= 0 && nx < 10 && ny < 10 && board[nx][ny] == ' ')
                    openEnds++;
            }

            if (count == 4 && openEnds >= 1)
                return true;
            if (count == 3 && openEnds == 2)
                return true;
        }

        return false;
    }

    QVector<QPair<int, int>> findAllThreats(char board[10][10], char symbol) {
        QVector<QPair<int, int>> threats;

        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 10; j++) {
                if (board[i][j] != ' ')
                    continue;

                board[i][j] = symbol;

                if (createsThreat(board, i, j, symbol))
                    threats.append({i, j});

                board[i][j] = ' ';
            }

        return threats;
    }
} // namespace

QPair<int, int> Bot::makeMove(char board[SIZE][SIZE], Difficulty level) {
    initZobrist();

    switch (level) {
        case Random:
            return randomMove(board);
        case Greedy:
            return greedyMove(board);
        case Minimax:
        case AlphaBeta:
            return alphaBetaMove(board);
    }

    return randomMove(board);
}

QPair<int, int> Bot::randomMove(char board[SIZE][SIZE]) {
    QVector<QPair<int, int>> empty;

    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            if (board[i][j] == ' ')
                empty.append({i, j});

    if (empty.isEmpty())
        return {-1, -1};

    return empty[QRandomGenerator::global()->bounded(empty.size())];
}

QVector<QPair<int, int>> Bot::generateCandidateMoves(char board[SIZE][SIZE]) {
    QVector<QPair<int, int>> moves;
    bool used[SIZE][SIZE] = {false};
    bool hasAny = false;

    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++) {
            if (board[i][j] != ' ') {
                hasAny = true;

                for (int dx = -SEARCH_RADIUS; dx <= SEARCH_RADIUS; dx++)
                    for (int dy = -SEARCH_RADIUS; dy <= SEARCH_RADIUS; dy++) {
                        int nx = i + dx;
                        int ny = j + dy;

                        if (nx < 0 || ny < 0 || nx >= SIZE || ny >= SIZE)
                            continue;

                        if (board[nx][ny] == ' ' && !used[nx][ny]) {
                            used[nx][ny] = true;
                            moves.append({nx, ny});
                        }
                    }
            }
        }

    if (!hasAny)
        moves.append({SIZE / 2, SIZE / 2});

    return moves;
}

QPair<int, int> Bot::greedyMove(char board[SIZE][SIZE]) {
    int bestScore = -1e9;
    QPair<int, int> best{-1, -1};

    auto moves = generateCandidateMoves(board);

    for (auto &m: moves) {
        int x = m.first, y = m.second;

        board[x][y] = 'O';
        int score = evaluateBoard(board);
        board[x][y] = ' ';

        if (score > bestScore) {
            bestScore = score;
            best = m;
        }
    }

    return best.first == -1 ? randomMove(board) : best;
}

QPair<int, int> Bot::alphaBetaMove(char board[SIZE][SIZE]) {
    int stones = 0;
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 10; j++)
            if (board[i][j] != ' ')
                stones++;

    if (stones < 2)
        return {5, 5};

    // выиграть
    auto win = findWinningMove(board, 'O');
    if (win.first != -1)
        return win;

    // блокировать
    auto block = findWinningMove(board, 'X');
    if (block.first != -1)
        return block;

    // 🔥 блок угроз
    // 🔥 блок угроз (умный выбор, а не первая попавшаяся)
    auto threats = findAllThreats(board, 'X');
    if (!threats.isEmpty())
    {
        int bestScore = -1e9;
        QPair<int,int> best = threats[0];

        for (auto& t : threats)
        {
            int x = t.first;
            int y = t.second;

            board[x][y] = 'O';

            int score = evaluateBoard(board);

            board[x][y] = ' ';

            if (score > bestScore)
            {
                bestScore = score;
                best = t;
            }
        }

        return best;
    }

    int bestScore = -1e9;
    QPair<int, int> best{-1, -1};

    const int MAX_DEPTH = 3;

    auto moves = generateCandidateMoves(board);

    struct MS {
        QPair<int, int> m;
        int s;
    };
    QVector<MS> ordered;

    for (auto &m: moves) {
        int x = m.first, y = m.second;

        board[x][y] = 'O';
        int s = evaluateMove(board, x, y, 'O');
        board[x][y] = ' ';

        ordered.append({m, s});
    }

    std::sort(ordered.begin(), ordered.end(), [](auto &a, auto &b) { return a.s > b.s; });

    if (ordered.size() > 10)
        ordered.resize(10);

    for (auto &item: ordered) {
        int x = item.m.first, y = item.m.second;

        board[x][y] = 'O';

        int score = alphaBeta(board, MAX_DEPTH, -1e9, 1e9, false, x, y);

        board[x][y] = ' ';

        if (score > bestScore) {
            bestScore = score;
            best = item.m;
        }
    }

    return best.first == -1 ? randomMove(board) : best;
}

int Bot::alphaBeta(char board[SIZE][SIZE], int depth, int alpha, int beta, bool max, int lastX, int lastY) {
    uint64_t key = computeHash(board) ^ depth ^ (max ? 1ULL << 63 : 0);

    if (transpositionTable.contains(key))
        return transpositionTable[key];

    if (lastX != -1) {
        if (quickCheckWin(board, lastX, lastY, max ? 'X' : 'O'))
            return max ? -1000000 : 1000000;
    }

    if (depth == 0)
        return evaluateBoard(board);

    auto moves = generateCandidateMoves(board);

    std::sort(moves.begin(), moves.end(), [&](auto &a, auto &b) {
        board[a.first][a.second] = max ? 'O' : 'X';
        int sa = evaluateMove(board, a.first, a.second, max ? 'O' : 'X');
        board[a.first][a.second] = ' ';

        board[b.first][b.second] = max ? 'O' : 'X';
        int sb = evaluateMove(board, b.first, b.second, max ? 'O' : 'X');
        board[b.first][b.second] = ' ';

        return sa > sb;
    });

    if (moves.size() > 8)
        moves.resize(8);

    int best = max ? -1e9 : 1e9;

    for (auto &m: moves) {
        int x = m.first, y = m.second;

        board[x][y] = max ? 'O' : 'X';

        int score = alphaBeta(board, depth - 1, alpha, beta, !max, x, y);

        board[x][y] = ' ';

        if (max) {
            best = std::max(best, score);
            alpha = std::max(alpha, best);
        } else {
            best = std::min(best, score);
            beta = std::min(beta, best);
        }

        if (beta <= alpha)
            break;
    }

    transpositionTable[key] = best;
    return best;
}

QPair<int, int> Bot::minimaxMove(char board[SIZE][SIZE]) { return alphaBetaMove(board); }

bool Bot::isBoardFull(char board[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            if (board[i][j] == ' ')
                return false;
    return true;
}
