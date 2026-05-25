#ifndef BOARDDATAEMULATOR_H
#define BOARDDATAEMULATOR_H

//#include "../IDriver.h"
#include "./../../Services/RCDriver/driverinterface.h"
#include "BoardDataGenerators/ParameterGenerator.h"
#include "BoardDataGenerators/BoardDataJsonGenerator.h"
#include <QTimer>
#include <QQueue>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <cmath>

namespace drv 
{

    class BoardDataEmulator : public IDriver
    {
        Q_OBJECT

    public:
        explicit BoardDataEmulator(QObject *parent = nullptr);
        ~BoardDataEmulator() override;

        // IDriver interface implementation
        bool write(const std::string &data) override;
        const std::string &read() override;
    
        // Функции управления прослушиванием
        bool start() override;
        void stop() override;

        void addBaudrate(int m_baudrate) override {}
        virtual void addPort(const std::string& m_port) override {}

    private slots:
        void onStateTimerTimeout();
        void onDataTimerTimeout();

    private:
        void setupGenerators();
        void generateParameters();
        void saveSentParametersToFile(const QString &jsonString);

    private:
        QTimer *m_stateTimer;
        QTimer *m_dataTimer;
        QQueue<std::string> m_messageQueue;
    
        // Генераторы параметров
        QList<ParameterGenerator*> m_generators;
    
        // JSON writer для создания JSON строк
        BoardDataJsonGenerator *m_jsonWriter;
    
        // Время для генерации параметров
        double m_time;
    
        State m_currentState;
        bool m_isRunning;
        bool m_isListening;
    };

} // namespace drv

#endif // BOARDDATAEMULATOR_H
