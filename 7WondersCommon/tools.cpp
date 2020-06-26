#include "tools.hpp"

namespace Tools {
    QString createPath(std::initializer_list<QString> list) {
        QString path = QCoreApplication::applicationDirPath() + QDir::separator() + "..";
        for (auto elem: list)
        {
            path += QDir::separator();
            path += elem;
        }
        return path;
    }

    QString imagePath(QString name) {
        return QString(":/images/") + name;
        return createPath({"..", "7WondersUI", "images", name});
    }

    QString imageCardPath(QString name) {
        return QString(":/images/cards/") + name;
        return createPath({"..", "7WondersUI", "images", "cards", name});
    }

    QString imageWonderPath(QString name) {
        return QString(":/images/wonders/") + name;
        return createPath({"..", "7WondersUI", "images", "wonders", name});
    }

    QString imageTokenPath(QString name) {
        return QString(":/images/tokens/") + name;
        return createPath({"..", "7WondersUI", "images", "tokens", name});
    }

    QStringList mySplit(const QString & string, const char * separator) {
        if (string.isEmpty()) {
            return QStringList();
        }
        return string.split(separator);
    }
}


