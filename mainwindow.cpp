#include "mainwindow.h"
#include "bot.h"

#include <QGridLayout>
#include <QMessageBox>
#include <QWidget>
#include <QVBoxLayout>
#include <QComboBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    createBoard();

    // Уровень сложности по умолчанию
    currentDifficulty = Bot::Random;
}

MainWindow::~MainWindow()
{
}

void MainWindow::createBoard()
{
    QWidget *central = new QWidget(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // Выпадающий список сложности
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

    // Игровое поле
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
    resize(700, 750);
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
                // Ход игрока
                board[i][j] = 'X';
                buttons[i][j]->setText("X");

                if (checkWin('X'))
                {
                    QMessageBox::information(
                        this,
                        "Победа",
                        "Игрок победил!"
                    );

                    resetGame();
                    return;
                }

                if (isBoardFull())
                {
                    QMessageBox::information(
                        this,
                        "Ничья",
                        "Поле заполнено!"
                    );

                    resetGame();
                    return;
                }

                // Ход бота
                botMove();

                if (checkWin('O'))
                {
                    QMessageBox::information(
                        this,
                        "Поражение",
                        "Бот победил!"
                    );

                    resetGame();
                    return;
                }

                if (isBoardFull())
                {
                    QMessageBox::information(
                        this,
                        "Ничья",
                        "Поле заполнено!"
                    );

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

bool MainWindow::checkWin(char symbol)
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            // Горизонталь
            if (j + WIN_COUNT <= SIZE)
            {
                bool win = true;

                for (int k = 0; k < WIN_COUNT; k++)
                {
                    if (board[i][j + k] != symbol)
                    {
                        win = false;
                        break;
                    }
                }

                if (win)
                    return true;
            }

            // Вертикаль
            if (i + WIN_COUNT <= SIZE)
            {
                bool win = true;

                for (int k = 0; k < WIN_COUNT; k++)
                {
                    if (board[i + k][j] != symbol)
                    {
                        win = false;
                        break;
                    }
                }

                if (win)
                    return true;
            }

            // Главная диагональ
            if (i + WIN_COUNT <= SIZE &&
                j + WIN_COUNT <= SIZE)
            {
                bool win = true;

                for (int k = 0; k < WIN_COUNT; k++)
                {
                    if (board[i + k][j + k] != symbol)
                    {
                        win = false;
                        break;
                    }
                }

                if (win)
                    return true;
            }

            // Побочная диагональ
            if (i + WIN_COUNT <= SIZE &&
                j - WIN_COUNT + 1 >= 0)
            {
                bool win = true;

                for (int k = 0; k < WIN_COUNT; k++)
                {
                    if (board[i + k][j - k] != symbol)
                    {
                        win = false;
                        break;
                    }
                }

                if (win)
                    return true;
            }
        }
    }

    return false;
}

bool MainWindow::isBoardFull()
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            if (board[i][j] == ' ')
                return false;
        }
    }

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
}

void MainWindow::changeDifficulty(int index)
{
    switch (index)
    {
        case 0:
            currentDifficulty = Bot::Random;
            break;

        case 1:
            currentDifficulty = Bot::Greedy;
            break;

        case 2:
            currentDifficulty = Bot::Minimax;
            break;

        case 3:
            currentDifficulty = Bot::AlphaBeta;
            break;

        default:
            currentDifficulty = Bot::Random;
            break;
    }
}