#include "bot.h"

#include <QRandomGenerator>
#include <QHash>
#include <algorithm>

namespace
{
    const int size = 10;
    uint64_t zobrist[size][size][2];
    bool zobristInitialized = false;

    void initZobrist()
    {
        if (zobristInitialized) return;

        for (int i = 0; i < size; i++)
        {
            for (int j = 0; j < size; j++)
            {
                zobrist[i][j][0] = QRandomGenerator::global()->generate64();
                zobrist[i][j][1] = QRandomGenerator::global()->generate64();
            }
        }

        zobristInitialized = true;
    }

    uint64_t computeHash(char board[size][size])
    {
        uint64_t h = 0;

        for (int i = 0; i < size; i++)
        {
            for (int j = 0; j < size; j++)
            {
                if (board[i][j] == 'O')
                    h ^= zobrist[i][j][0];
                else if (board[i][j] == 'X')
                    h ^= zobrist[i][j][1];
            }
        }

        return h;
    }

    QHash<uint64_t, int> transpositionTable;

    bool quickCheckWin(char board[size][size], int x, int y, char symbol)
    {
        const int dirs[4][2] = {
            {1, 0}, {0, 1}, {1, 1}, {1, -1}
        };

        for (auto& d : dirs)
        {
            int count = 1;

            for (int s = -1; s <= 1; s += 2)
            {
                int dx = d[0] * s;
                int dy = d[1] * s;

                int nx = x + dx;
                int ny = y + dy;

                while (nx >= 0 && nx < size &&
                       ny >= 0 && ny < size &&
                       board[nx][ny] == symbol)
                {
                    count++;
                    nx += dx;
                    ny += dy;
                }
            }

            if (count >= 5) return true;
        }

        return false;
    }

    int evaluateMove(char board[size][size], int x, int y, char symbol)
    {
        const int dirs[4][2] = {
            {1, 0}, {0, 1}, {1, 1}, {1, -1}
        };

        int total = 0;

        for (auto& d : dirs)
        {
            int count = 1;

            for (int s = -1; s <= 1; s += 2)
            {
                int dx = d[0] * s;
                int dy = d[1] * s;

                int nx = x + dx;
                int ny = y + dy;

                while (nx >= 0 && nx < size &&
                       ny >= 0 && ny < size &&
                       board[nx][ny] == symbol)
                {
                    count++;
                    nx += dx;
                    ny += dy;
                }
            }

            switch (count)
            {
                case 2: total += 50; break;
                case 3: total += 500; break;
                case 4: total += 5000; break;
                case 5: total += 100000; break;
            }
        }

        return total;
    }
}


QPair<int, int> Bot::makeMove(char board[SIZE][SIZE], Difficulty level)
{
    initZobrist();

    switch (level)
    {
        case Random: return randomMove(board);
        case Greedy: return greedyMove(board);
        case Minimax: return minimaxMove(board);
        case AlphaBeta: return alphaBetaMove(board);
    }

    return randomMove(board);
}

QPair<int, int> Bot::randomMove(char board[SIZE][SIZE])
{
    QVector<QPair<int,int>> empty;

    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            if (board[i][j] == ' ')
                empty.append({i,j});

    if (empty.isEmpty()) return {-1,-1};

    return empty[QRandomGenerator::global()->bounded(empty.size())];
}

QVector<QPair<int,int>> Bot::generateCandidateMoves(char board[SIZE][SIZE])
{
    QVector<QPair<int,int>> moves;
    bool used[SIZE][SIZE] = {false};
    bool hasAny = false;

    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            if (board[i][j] != ' ')
            {
                hasAny = true;

                for (int dx = -SEARCH_RADIUS; dx <= SEARCH_RADIUS; dx++)
                {
                    for (int dy = -SEARCH_RADIUS; dy <= SEARCH_RADIUS; dy++)
                    {
                        int nx = i + dx;
                        int ny = j + dy;

                        if (nx < 0 || ny < 0 || nx >= SIZE || ny >= SIZE)
                            continue;

                        if (board[nx][ny] == ' ' && !used[nx][ny])
                        {
                            used[nx][ny] = true;
                            moves.append({nx,ny});
                        }
                    }
                }
            }
        }
    }

    if (!hasAny)
        moves.append({SIZE/2, SIZE/2});

    return moves;
}

QPair<int,int> Bot::greedyMove(char board[SIZE][SIZE])
{
    int bestScore = -1e9;
    QPair<int,int> best{-1,-1};

    auto moves = generateCandidateMoves(board);

    for (auto& m : moves)
    {
        int x = m.first, y = m.second;

        board[x][y] = 'O';
        int score = evaluateMove(board, x, y, 'O');
        board[x][y] = ' ';

        if (score > bestScore)
        {
            bestScore = score;
            best = m;
        }
    }

    return best.first == -1 ? randomMove(board) : best;
}

QPair<int,int> Bot::alphaBetaMove(char board[SIZE][SIZE])
{
    int bestScore = -1e9;
    QPair<int,int> best{-1,-1};

    const int MAX_DEPTH = 3;

    auto moves = generateCandidateMoves(board);

    struct MS { QPair<int,int> m; int score; };
    QVector<MS> ordered;

    for (auto& m : moves)
    {
        int x = m.first, y = m.second;

        board[x][y] = 'O';

        if (quickCheckWin(board,x,y,'O'))
        {
            board[x][y] = ' ';
            return m;
        }

        int score = evaluateMove(board,x,y,'O');

        // блокировка
        board[x][y] = 'X';
        if (quickCheckWin(board,x,y,'X'))
            score += 90000;
        board[x][y] = ' ';

        ordered.append({m,score});
    }

    std::sort(ordered.begin(), ordered.end(),
              [](auto&a, auto&b){return a.score>b.score;});

    for (auto& item : ordered)
    {
        int x=item.m.first,y=item.m.second;

        board[x][y]='O';

        int score = alphaBeta(board,MAX_DEPTH,-1e9,1e9,false,x,y);

        board[x][y]=' ';

        if (score > bestScore)
        {
            bestScore = score;
            best = item.m;
        }
    }

    return best.first==-1 ? randomMove(board) : best;
}

int Bot::alphaBeta(char board[SIZE][SIZE], int depth,
                   int alpha, int beta, bool max,
                   int lastX, int lastY)
{
    uint64_t key = computeHash(board) ^ depth ^ (max ? 1ULL<<63 : 0);

    if (transpositionTable.contains(key))
        return transpositionTable[key];

    if (lastX != -1)
    {
        if (quickCheckWin(board,lastX,lastY, max?'X':'O'))
            return max ? -100000 : 100000;
    }

    if (depth == 0)
        return 0;

    auto moves = generateCandidateMoves(board);

    int best = max ? -1e9 : 1e9;

    for (auto& m : moves)
    {
        int x=m.first,y=m.second;

        board[x][y] = max?'O':'X';

        int score = alphaBeta(board,depth-1,alpha,beta,!max,x,y);

        board[x][y] = ' ';

        if (max)
        {
            best = std::max(best, score);
            alpha = std::max(alpha, best);
        }
        else
        {
            best = std::min(best, score);
            beta = std::min(beta, best);
        }

        if (beta <= alpha)
            break;
    }

    transpositionTable[key] = best;
    return best;
}

QPair<int,int> Bot::minimaxMove(char board[SIZE][SIZE])
{
    return alphaBetaMove(board);
}

bool Bot::isBoardFull(char board[SIZE][SIZE])
{
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            if (board[i][j] == ' ')
                return false;
    return true;
}