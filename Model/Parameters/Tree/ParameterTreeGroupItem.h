#ifndef PARAMETERTREEGROUPITEM_H
#define PARAMETERTREEGROUPITEM_H

#include "ParameterTreeItem.h"

class ParameterTreeGroupItem : public ParameterTreeItem
{
    Q_OBJECT
public:
    explicit ParameterTreeGroupItem(const QString &label, ParameterTreeItem *parent = nullptr);
    ItemType type() const override { return ItemType::Group; }
};

#endif // PARAMETERTREEGROUPITEM_H
