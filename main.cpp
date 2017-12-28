/****************************************************************************
 main.cpp: program entry point
 author: Joe Petrus
 date: June 10th 2010
****************************************************************************/

#include <QApplication>
#include "AppWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Q_INIT_RESOURCE(AppResources);
    app.setWindowIcon(QIcon(":/icons/icon.icns"));

    AppConfig appConfig(NULL);
    AppWindow appWindow(appConfig);
    appWindow.show();

    return app.exec();
}
