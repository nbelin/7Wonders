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
        return createPath({"..", "7Wonders", "images", name});
    }

    QString imageWonderPath(QString name) {
        return createPath({"..", "7Wonders", "images", "wonders", name});
    }

    QString imageTokenPath(QString name) {
        return createPath({"..", "7Wonders", "images", "tokens", name});
    }

    QStringList mySplit(const QString & string, const char * separator) {
        if (string.isEmpty()) {
            return QStringList();
        }
        return string.split(separator);
    }
}


