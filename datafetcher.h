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

#ifndef DATAFETCHER_H
#define DATAFETCHER_H

#include <QObject>
#include <QVariant>

class DataFetcher : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QVariantList byProperties READ byProperties NOTIFY sourceChanged)
    Q_PROPERTY(QVariantList labels READ labels NOTIFY sourceChanged)
    Q_PROPERTY(QVariantMap checkedMap READ checkedMap NOTIFY checkedMapChanged)
    Q_PROPERTY(QStringList properties READ properties WRITE setProperties NOTIFY propertiesChanged)
    Q_PROPERTY(QVariantList mergables READ mergables WRITE setMergables NOTIFY mergablesChanged)
    class Private;

public:
    DataFetcher(QObject *parent = Q_NULLPTR);
    virtual ~DataFetcher();

    QString source() const;
    void setSource(const QString &source);

    QStringList properties() const;
    void setProperties(const QStringList &properties);

    QVariantList mergables() const;
    void setMergables(const QVariantList &mergables);

    QVariantList byProperties();
    QVariantList labels();

    QVariantMap checkedMap() const;

public Q_SLOTS:
    QVariantMap check(const QString &path);

Q_SIGNALS:
    void sourceChanged();
    void mergablesChanged();
    void propertiesChanged();
    void checkedMapChanged();

private:
    void load();
    void calculateProperties();
    void calculateFunctions();

private:
    Private *p;
};

#endif // DATAFETCHER_H
