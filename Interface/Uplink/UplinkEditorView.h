#ifndef UPLINKPARAMETERSTREEVIEW_H
#define UPLINKPARAMETERSTREEVIEW_H

#include <QTreeView>

class UplinkEditorDelegates;
class DriverAdapter;
class UplinkParametersTreeModel;

/**
 * @brief Редактор параметров для отправки на борт (Uplink).
 * 
 * Отображает дерево настраиваемых параметров и предоставляет виджеты
 * (слайдеры, спинбоксы и т.д.) для их изменения.
 * Позволяет формировать и отправлять команды управления дрону.
 */
class UplinkEditorView : public QTreeView
{
    Q_OBJECT
public:
    explicit UplinkEditorView(QWidget *parent = nullptr);
    void setModel(QAbstractItemModel* model) override;

    void setSendDataImmediately(bool immediately);
    
    void setDriverAdapter(DriverAdapter* adapter) { m_driverAdapter = adapter; }
    void setParametersModel(UplinkParametersTreeModel* model) { m_parametersModel = model; }

    UplinkEditorDelegates* getDelegate() const { return m_delegate; }

private:
    void setupControlWidgets();
    void setupControlWidgetsRecursive(const QModelIndex &parent = QModelIndex());
    void clearControlWidgetsRecursive(const QModelIndex &parent = QModelIndex());
    QWidget* createControlWidget(const QModelIndex &index) const;
    void sendParameterSnapshot(const QModelIndex &index, const QVariant &value) const;

private:
    bool m_sendDataImmediately = false;
    UplinkEditorDelegates* m_delegate = nullptr;
    DriverAdapter* m_driverAdapter = nullptr;
    UplinkParametersTreeModel* m_parametersModel = nullptr;
};

#endif // UPLINKPARAMETERSTREEVIEW_H

