#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QComboBox>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include "bot.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QComboBox *sideBox;

    char playerSymbol = 'X';
    char botSymbol = 'O';

    void handleButtonClick();
    void changeDifficulty(int index);
    void changeMode(int index);

    static const int SIZE = 10;
    static const int WIN_COUNT = 5;

    QPushButton *buttons[SIZE][SIZE];
    char board[SIZE][SIZE];

    QComboBox *difficultyBox;
    QComboBox *modeBox;
    QLabel *turnLabel;

    Bot::Difficulty currentDifficulty;

    bool isPvP = false;
    char currentPlayer = 'X';

    void createBoard();
    void botMove();
    bool checkWin(char symbol);
    bool isBoardFull();
    void resetGame();
    void updateTurnLabel();
};

#endif // MAINWINDOW_H
