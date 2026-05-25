#include "BoardStationApp.h"

#include "mainwindow.h"

#include <QLocale>
#include <QTranslator>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
	BoardStationApp app(argc, argv);

	// int id = QFontDatabase::addApplicationFont(":/Interface/Qml/MaterialSymbolsRounded-Regular.ttf");
	// if (id == -1)
	//     qWarning() << "Failed to load MaterialSymbolsRounded-Regular.ttf";
	// else
	//     qDebug() << "Loaded fonts:" << QFontDatabase::applicationFontFamilies(id);

	// QTranslator translator;
	// const QStringList uiLanguages = QLocale::system().uiLanguages();
	// for (const QString &locale : uiLanguages) {
	//     const QString baseName = "BoardStation_" + QLocale(locale).name();
	//     if (translator.load(":/i18n/" + baseName)) {
	//         app.installTranslator(&translator);
	//         break;
	//     }
	// }
	
	// QmlMainWindow w;
	// w.setApp(&app);
	// w.show();

	QTranslator translator;
	if (translator.load(":/BoardStation_ru_RU.qm"))
	{
		app.installTranslator(&translator);
	}
	else
	{
		qDebug() << "cannot load translation";
	}

	MainWindow mainWindow;
	mainWindow.setApp(&app);
	mainWindow.showMaximized();
	//mainWindow.expandLiveDataTelemetry();

	return app.exec();
}
