#include "board.hpp"

#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Board board;

    return a.exec();
}

