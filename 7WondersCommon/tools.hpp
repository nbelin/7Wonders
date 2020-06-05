#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <initializer_list>

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QDir>

namespace Tools {
    QString createPath(std::initializer_list<QString> list);
    QString imagePath(QString name);
    QString imageWonderPath(QString name);
    QString imageTokenPath(QString name);
    QStringList mySplit(const QString & string, const char * separator);
}

#endif // TOOLS_HPP

