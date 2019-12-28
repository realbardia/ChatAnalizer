/*
    Copyright (C) 2019 Aseman Team
    http://aseman.io

    This project is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This project is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QDateTime>
#include <QApplication>
#include <QQmlEngine>
#include <QJSEngine>
#include <QQmlApplicationEngine>

#include "asemantools.h"
#include "datafetcher.h"

int main(int argc, char *argv[])
{
    qsrand(1601353213);
    qmlRegisterType<DataFetcher>("TgAnalizer", 1, 0, "DataFetcher");
    qmlRegisterSingletonType<AsemanTools>("TgAnalizer", 1, 0, "Tools", [](QQmlEngine *, QJSEngine *) -> QObject * {
        return new AsemanTools();
    });

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication app(argc, argv);
    app.setApplicationName("TgAnalizer");
    app.setOrganizationName("Aseman");

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
