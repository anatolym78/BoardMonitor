#include <iostream>
#include <optional>

#include <boost/dll/import.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/system/system_error.hpp>

#include <QCoreApplication>

#include "builder.hh"
#include "driver.hh"

class EventPrinter : public QObject {
    Q_OBJECT

public slots:
    void onDataAvailable(const QString& data) {
        std::cout << "[SIGNAL] dataAvailable: length=" << data.length()
                  << ", preview: " << data.left(50).toStdString()
                  << (data.length() > 50 ? "..." : "") << std::endl;
    }

    void onStateChanged(radio::IDriver::State state) {
        std::cout << "[SIGNAL] stateChanged: "
                  << (state == radio::IDriver::State::kConnected ? "Connected" : "Disconnected")
                  << std::endl;
    }
};

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    boost::filesystem::path plugin_path =
        boost::dll::program_location().parent_path() / "RCImitator_plugin";

    using BuilderFactory = decltype(boost::dll::import_alias<radio::IDriverBuilder*()>(
        plugin_path, "dllCreateBuilder", boost::dll::load_mode::append_decorations
    ));
    std::optional<BuilderFactory> plugin_loader;

    std::unique_ptr<radio::IDriverBuilder> builder;
    try {
        plugin_loader.emplace(boost::dll::import_alias<radio::IDriverBuilder*()>(
            plugin_path, "dllCreateBuilder",
            boost::dll::load_mode::append_decorations
        ));
        builder.reset((*plugin_loader)());
    } catch (const boost::system::system_error& ex) {
        std::cerr << "Failed to load plugin from: " << plugin_path.string()
                  << std::endl;
        std::cerr << "Reason: " << ex.what() << std::endl;
        return 1;
    }

    auto driver = builder->setDelay(1).enableAck(false).enableShutdown(true).get();

    std::cout << "The driver object has been constructed" << std::endl;

    EventPrinter printer;
    QObject::connect(driver.get(), &radio::IDriver::dataAvailable,
                     &printer, &EventPrinter::onDataAvailable);
    QObject::connect(driver.get(), &radio::IDriver::stateChanged,
                     &printer, &EventPrinter::onStateChanged);

    driver->start();
    return app.exec();
}

#include "main.moc"
