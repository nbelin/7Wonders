#include "ui.hpp"
#include "tools.hpp"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(Tools::imagePath("icon.png")));
    UI w;
    w.prepareMenu();
    w.show();

    return a.exec();
}
