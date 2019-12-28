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

#include "asemantools.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QJSValue>
#include <QHash>
#include <QProcess>
#include <QEventLoop>

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS) || defined(Q_OS_WINPHONE) || defined(Q_OS_UBUNTUTOUCH)
#define TOUCH_DEVICE
#else
#define DESKTOP_DEVICE
#if defined(Q_OS_LINUX) || defined(Q_OS_OPENBSD)
#define DESKTOP_LINUX
#endif
#endif

class AsemanToolsPrivate
{
public:
    QHash<int, QJSValue> js_delay_call_timers;
};

AsemanTools::AsemanTools(QObject *parent) :
    QObject(parent)
{
    p = new AsemanToolsPrivate;
}

QString AsemanTools::getOpenFileName(QWindow *window, const QString & title, const QString &filter, const QString &startPath)
{
#if defined(DESKTOP_DEVICE) && defined(QT_WIDGETS_LIB)
    const int dsession = desktopSession();
    switch( dsession )
    {
    case AsemanTools::Kde:
    case AsemanTools::Plasma:
        if( QFileInfo::exists(QStringLiteral("/usr/bin/kdialog")) )
        {
            QStringList args = QStringList()<< QStringLiteral("--title") << title << QStringLiteral("--getopenfilename")
                                            << startPath << filter;
            if( window )
                args << QStringLiteral("--attach") << QString::number(window->winId());

            QProcess process;
            QEventLoop loop;
            connect(&process, static_cast<void (QProcess::*)(int)>(&QProcess::finished), &loop, &QEventLoop::quit, Qt::QueuedConnection );

            process.start(QStringLiteral("/usr/bin/kdialog"), args );
            loop.exec(QEventLoop::ExcludeUserInputEvents);

            if( process.exitStatus() == QProcess::NormalExit )
                return QString::fromUtf8(process.readAll()).remove(QStringLiteral("\n"));
            else
                return QFileDialog::getOpenFileName(Q_NULLPTR, title, startPath, filter);
        }
        else
            return QFileDialog::getOpenFileName(Q_NULLPTR, title, startPath, filter);

    case AsemanTools::Unity:
    case AsemanTools::GnomeFallBack:
    case AsemanTools::Gnome:
        if( QFileInfo::exists(QStringLiteral("/usr/bin/zenity")) )
        {
            QStringList args = QStringList()<< QStringLiteral("--title=") << QStringLiteral("--file-selection") <<
                                               QStringLiteral("--class=Cutegram") << QStringLiteral("--name=Cutegram");
            if(!filter.isEmpty())
                args << QStringLiteral("--file-filter=") + filter;

            QProcess process;
            QEventLoop loop;
            connect(&process, static_cast<void (QProcess::*)(int)>(&QProcess::finished), &loop, &QEventLoop::quit, Qt::QueuedConnection );

            process.start(QStringLiteral("/usr/bin/zenity"), args );
            loop.exec(QEventLoop::ExcludeUserInputEvents);

            if( process.exitStatus() == QProcess::NormalExit )
                return QString::fromUtf8(process.readAll()).remove(QStringLiteral("\n"));
            else
                return QFileDialog::getOpenFileName(Q_NULLPTR, title, startPath, filter);
        }
        else
            return QFileDialog::getOpenFileName(Q_NULLPTR, title, startPath, filter);

    case AsemanTools::Mac:
    case AsemanTools::Windows:
        return QFileDialog::getOpenFileName(Q_NULLPTR, title, startPath, filter);
    }

    return QString();
#else
#if defined(QT_WIDGETS_LIB)
    return QFileDialog::getOpenFileName(Q_NULLPTR, title, startPath, filter);
#else
    Q_UNUSED(window)
    Q_UNUSED(title)
    Q_UNUSED(filter)
    Q_UNUSED(startPath)
    return QString();
#endif
#endif
}

QString AsemanTools::getExistingDirectory(QWindow *window, const QString &title, const QString &startPath)
{
#if defined(DESKTOP_DEVICE) && defined(QT_WIDGETS_LIB)
    const int dsession = desktopSession();
    switch( dsession )
    {
    case AsemanTools::Kde:
    case AsemanTools::Plasma:
        if( QFileInfo::exists(QStringLiteral("/usr/bin/kdialog")) )
        {
            QStringList args = QStringList()<< QStringLiteral("--title") << title << QStringLiteral("--getexistingdirectory")
                                            << startPath;
            if( window )
                args << QStringLiteral("--attach") << QString::number(window->winId());

            QProcess process;
            QEventLoop loop;
            connect(&process, static_cast<void (QProcess::*)(int)>(&QProcess::finished), &loop, &QEventLoop::quit, Qt::QueuedConnection );

            process.start(QStringLiteral("/usr/bin/kdialog"), args );
            loop.exec(QEventLoop::ExcludeUserInputEvents);

            if( process.exitStatus() == QProcess::NormalExit )
                return QString::fromUtf8(process.readAll()).remove(QStringLiteral("\n"));
            else
                return QFileDialog::getExistingDirectory(Q_NULLPTR, title, startPath);
        }
        else
        {
            return QFileDialog::getExistingDirectory(Q_NULLPTR, title, startPath);
        }

    case AsemanTools::Unity:
    case AsemanTools::GnomeFallBack:
    case AsemanTools::Gnome:
        if( QFileInfo::exists(QStringLiteral("/usr/bin/zenity")) )
        {
            QStringList args = QStringList()<< QStringLiteral("--title=") << QStringLiteral("--file-selection") << QStringLiteral("--directory") <<
                                               QStringLiteral("--class=Cutegram") << QStringLiteral("--name=Cutegram");

            QProcess process;
            QEventLoop loop;
            connect(&process, static_cast<void (QProcess::*)(int)>(&QProcess::finished), &loop, &QEventLoop::quit, Qt::QueuedConnection );

            process.start(QStringLiteral("/usr/bin/zenity"), args );
            loop.exec(QEventLoop::ExcludeUserInputEvents);

            if( process.exitStatus() == QProcess::NormalExit )
                return QString::fromUtf8(process.readAll()).remove(QStringLiteral("\n"));
            else
                return QFileDialog::getExistingDirectory(Q_NULLPTR, title, startPath);
        }
        else
            return QFileDialog::getExistingDirectory(Q_NULLPTR, title, startPath);

    case AsemanTools::Mac:
    case AsemanTools::Windows:
        return QFileDialog::getExistingDirectory(Q_NULLPTR, title, startPath);
    }

    return QString();
#else
#if defined(QT_WIDGETS_LIB)
    return QFileDialog::getExistingDirectory(Q_NULLPTR, title, startPath);
#else
    Q_UNUSED(window)
    Q_UNUSED(title)
    Q_UNUSED(startPath)
    return QString();
#endif
#endif
}

int AsemanTools::desktopSession()
{
    static int result = -1;
    if( result != -1 )
        return result;

#ifdef Q_OS_MAC
    result = AsemanTools::Mac;
#else
#ifdef Q_OS_WIN
    result = AsemanTools::Windows;
#else
    static QString *desktop_session = Q_NULLPTR;
    if( !desktop_session )
        desktop_session = new QString( QString::fromUtf8(qgetenv("DESKTOP_SESSION")) );

    if( desktop_session->contains(QStringLiteral("kde"),Qt::CaseInsensitive) )
        result = AsemanTools::Kde;
    else
    if( desktop_session->contains(QStringLiteral("plasma"),Qt::CaseInsensitive) )
        result = AsemanTools::Plasma;
    else
    if( desktop_session->contains(QStringLiteral("ubuntu"),Qt::CaseInsensitive) )
        result = AsemanTools::Unity;
    else
    if( desktop_session->contains(QStringLiteral("gnome-fallback"),Qt::CaseInsensitive) )
        result = AsemanTools::GnomeFallBack;
    else
    if( desktop_session->contains(QStringLiteral("gnome"),Qt::CaseInsensitive) )
        result = AsemanTools::Gnome;
#endif
#endif

    if( result == -1 )
        result = AsemanTools::Unknown;

    return result;
}

QString AsemanTools::fileParent(const QString &path)
{
    if(path.count() == 1 && path[0] == '/')
        return path;
#ifdef Q_OS_WIN
    if(path.count() == 3 && path[0].isLetter() && path[1] == ':')
        return path;
#endif
    QString res = path.mid(0, path.lastIndexOf(QStringLiteral("/")));
#ifndef Q_OS_WIN
    if(res.isEmpty())
        res += "/";
#endif
    return res;
}

QString AsemanTools::fileName(const QString &path)
{
    return QFileInfo(path).baseName();
}

void AsemanTools::jsDelayCall(int ms, const QJSValue &value)
{
    p->js_delay_call_timers[ startTimer(ms) ] = value;
}

void AsemanTools::timerEvent(QTimerEvent *e)
{
    if(p->js_delay_call_timers.contains(e->timerId()))
    {
        p->js_delay_call_timers.take(e->timerId()).call();
    }
    else
        QObject::timerEvent(e);
}

AsemanTools::~AsemanTools()
{
    delete p;
}
