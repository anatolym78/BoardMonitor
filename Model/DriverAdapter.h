#ifndef DRIVERADAPTER_H
#define DRIVERADAPTER_H

#include <QObject>

//#include "./../Services/RCDriver/driverinterface.h"
#include "./../Services/RCImitator/src/driver.hh"
#include "./../Services/RCImitator/src/builder.hh"

#include <boost/dll/import.hpp>
#include <boost/dll/shared_library.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/system/system_error.hpp>

#include <iostream>
#include <optional>
#include <string>

#include "Parameters/Tree/ParameterTreeStorage.h"

class ParameterTreeJsonParser;

class DriverAdapter : public QObject
{
    Q_OBJECT

public:
    explicit DriverAdapter(QObject *parent = nullptr);
    ~DriverAdapter();

    // Методы управления прослушиванием
    void startListening();
    void stopListening();
    
    // Метод для отправки параметра на борт
    void sendParameterTreeSnapshot(ParameterTreeStorage* snapshot);

signals:
    // Сигнал для древовидных параметров
    void parameterTreeReceived(ParameterTreeStorage* root);
    void driverStateChanged(radio::IDriver::State state);
    void driverConnected();
    void driverDisconnected();

private:
    void onDriverDataAvailable(QString data);
    void onDriverStateChanged(radio::IDriver::State state);

private:
    void createDriver();
    void createTreeParameters(const QString& data);

    void connectToDriver();
    void disconnectFromDriver();

private:
    /** Удерживает DLL плагина загруженной на время жизни драйвера. */
    boost::dll::shared_library m_pluginLibrary;
    std::shared_ptr<radio::IDriver> m_driver;
    ParameterTreeJsonParser* m_treeJsonParser;
    bool m_isConnected;
};

#endif // DRIVERADAPTER_H
