#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QComboBox>
#include "bot.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleButtonClick();
    void changeDifficulty(int index);

private:
    static const int SIZE = 10;
    static const int WIN_COUNT = 5;

    QPushButton* buttons[SIZE][SIZE];
    char board[SIZE][SIZE];

    QComboBox* difficultyBox;

    Bot::Difficulty currentDifficulty;

    void createBoard();
    void botMove();
    bool checkWin(char symbol);
    bool isBoardFull();
    void resetGame();
};

#endif // MAINWINDOW_H