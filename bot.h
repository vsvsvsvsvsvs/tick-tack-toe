#ifndef BOT_H
#define BOT_H

#include <QPair>
#include <QVector>

class Bot {
public:
    enum Difficulty { Random = 1, Greedy = 2, Minimax = 3, AlphaBeta = 4 };

    static QPair<int, int> makeMove(char board[10][10], Difficulty level);

private:
    static const int SIZE = 10;
    static const int WIN_COUNT = 5;

    static const int SEARCH_RADIUS = 2;

    static QPair<int, int> randomMove(char board[SIZE][SIZE]);
    static QPair<int, int> greedyMove(char board[SIZE][SIZE]);
    static QPair<int, int> minimaxMove(char board[SIZE][SIZE]);
    static QPair<int, int> alphaBetaMove(char board[SIZE][SIZE]);

    static QVector<QPair<int, int>> generateCandidateMoves(char board[SIZE][SIZE]);

    static int alphaBeta(char board[SIZE][SIZE], int depth, int alpha, int beta, bool isMaximizing, int lastX,
                         int lastY);

    static bool isBoardFull(char board[SIZE][SIZE]);
};

#endif // BOT_H
