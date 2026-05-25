#include "AppConfigurationReader.h"
#include <QFile>
#include <QJsonParseError>
#include <QDebug>

AppConfigurationReader::AppConfigurationReader()
{
}

bool AppConfigurationReader::loadConfiguration(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "AppConfigurationReader: Не удалось открыть файл:" << filePath;
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "AppConfigurationReader: Ошибка парсинга JSON:" << error.errorString();
        return false;
    }
    
    if (!doc.isObject()) {
        qWarning() << "AppConfigurationReader: JSON должен содержать объект";
        return false;
    }
    
    m_configuration = doc.object();
    qDebug() << "AppConfigurationReader: Конфигурация успешно загружена из" << filePath;
    
    return true;
}

QJsonArray AppConfigurationReader::getParametersNode() const
{
    if (m_configuration.contains("Parameters")) {
        QJsonValue paramsValue = m_configuration["Parameters"];
        if (paramsValue.isArray()) {
            return paramsValue.toArray();
        }
    }
    
    qWarning() << "AppConfigurationReader: Узел Parameters не найден или не является массивом";
    return QJsonArray();
}
