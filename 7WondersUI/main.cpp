#include "ui.hpp"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    UI w;
    w.prepareMenu();
    w.show();

    return a.exec();
}
