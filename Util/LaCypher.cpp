#include "lacypher.h"

#include <QDebug>

LaCypher::LaCypher(QObject *parent=0)
    : QObject(parent)
{
}

QString LaCypher::cypher(QString word) {
    char mCypher = '$';
    QString newWord;

    for (int i = 0; i < word.size(); ++i) {
        newWord.append( word.toUtf8().at(i) ^ mCypher );
    }
    return newWord;
}
