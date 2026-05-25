#ifndef PARAMETERTREEARRAYITEM_H
#define PARAMETERTREEARRAYITEM_H

#include "ParameterTreeItem.h"

class ParameterTreeArrayItem : public ParameterTreeItem
{
    Q_OBJECT
public:
    explicit ParameterTreeArrayItem(const QString &label, ParameterTreeItem *parent = nullptr);
    ItemType type() const override { return ItemType::Array; }
    
    int arrayHue() const { return m_arrayHue; }
    int getNextSaturationIndex() { return m_saturationIndex++; }

private:
    int m_arrayHue = 0;
    int m_saturationIndex = 0;
};

#endif // PARAMETERTREEARRAYITEM_H

