#include "ParameterTreeJsonParser.h"
#include "ParameterTreeStorage.h"
#include "ParameterTreeGroupItem.h"
#include "ParameterTreeHistoryItem.h"
#include "ParameterTreeArrayItem.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QTextStream>
#include <QFile>

ParameterTreeJsonParser::ParameterTreeJsonParser(QObject *parent)
    : QObject(parent)
{
}

ParameterTreeStorage* ParameterTreeJsonParser::parseJson(const QString &jsonString)
{
    auto root = new ParameterTreeStorage(this);
    updateJson(jsonString, root);
    return root;
}

QString ParameterTreeJsonParser::toJson(ParameterTreeStorage* root)
{
    if (!root)
    {
        m_lastError = "Root storage is null";
        return QString();
    }
    
    QJsonArray parametersArray;
    
    // Обходим все дочерние элементы корня
    for (int i = 0; i < root->childCount(); ++i)
    {
        auto child = root->child(i);
        if (child)
        {
            serializeTreeItem(child, parametersArray);
        }
    }
    
    // Создаем объект с полем "Parameters"
    QJsonObject rootObject;
    rootObject["Parameters"] = parametersArray;
    
    QJsonDocument doc(rootObject);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QString ParameterTreeJsonParser::toBoardJson(ParameterTreeStorage* root)
{
    if (!root)
    {
        m_lastError = "Root storage is null";
        return QString();
    }

    QJsonObject rootObject;

    for (int i = 0; i < root->childCount(); ++i)
    {
        ParameterTreeItem* child = root->child(i);
        if (!child)
        {
            continue;
        }

        QJsonArray groupArray;
        if (child->type() == ParameterTreeItem::ItemType::Group)
        {
            serializeBoardGroupChildren(child, groupArray);
            rootObject[child->label()] = groupArray;
        }
        else
        {
            appendBoardParameterItem(child, groupArray);
            if (!groupArray.isEmpty())
            {
                rootObject[child->label()] = groupArray;
            }
        }
    }

    QJsonDocument doc(rootObject);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

void ParameterTreeJsonParser::updateJson(const QString &jsonString, ParameterTreeStorage *root)
{
    m_lastError.clear();

	QJsonParseError parseError;
	QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &parseError);

    QFile file("parameters.json");
    if (file.open(QFile::WriteOnly))
    {
        QTextStream textStream(&file);
        textStream << doc.toJson(QJsonDocument::Indented);

        file.close();
    }

    if (parseError.error != QJsonParseError::NoError)
    {
        m_lastError = "Json parse error: " + parseError.errorString();
        return;
    }

    if (doc.isObject())
    {
        const QJsonObject obj = doc.object();

        // Формат с полем "Parameters" (как раньше)
        if (obj.contains("Parameters") && obj["Parameters"].isArray())
        {
            updateJsonFromArray(obj["Parameters"].toArray(), root);
            return;
        }

        if (!obj.isEmpty())
        {
            bool hasGroupedArray = false;
            for (auto it = obj.constBegin(); it != obj.constEnd(); ++it)
            {
                if (it.value().isArray())
                {
                    hasGroupedArray = true;
                    break;
                }
            }
            if (!hasGroupedArray)
            {
                m_lastError = "JSON object has no 'Parameters' array and no grouped parameter arrays.";
                return;
            }
        }

        updateJsonFromGroupedObject(obj, root);
        return;
    }

    if (doc.isArray())
    {
        updateJsonFromArray(doc.array(), root);
        return;
    }

    m_lastError = "Root element is neither a JSON array nor a supported JSON object.";
}

void ParameterTreeJsonParser::updateJsonFromGroupedObject(const QJsonObject &obj, ParameterTreeStorage *root)
{
    m_lastError.clear();

    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it)
    {
        const QString groupKey = it.key();
        const QJsonValue groupVal = it.value();
        if (!groupVal.isArray())
        {
            continue;
        }

        const QJsonArray items = groupVal.toArray();
        for (const QJsonValue &elem : items)
        {
            if (!elem.isObject())
            {
                continue;
            }

            const QJsonObject itemObj = elem.toObject();
            if (!itemObj.contains("label") || !itemObj.contains("value"))
            {
                continue;
            }

            const QString localLabel = itemObj["label"].toString();
            if (localLabel.isEmpty())
            {
                continue;
            }

            QJsonObject merged = itemObj;
            merged["label"] = groupKey + QLatin1String(".") + localLabel;
            processJsonObject(merged, root);
        }
    }
}

void ParameterTreeJsonParser::updateJsonFromArray(const QJsonArray &jsonArray, ParameterTreeStorage *root)
{
    m_lastError.clear();
    
    for (const QJsonValue &value : jsonArray)
    {
        if (value.isObject())
        {
            processJsonObject(value.toObject(), root);
        }
    }
}

QString ParameterTreeJsonParser::getLastError() const
{
    return m_lastError;
}

void ParameterTreeJsonParser::processJsonObject(const QJsonObject &jsonObject, ParameterTreeItem *parent)
{
    if (!jsonObject.contains("label") || !jsonObject.contains("value"))
    {
        return; // Пропускаем объекты без обязательных полей
    }

    QString label = jsonObject["label"].toString();
    QJsonValue value = jsonObject["value"];

    // Извлекаем дополнительные поля для ParameterTreeHistoryItem
    QString control;
    QVariant minValue;
    QVariant maxValue;

    if (jsonObject.contains("control"))
    {
        control = jsonObject["control"].toString();
    }

    if (jsonObject.contains("min"))
    {
        minValue = convertJsonValue(jsonObject["min"]);
    }

    if (jsonObject.contains("max"))
    {
        maxValue = convertJsonValue(jsonObject["max"]);
    }

    processValue(label, value, parent, control, minValue, maxValue);
}

void ParameterTreeJsonParser::processValue(const QString &key, const QJsonValue &value, ParameterTreeItem *parent, 
                                           const QString &control, const QVariant &min, const QVariant &max)
{
    QStringList parts = key.split('.');
    ParameterTreeItem *currentItem = parent;

    for (int i = 0; i < parts.size() - 1; ++i)
    {
        const QString &part = parts[i];
        ParameterTreeItem *child = currentItem->findChildByLabel(part);
        if (!child)
        {
            child = new ParameterTreeGroupItem(part, currentItem);
            currentItem->appendChild(child);
        }
        currentItem = child;
    }

    QString finalKey = parts.last();

    if (value.isArray())
    {
        ParameterTreeItem *arrayItem = currentItem->findChildByLabel(finalKey);
        if (!arrayItem)
        {
            arrayItem = new ParameterTreeArrayItem(finalKey, currentItem);
            currentItem->appendChild(arrayItem);
        }

        QJsonArray array = value.toArray();
        for (int i = 0; i < array.size(); ++i)
        {
            QString itemKey = QString::number(i);
            ParameterTreeHistoryItem *historyItem = static_cast<ParameterTreeHistoryItem*>(arrayItem->findChildByLabel(itemKey));
            if (!historyItem)
            {
                historyItem = new ParameterTreeHistoryItem(itemKey, arrayItem);
                arrayItem->appendChild(historyItem);
            }
            historyItem->addValue(convertJsonValue(array[i]), QDateTime::currentDateTime());
            // Устанавливаем дополнительные поля для элементов массива
            if (!control.isEmpty())
            {
                historyItem->setControl(control);
            }
            if (min.isValid())
            {
                historyItem->setMin(min);
            }
            if (max.isValid())
            {
                historyItem->setMax(max);
            }
        }
    }
    else
    {
        ParameterTreeHistoryItem *historyItem = static_cast<ParameterTreeHistoryItem*>(currentItem->findChildByLabel(finalKey));
        if (!historyItem)
        {
            historyItem = new ParameterTreeHistoryItem(finalKey, currentItem);
            currentItem->appendChild(historyItem);
        }
        historyItem->addValue(convertJsonValue(value), QDateTime::currentDateTime());
        // Устанавливаем дополнительные поля
        if (!control.isEmpty())
        {
            historyItem->setControl(control);
        }
        if (min.isValid())
        {
            historyItem->setMin(min);
        }
        if (max.isValid())
        {
            historyItem->setMax(max);
        }
    }
}

QVariant ParameterTreeJsonParser::convertJsonValue(const QJsonValue &jsonValue)
{
    switch (jsonValue.type())
    {
        case QJsonValue::Bool:
            return jsonValue.toBool();
        case QJsonValue::Double:
            {
                QString str = QString::number(jsonValue.toDouble(), 'f', 10);
                if (str.contains('.'))
                {
                    return jsonValue.toDouble();
                }
                return jsonValue.toInt();
            }
        case QJsonValue::String:
            return jsonValue.toString();
        default:
            return QVariant();
    }
}

void ParameterTreeJsonParser::serializeTreeItem(ParameterTreeItem* item, QJsonArray& jsonArray)
{
    if (!item) return;
    
    // В зависимости от типа элемента формируем JSON
    if (item->type() == ParameterTreeItem::ItemType::History)
    {
        auto historyItem = static_cast<ParameterTreeHistoryItem*>(item);
        
        QJsonObject paramObject;
        paramObject["label"] = historyItem->fullName();
        paramObject["value"] = convertVariantToJson(historyItem->lastValue());
        
        // Добавляем дополнительные поля, если они заданы
        if (!historyItem->control().isEmpty())
        {
            paramObject["control"] = historyItem->control();
        }
        if (historyItem->min().isValid())
        {
            paramObject["min"] = convertVariantToJson(historyItem->min());
        }
        if (historyItem->max().isValid())
        {
            paramObject["max"] = convertVariantToJson(historyItem->max());
        }
        
        jsonArray.append(paramObject);
    }
    else if (item->type() == ParameterTreeItem::ItemType::Array)
    {
        auto arrayItem = static_cast<ParameterTreeArrayItem*>(item);
        
        QJsonArray valueArray;
        for (int i = 0; i < arrayItem->childCount(); ++i)
        {
            auto child = arrayItem->child(i);
            if (child && child->type() == ParameterTreeItem::ItemType::History)
            {
                auto historyChild = static_cast<ParameterTreeHistoryItem*>(child);
                valueArray.append(convertVariantToJson(historyChild->lastValue()));
            }
        }
        
        QJsonObject paramObject;
        paramObject["label"] = arrayItem->fullName();
        paramObject["value"] = valueArray;
        
        // Получаем control/min/max от первого элемента массива (если есть)
        if (arrayItem->childCount() > 0)
        {
            auto firstChild = static_cast<ParameterTreeHistoryItem*>(arrayItem->child(0));
            if (firstChild)
            {
                if (!firstChild->control().isEmpty())
                {
                    paramObject["control"] = firstChild->control();
                }
                if (firstChild->min().isValid())
                {
                    paramObject["min"] = convertVariantToJson(firstChild->min());
                }
                if (firstChild->max().isValid())
                {
                    paramObject["max"] = convertVariantToJson(firstChild->max());
                }
            }
        }
        
        jsonArray.append(paramObject);
    }
    else if (item->type() == ParameterTreeItem::ItemType::Group)
    {
        // Для группы рекурсивно сериализуем все дочерние элементы
        for (int i = 0; i < item->childCount(); ++i)
        {
            serializeTreeItem(item->child(i), jsonArray);
        }
    }
}

QJsonValue ParameterTreeJsonParser::convertVariantToJson(const QVariant& variant)
{
    switch (variant.type())
    {
        case QVariant::Bool:
            return QJsonValue(variant.toBool());
        case QVariant::Int:
            return QJsonValue(variant.toInt());
        case QVariant::Double:
            return QJsonValue(variant.toDouble());
        case QVariant::String:
            return QJsonValue(variant.toString());
        case QVariant::LongLong:
            return QJsonValue(static_cast<qint64>(variant.toLongLong()));
        case QVariant::ULongLong:
            return QJsonValue(static_cast<qint64>(variant.toULongLong()));
        default:
            return QJsonValue(variant.toString());
    }
}

void ParameterTreeJsonParser::serializeBoardGroupChildren(ParameterTreeItem* group, QJsonArray& groupArray)
{
    if (!group)
    {
        return;
    }

    for (int i = 0; i < group->childCount(); ++i)
    {
        appendBoardParameterItem(group->child(i), groupArray);
    }
}

void ParameterTreeJsonParser::appendBoardParameterItem(ParameterTreeItem* item, QJsonArray& groupArray)
{
    if (!item)
    {
        return;
    }

    if (item->type() == ParameterTreeItem::ItemType::History)
    {
        auto historyItem = static_cast<ParameterTreeHistoryItem*>(item);

        QJsonObject paramObject;
        paramObject["label"] = historyItem->label();
        paramObject["value"] = convertVariantToJson(historyItem->lastValue());
        groupArray.append(paramObject);
    }
    else if (item->type() == ParameterTreeItem::ItemType::Array)
    {
        auto arrayItem = static_cast<ParameterTreeArrayItem*>(item);

        QJsonArray valueArray;
        for (int i = 0; i < arrayItem->childCount(); ++i)
        {
            auto child = arrayItem->child(i);
            if (child && child->type() == ParameterTreeItem::ItemType::History)
            {
                auto historyChild = static_cast<ParameterTreeHistoryItem*>(child);
                valueArray.append(convertVariantToJson(historyChild->lastValue()));
            }
        }

        QJsonObject paramObject;
        paramObject["label"] = arrayItem->label();
        paramObject["value"] = valueArray;
        groupArray.append(paramObject);
    }
    else if (item->type() == ParameterTreeItem::ItemType::Group)
    {
        for (int i = 0; i < item->childCount(); ++i)
        {
            appendBoardParameterItem(item->child(i), groupArray);
        }
    }
}
