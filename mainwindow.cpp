#include "mainwindow.h"

#include <QGridLayout>
#include <QMessageBox>
#include <QWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    createBoard();

    currentDifficulty = Bot::Random;
}

MainWindow::~MainWindow()
{
}

void MainWindow::createBoard()
{
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // ===== РЕЖИМ ИГРЫ =====
    modeBox = new QComboBox();
    modeBox->addItem("Player vs Bot");
    modeBox->addItem("Player vs Player");

    connect(
        modeBox,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        &MainWindow::changeMode
    );

    mainLayout->addWidget(modeBox);

    // ===== СЛОЖНОСТЬ =====
    difficultyBox = new QComboBox();
    difficultyBox->addItem("Random");
    difficultyBox->addItem("Greedy");
    difficultyBox->addItem("Minimax");
    difficultyBox->addItem("Alpha-Beta");

    connect(
        difficultyBox,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        &MainWindow::changeDifficulty
    );

    mainLayout->addWidget(difficultyBox);

    // ===== ИНДИКАТОР ХОДА =====
    turnLabel = new QLabel();
    turnLabel->setText("Ход: X");
    mainLayout->addWidget(turnLabel);

    // ===== ПОЛЕ =====
    QGridLayout *gridLayout = new QGridLayout();

    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            buttons[i][j] = new QPushButton("");
            buttons[i][j]->setFixedSize(50, 50);

            board[i][j] = ' ';

            connect(
                buttons[i][j],
                &QPushButton::clicked,
                this,
                &MainWindow::handleButtonClick
            );

            gridLayout->addWidget(buttons[i][j], i, j);
        }
    }

    mainLayout->addLayout(gridLayout);

    setCentralWidget(central);
    setWindowTitle("Крестики-нолики 10x10");
    resize(700, 800);
}

void MainWindow::handleButtonClick()
{
    QPushButton *clicked =
        qobject_cast<QPushButton*>(sender());

    if (!clicked)
        return;

    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            if (buttons[i][j] == clicked && board[i][j] == ' ')
            {
                // ===== PvP режим =====
                if (isPvP)
                {
                    board[i][j] = currentPlayer;
                    buttons[i][j]->setText(QString(currentPlayer));

                    if (checkWin(currentPlayer))
                    {
                        QMessageBox::information(
                            this,
                            "Победа",
                            QString("Игрок %1 победил!").arg(currentPlayer)
                        );
                        resetGame();
                        return;
                    }

                    if (isBoardFull())
                    {
                        QMessageBox::information(this, "Ничья", "Поле заполнено!");
                        resetGame();
                        return;
                    }

                    // смена игрока
                    currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
                    updateTurnLabel();

                    return;
                }

                // ===== PvBot =====
                board[i][j] = 'X';
                buttons[i][j]->setText("X");

                if (checkWin('X'))
                {
                    QMessageBox::information(this, "Победа", "Игрок победил!");
                    resetGame();
                    return;
                }

                if (isBoardFull())
                {
                    QMessageBox::information(this, "Ничья", "Поле заполнено!");
                    resetGame();
                    return;
                }

                botMove();

                if (checkWin('O'))
                {
                    QMessageBox::information(this, "Поражение", "Бот победил!");
                    resetGame();
                    return;
                }

                if (isBoardFull())
                {
                    QMessageBox::information(this, "Ничья", "Поле заполнено!");
                    resetGame();
                    return;
                }

                return;
            }
        }
    }
}

void MainWindow::botMove()
{
    QPair<int, int> move =
        Bot::makeMove(board, currentDifficulty);

    int x = move.first;
    int y = move.second;

    if (x == -1 || y == -1)
        return;

    board[x][y] = 'O';
    buttons[x][y]->setText("O");
}

void MainWindow::updateTurnLabel()
{
    turnLabel->setText(QString("Ход: %1").arg(currentPlayer));
}

void MainWindow::changeMode(int index)
{
    isPvP = (index == 1);
    resetGame();
}

void MainWindow::changeDifficulty(int index)
{
    switch (index)
    {
        case 0: currentDifficulty = Bot::Random; break;
        case 1: currentDifficulty = Bot::Greedy; break;
        case 2: currentDifficulty = Bot::Minimax; break;
        case 3: currentDifficulty = Bot::AlphaBeta; break;
    }
}

bool MainWindow::checkWin(char symbol)
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            if (j + WIN_COUNT <= SIZE)
            {
                bool win = true;
                for (int k = 0; k < WIN_COUNT; k++)
                    if (board[i][j + k] != symbol) win = false;
                if (win) return true;
            }

            if (i + WIN_COUNT <= SIZE)
            {
                bool win = true;
                for (int k = 0; k < WIN_COUNT; k++)
                    if (board[i + k][j] != symbol) win = false;
                if (win) return true;
            }

            if (i + WIN_COUNT <= SIZE && j + WIN_COUNT <= SIZE)
            {
                bool win = true;
                for (int k = 0; k < WIN_COUNT; k++)
                    if (board[i + k][j + k] != symbol) win = false;
                if (win) return true;
            }

            if (i + WIN_COUNT <= SIZE && j - WIN_COUNT + 1 >= 0)
            {
                bool win = true;
                for (int k = 0; k < WIN_COUNT; k++)
                    if (board[i + k][j - k] != symbol) win = false;
                if (win) return true;
            }
        }
    }
    return false;
}

bool MainWindow::isBoardFull()
{
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            if (board[i][j] == ' ')
                return false;
    return true;
}

void MainWindow::resetGame()
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            board[i][j] = ' ';
            buttons[i][j]->setText("");
        }
    }

    currentPlayer = 'X';
    updateTurnLabel();
}