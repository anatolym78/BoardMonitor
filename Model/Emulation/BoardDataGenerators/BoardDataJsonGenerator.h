#ifndef BOARDDATAJSONGENERATOR_H
#define BOARDDATAJSONGENERATOR_H

#include <QObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include "../../Parameters/BoardParameterSingle.h"

class BoardDataJsonGenerator : public QObject
{
    Q_OBJECT

public:
    explicit BoardDataJsonGenerator(QObject *parent = nullptr);
    
    // Создание JSON строки из списка параметров
    QString createJsonString(const QList<BoardParameterSingle*> &parameters);
    
    // Создание JSON документа из списка параметров
    QJsonDocument createJsonDocument(const QList<BoardParameterSingle*> &parameters);
    
    // Создание JSON массива из списка параметров
    QJsonArray createJsonArray(const QList<BoardParameterSingle*> &parameters);

signals:
    void writingError(const QString &error);
    void writingSuccess(const QString &jsonString);

private:
    // Преобразование параметра в JSON объект
    QJsonObject parameterToJsonObject(const BoardParameterSingle &parameter);
};

#endif // BOARDDATAJSONGENERATOR_H
