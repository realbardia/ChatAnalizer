/*
    Copyright (C) 2017 Aseman Team
    http://aseman.co

    AsemanQtTools is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AsemanQtTools is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ASEMANTOOLS_H
#define ASEMANTOOLS_H

#include <QObject>
#include <QWindow>
#include <QDir>
#include <QJSValue>

class AsemanToolsPrivate;
class AsemanTools : public QObject
{
    Q_OBJECT
public:
    AsemanTools(QObject *parent = Q_NULLPTR);
    virtual ~AsemanTools();

    enum DesktopSession {
        Unknown,
        Gnome,
        GnomeFallBack,
        Unity,
        Kde,
        Plasma,
        Windows,
        Mac
    };

    static int desktopSession();

public Q_SLOTS:
    QString getOpenFileName(QWindow *window = Q_NULLPTR, const QString &title = QString(), const QString &filter = QString(), const QString & startPath = QDir::homePath() );
    QString getExistingDirectory(QWindow *window = Q_NULLPTR, const QString &title = QString(), const QString & startPath = QDir::homePath());

    QString fileParent( const QString & path );
    QString fileName( const QString & path );

    void jsDelayCall(int ms, const QJSValue &value);

protected:
    void timerEvent(QTimerEvent *e);

private:
    AsemanToolsPrivate *p;
};

#endif // ASEMANTOOLS_H
