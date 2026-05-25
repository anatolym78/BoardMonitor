#include "BoardDataJsonGenerator.h"
#include <QDebug>

BoardDataJsonGenerator::BoardDataJsonGenerator(QObject *parent)
    : QObject(parent)
{
}

QString BoardDataJsonGenerator::createJsonString(const QList<BoardParameterSingle*> &parameters)
{
    QJsonDocument doc = createJsonDocument(parameters);
    QString jsonString = doc.toJson(QJsonDocument::Compact);
    
    if (jsonString.isEmpty()) {
        emit writingError("Не удалось создать JSON строку");
    } else {
        emit writingSuccess(jsonString);
    }
    
    return jsonString;
}

QJsonDocument BoardDataJsonGenerator::createJsonDocument(const QList<BoardParameterSingle*> &parameters)
{
    QJsonArray array = createJsonArray(parameters);
    return QJsonDocument(array);
}

QJsonArray BoardDataJsonGenerator::createJsonArray(const QList<BoardParameterSingle*> &parameters)
{
    QJsonArray array;
    
    for (BoardParameterSingle *param : parameters) {
        if (param) {
            QJsonObject obj = parameterToJsonObject(*param);
            array.append(obj);
        }
    }
    
    return array;
}

QJsonObject BoardDataJsonGenerator::parameterToJsonObject(const BoardParameterSingle &parameter)
{
    QJsonObject obj;
    obj["label"] = parameter.label();
    
    QJsonValue val = QJsonValue::fromVariant(parameter.value());
    obj["value"] = val;

    return obj;
}
