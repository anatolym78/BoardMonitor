#ifndef PARAMETERTREEITEM_H
#define PARAMETERTREEITEM_H

#include <QObject>
#include <QList>
#include <QString>
#include <QVariant>
#include <QColor>

class ParameterTreeItem : public QObject
{
    Q_OBJECT

public:
    explicit ParameterTreeItem(const QString &label, ParameterTreeItem *parent = nullptr);
    ~ParameterTreeItem() override;

    void appendChild(ParameterTreeItem *child);
    void removeChild(ParameterTreeItem *child);

    ParameterTreeItem *child(int row);
    ParameterTreeItem* findChildByLabel(const QString& label, bool recursive = false);
    int childCount() const;
    int row() const;
    ParameterTreeItem *parentItem() const;
    int level() const;

    QString label() const;
    QString fullName() const;
    QList<ParameterTreeItem*> children() const;
    QColor color() const;
    
    bool isChartVisible() const;
    void setIsChartVisible(bool visible);

    enum class ItemType {
        Root,
        Group,
        History,
        Array
    };
    virtual ItemType type() const = 0;

private:
    Q_DISABLE_COPY(ParameterTreeItem)

protected:
    QString m_label;
    ParameterTreeItem *m_parentItem;
    QList<ParameterTreeItem*> m_childItems;
    QColor m_color = Qt::black;
    bool m_isChartVisible = false;
};

#endif // PARAMETERTREEITEM_H
