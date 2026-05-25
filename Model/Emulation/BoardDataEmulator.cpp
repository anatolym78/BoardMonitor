#include "BoardDataEmulator.h"
#include "BoardDataGenerators/ThrottleAdjustmentValuesGenerator.h"
#include "BoardDataGenerators/GroundSpeedGenerator.h"
#include "BoardDataGenerators/AccelerationGenerator.h"
#include "BoardDataGenerators/DesiredAngleRatesGenerator.h"
#include "BoardDataGenerators/PositionAltitudeGenerator.h"
#include "BoardDataGenerators/GpsLocationGenerator.h"
#include "BoardDataGenerators/BatteryVoltageGenerator.h"
#include "BoardDataGenerators/IsArmedGenerator.h"
#include "BoardDataGenerators/GyroAngleRatesGenerator.h"
#include "BoardDataGenerators/BoardDataJsonGenerator.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QApplication>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace drv 
{

    BoardDataEmulator::BoardDataEmulator(QObject *)
        : m_stateTimer(new QTimer(this))
        , m_dataTimer(new QTimer(this))
        , m_jsonWriter(new BoardDataJsonGenerator(this))
        , m_time(0.0)
        , m_currentState(State::k_Disconnected)
        , m_isRunning(false)
        , m_isListening(true) // По умолчанию слушаем
    {
        // Настройка таймера состояния (каждые 30 секунд)
        m_stateTimer->setInterval(30000);
        connect(m_stateTimer, &QTimer::timeout, this, &BoardDataEmulator::onStateTimerTimeout);
    
        // Настройка таймера данных (30 раз в секунду)
        m_dataTimer->setInterval(1000 / 30.0);
        m_dataTimer->setTimerType(Qt::PreciseTimer);
        connect(m_dataTimer, &QTimer::timeout, this, &BoardDataEmulator::onDataTimerTimeout);
    
        // Настраиваем генераторы параметров
        setupGenerators();
    
        /// Автоматически запускаем эмулятор
        start();
    }

    BoardDataEmulator::~BoardDataEmulator()
    {
        stop();
    
        // Очищаем генераторы
        qDeleteAll(m_generators);
        m_generators.clear();
    }

    bool BoardDataEmulator::write(const std::string &data)
    {
        // Добавляем входящее сообщение в очередь
        m_messageQueue.enqueue(data);
    
        // Сохраняем отправленные параметры в файл
        QString jsonString = QString::fromStdString(data);
        saveSentParametersToFile(jsonString);
    
        // Эмитируем сигнал с отправленными данными
        emitDataSent(jsonString);

        return true;
    }

    const std::string &BoardDataEmulator::read()
    {
        static const std::string emptyString;
    
        if (m_messageQueue.isEmpty()) {
            return emptyString;
        }
    
        // Извлекаем последнее пришедшее сообщение
        return m_messageQueue.last();
    }

    bool BoardDataEmulator::start()
    {        
        if (m_isRunning)
        {
            return false;
        }
    
        m_isRunning = true;
        m_time = 0.0;
    
        // Запускаем таймеры
        //m_stateTimer->start();
        m_dataTimer->start();
    
        // Устанавливаем начальное состояние
        m_currentState = State::k_Connected;
        emitStateChanged(m_currentState);

        m_isListening = true;

        return true;

        //qDebug() << "BoardDataEmulator started";
    }

    void BoardDataEmulator::stop()
    {
        if (!m_isRunning) {
            return;
        }
    
        m_isRunning = false;
    
        // Останавливаем таймеры
        //m_stateTimer->stop();
        m_dataTimer->stop();

        m_isListening = false;
    
        //qDebug() << "BoardDataEmulator stopped";
    }

    void BoardDataEmulator::onStateTimerTimeout()
    {
        if (!m_isRunning) {
            return;
        }
    
        // Переключаем состояние между connected и disconnected
        m_currentState = (m_currentState == State::k_Connected) ? 
                            State::k_Disconnected : State::k_Connected;
    
        emitStateChanged(m_currentState);
    
        //qDebug() << "BoardDataEmulator state changed to:" << (m_currentState == State::k_Connected ? "Connected" : "Disconnected");
    }

    void BoardDataEmulator::onDataTimerTimeout()
    {
        if (!m_isRunning || m_currentState != State::k_Connected || !m_isListening) {
            return;
        }
    
        // Генерируем параметры
        generateParameters();
    
        // Увеличиваем время
        m_time += 1.0 / 30.0;
    }

    void BoardDataEmulator::setupGenerators()
    {
        m_generators.append(new ThrottleAdjustmentValuesGenerator(this));
        m_generators.append(new GroundSpeedGenerator(this));
        m_generators.append(new AccelerationGenerator(this));
        m_generators.append(new DesiredAngleRatesGenerator(this));
        m_generators.append(new PositionAltitudeGenerator(this));
        m_generators.append(new GpsLocationGenerator(this));
        m_generators.append(new BatteryVoltageGenerator(this));
        m_generators.append(new IsArmedGenerator(this));
        m_generators.append(new GyroAngleRatesGenerator(this));

        //qDebug() << "BoardDataEmulator: Настроены генераторы параметров:" << m_generators.size();
    }

    void BoardDataEmulator::generateParameters()
    {
        QList<BoardParameterSingle*> parameters;
    
        // Генерируем параметры от всех генераторов
        for (ParameterGenerator *generator : m_generators)
        {
            parameters.append(generator->generate(m_time));
        }
    
        if (parameters.isEmpty())
        {
            //qDebug() << "BoardDataEmulator: Не удалось сгенерировать параметры";
            qDeleteAll(parameters);
            return;
        }
    
        // Создаем JSON строку из параметров
        QString jsonString = m_jsonWriter->createJsonString(parameters);
    
        if (!jsonString.isEmpty()) 
        {
            // Добавляем JSON строку в очередь сообщений
            std::string jsonData = jsonString.toStdString();
            m_messageQueue.enqueue(jsonData);
        
            // Генерируем сигнал о доступности данных
            emitDataAvailable();
        
            //qDebug() << "BoardDataEmulator: Сгенерированы параметры, JSON:" << jsonString;
        }

        qDeleteAll(parameters);
    }

    void BoardDataEmulator::saveSentParametersToFile(const QString &jsonString)
    {
        QString filename = QApplication::applicationDirPath() + "/TestSendParameters.json";
    
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "BoardDataEmulator: Cannot open file for writing:" << filename;
            return;
        }
    
        // Парсим JSON и переформатируем его для читаемости
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &error);
    
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "BoardDataEmulator: JSON parse error:" << error.errorString();
            // Если не удалось распарсить, сохраняем как есть
            QTextStream stream(&file);
            stream << jsonString;
        } else {
            // Сохраняем в читаемом формате
            QByteArray formattedJson = doc.toJson(QJsonDocument::Indented);
            file.write(formattedJson);
        }
    
        file.close();
    
        //qDebug() << "BoardDataEmulator: Sent parameters saved to" << filename;
        //qDebug() << "BoardDataEmulator: JSON content:" << jsonString;
    }

} // namespace drv

