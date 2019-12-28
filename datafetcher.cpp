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

#define RESOLUTION 1000

#include "datafetcher.h"

#include <QFile>
#include <QList>
#include <QHash>
#include <QDir>
#include <QJsonDocument>
#include <QColor>
#include <QDebug>
#include <QtMath>

class DataFetcher::Private
{
public:
    class DataItem;
    class PropertyItem;
    class PropertyLabel;
    class PropertyValue;

    bool friendsMode = false;
    QString source;

    QVariantMap checkedMap;
    QStringList propertiesValue;
    QMap<QString, PropertyItem> properties;
    QHash<QString, DataItem> hash;
};

class DataFetcher::Private::DataItem
{
public:
    QVariantList list;
    QColor color;
    qint32 index = 0;
    QString label;
};

class DataFetcher::Private::PropertyItem
{
public:
    QMap<QString, PropertyLabel> labels;

    QString property;
    qreal maximum = INT_MIN;
    qreal minimum = INT_MAX;
    qreal sum = 0;

    qreal average() const { return labels.isEmpty()? 0 : sum / labels.count(); }
};

class DataFetcher::Private::PropertyLabel
{
public:
    QMap<qint32, qreal> function;
    QList<PropertyValue> values;
    QString label;
    qint32 labelIndex;
    QColor color;

    qreal checkRate(const PropertyItem &pItem, qreal value);
    qreal calculateRate_1(const PropertyItem &pItem, qreal value);
    qreal calculateRate_2(const PropertyItem &pItem, qreal value);
};

class DataFetcher::Private::PropertyValue
{
public:
    qreal value;
    QString json;
};

DataFetcher::DataFetcher(QObject *parent) :
    QObject(parent)
{
    p = new Private;
}

QString DataFetcher::source() const
{
    return p->source;
}

void DataFetcher::setSource(const QString &source)
{
    if (p->source == source)
        return;

    p->source = source;

    load();
    calculateProperties();
    calculateFunctions();

    Q_EMIT sourceChanged();
}

QStringList DataFetcher::properties() const
{
    return p->propertiesValue;
}

void DataFetcher::setProperties(const QStringList &properties)
{
    if (p->propertiesValue == properties)
        return;

    p->propertiesValue = properties;
    Q_EMIT propertiesChanged();
}

QVariantList DataFetcher::byProperties()
{
    QVariantList res;
    QMapIterator<QString, Private::PropertyItem> ip(p->properties);
    while (ip.hasNext())
    {
        ip.next();

        Private::PropertyItem pItem = ip.value();
        if (pItem.maximum == pItem.minimum)
            continue;

        QVariantList list;

        QMapIterator<QString, Private::PropertyLabel> il(pItem.labels);
        while (il.hasNext())
        {
            il.next();
            Private::PropertyLabel pLabel = il.value();

            for (const Private::PropertyValue &v: pLabel.values)
            {
                QVariantMap itemValues;
                itemValues["value"] = v.value;
                itemValues["color"] = pLabel.color;
                itemValues["labelIndex"] = pLabel.labelIndex;
                itemValues["label"] = pLabel.label;
                itemValues["json"] = v.json;

                list << itemValues;
            }
        }

        QVariantMap map;
        map["property"] = pItem.property;
        map["maximum"] = pItem.maximum;
        map["minimum"] = pItem.minimum;
        map["list"] = list;

        res << map;
    }

    return res;
}

QVariantList DataFetcher::labels()
{
    QVariantList res;
    QHashIterator<QString, DataFetcher::Private::DataItem> i(p->hash);
    while (i.hasNext())
    {
        i.next();
        DataFetcher::Private::DataItem item = i.value();

        QVariantMap map;
        map["label"] = item.label;
        map["color"] = item.color;
        map["labelIndex"] = item.index;

        res << map;
    }

    return res;
}

bool DataFetcher::friendsMode() const
{
    return p->friendsMode;
}

void DataFetcher::setFriendsMode(bool friendsMode)
{
    if (p->friendsMode == friendsMode)
        return;

    p->friendsMode = friendsMode;
    Q_EMIT friendsModeChanged();
}

QVariantMap DataFetcher::checkedMap() const
{
    return p->checkedMap;
}

QVariantMap DataFetcher::check(const QString &path)
{
    p->checkedMap.clear();

    QFile file(path);
    file.open(QFile::ReadOnly);

    QVariantList list = QJsonDocument::fromJson(file.readAll()).toVariant().toList();
    if (list.isEmpty())
    {
        Q_EMIT checkedMapChanged();
        return {};
    }

    QVariantMap map = list.first().toMap();
    QVariantMap months = map.value("months").toMap();
    if (months.isEmpty())
    {
        Q_EMIT checkedMapChanged();
        return {};
    }

    QHash<QString, qreal> globalRates;
    qreal globalRatesSum = 0;

    QString res;
    QMapIterator<QString, QVariant> mi(months);
    while (mi.hasNext())
    {
        mi.next();

        QHash<QString, qreal> rates;

        QVariantMap sum = mi.value().toMap().value("sum").toMap();
        QMapIterator<QString, QVariant> i(sum);
        while (i.hasNext())
        {
            i.next();
            QString property = i.key();
            if (p->propertiesValue.count() && !p->propertiesValue.contains(property))
                continue;

            bool ok = false;
            qreal value = i.value().toReal(&ok);
            if (!ok) continue;

            Private::PropertyItem &pItem = p->properties[property];
            if (pItem.maximum == pItem.minimum)
                continue;

            QMapIterator<QString, Private::PropertyLabel> il(pItem.labels);
            while (il.hasNext())
            {
                il.next();
                Private::PropertyLabel &pLabel = pItem.labels[il.key()];

                qreal rate = pLabel.checkRate(pItem, value);
                rates[pLabel.label] += rate;
                globalRates[pLabel.label] += rate;
            }

            QVariantMap monthMap;
            monthMap["month"] = mi.key();
            monthMap["value"] = sum.value(property).toReal();
            monthMap["minimum"] = pItem.minimum;
            monthMap["maximum"] = pItem.maximum;
            monthMap["property"] = property;

            QVariantList monthsList = p->checkedMap.value(property).toList();
            monthsList << monthMap;

            p->checkedMap[property] = monthsList;
        }

        qreal ratesSum = 0;
        QMap<qreal, QString> ratesMap;
        QHashIterator<QString, qreal> ir(rates);
        while (ir.hasNext())
        {
            ir.next();
            QString property = ir.key();
            qreal value = ir.value();

            ratesSum += value;
            globalRatesSum += value;
            ratesMap[value] = property;
        }

        res += mi.key() + ": ";

        QString valuesStr;
        QMapIterator<qreal, QString> ri(ratesMap);
        while (ri.hasNext())
        {
            ri.next();
            if (!valuesStr.isEmpty())
                valuesStr = ", " + valuesStr;

            valuesStr = ri.value() + " (" + QString::number(qFloor(ri.key()*1000/ratesSum)/10.0) + "%)" + valuesStr;
        }
        res += valuesStr + "\n";
    }

    if (p->friendsMode)
        globalRates["Friends"] = globalRates["Casual rel"] + globalRates["Close rel"];

    QMap<qreal, QString> globalRatesMap;
    QHashIterator<QString, qreal> ir(globalRates);
    while (ir.hasNext())
    {
        ir.next();
        QString property = ir.key();
        qreal value = ir.value();

        globalRatesMap[ir.value()] = property;
    }

    QString winner = globalRatesMap.last();
    if (winner == "Friends")
    {
        if (globalRates["Casual rel"] > globalRates["Close rel"])
            winner = "Friends (Casual rel)";
        else
            winner = "Friends (Close rel)";
    }

    QString percents;
    QMapIterator<qreal, QString> ri(globalRatesMap);
    while (ri.hasNext())
    {
        ri.next();
        if (ri.value() == "Friends")
            continue;

        if (!percents.isEmpty())
            percents = ", " + percents;

        percents = ri.value() + " (" + QString::number(qFloor(ri.key()*1000/globalRatesSum)/10.0) + "%)" + percents;
    }

    Q_EMIT checkedMapChanged();
    return { {"result", winner}, {"percents", percents}, {"string", res.trimmed()} };
}

void DataFetcher::load()
{
    QStringList files = QDir(p->source).entryList({"*.json"});
    qint32 labelIndex = 0;
    for (const QString &f: files)
    {
        QString path = p->source + "/" + f;
        QFile file(path);
        file.open(QFile::ReadOnly);

        QVariantList list = QJsonDocument::fromJson(file.readAll()).toVariant().toList();
        if (list.isEmpty())
            continue;

        QVariantMap map = list.first().toMap();
        QString label = map.value("label").toString();
//        label.remove("!");
        if (label.contains("!"))
            continue;
        if (!p->hash.contains(label))
        {
            DataFetcher::Private::DataItem item;
            item.color = QColor(qrand()%255, qrand()%255, qrand()%255);
            item.index = labelIndex++;
            item.label = label;

            p->hash[label] = item;
        }

        DataFetcher::Private::DataItem &item = p->hash[label];

        QVariantMap months = map.value("months").toMap();
        QMapIterator<QString, QVariant> im(months);
        while (im.hasNext())
        {
            im.next();
            QVariantMap month = im.value().toMap();
            QVariantMap sum = month.value("sum").toMap();
            if (sum.isEmpty())
                continue;

            item.list << sum;
        }
    }
}

void DataFetcher::calculateProperties()
{
    QVariantList res;

    p->properties.clear();
    QHashIterator<QString, DataFetcher::Private::DataItem> i(p->hash);
    while (i.hasNext())
    {
        i.next();

        QString label = i.key();
        DataFetcher::Private::DataItem item = i.value();

        for (const QVariant &l: item.list)
        {
            QVariantMap map = l.toMap();
            QMapIterator<QString, QVariant> ii(map);
            while (ii.hasNext())
            {
                ii.next();

                QString property = ii.key();
                QVariant value = ii.value();
                switch (static_cast<qint32>(value.type()))
                {
                case QVariant::Map:
                case QVariant::List:
                    continue;
                }

                if (!p->properties.contains(property))
                {
                    Private::PropertyItem _pItem;
                    _pItem.property = property;

                    p->properties[property] = _pItem;
                }

                Private::PropertyItem &pItem = p->properties[property];
                if (!pItem.labels.contains(label))
                {
                    Private::PropertyLabel _pLabel;
                    _pLabel.label = label;
                    _pLabel.color = item.color;
                    _pLabel.labelIndex = item.index;

                    pItem.labels[label] = _pLabel;
                }

                const qreal valueReal = value.toReal();

                Private::PropertyValue pValue;
                pValue.value = valueReal;
//                pValue.json = QJsonDocument::fromVariant(map).toJson();

                pItem.sum += valueReal;
                if (pItem.maximum < valueReal) pItem.maximum = valueReal;
                if (pItem.minimum > valueReal) pItem.minimum = valueReal;

                Private::PropertyLabel &pLabel = pItem.labels[label];
                pLabel.values << pValue;
            }
        }
    }
}

void DataFetcher::calculateFunctions()
{
    QVariantList res;
    QMapIterator<QString, Private::PropertyItem> ip(p->properties);
    while (ip.hasNext())
    {
        ip.next();

        Private::PropertyItem &pItem = p->properties[ip.key()];
        if (pItem.maximum == pItem.minimum)
            continue;

        QMapIterator<QString, Private::PropertyLabel> il(pItem.labels);
        while (il.hasNext())
        {
            il.next();
            Private::PropertyLabel &pLabel = pItem.labels[il.key()];

            for (const Private::PropertyValue &v: pLabel.values)
            {
                qreal normalValue = (v.value - pItem.minimum) / (pItem.maximum - pItem.minimum);
                qint32 index = normalValue * RESOLUTION;
                if (index == RESOLUTION) index--;

                pLabel.function[index] += (normalValue / pLabel.values.count());
            }
        }
    }
}

DataFetcher::~DataFetcher()
{
    delete p;
}


qreal DataFetcher::Private::PropertyLabel::checkRate(const PropertyItem &pItem, qreal value)
{
    return calculateRate_2(pItem, value);
}

qreal DataFetcher::Private::PropertyLabel::calculateRate_1(const PropertyItem &pItem, qreal value)
{
    qint32 index = ( (value - pItem.minimum) / (pItem.maximum - pItem.minimum) ) * RESOLUTION;
    if (function.contains(index))
        return function.value(index);

    qint32 beforeIndex = -1;
    qint32 afterIndex = RESOLUTION;

    for (qint32 i=0; i<RESOLUTION; i++)
    {
        if (!function.contains(i))
            continue;

        if (i < index)
            beforeIndex = i;
        if (i > index)
        {
            afterIndex = i;
            break;
        }
    }

    qreal before = (beforeIndex == -1? pItem.minimum / values.count() : function.value(beforeIndex));
    qreal after = (afterIndex == RESOLUTION? pItem.minimum / values.count() : function.value(afterIndex));

    qreal difVal = qAbs(after - before);
    qreal difIdx = (afterIndex - beforeIndex);
    qreal ratio = difVal / difIdx;
    qreal res = qMin(before, after) + (afterIndex - index) * ratio;

    return res;
}

qreal DataFetcher::Private::PropertyLabel::calculateRate_2(const PropertyItem &pItem, qreal value)
{
    qint32 index = ( (value - pItem.minimum) / (pItem.maximum - pItem.minimum) ) * RESOLUTION;

    qreal res = 0;
    for (qint32 i=0; i<RESOLUTION; i++)
    {
        qreal rate = function.value(i);
        res += rate / qPow(qAbs(i - index) + 1, 4);
    }

    return res;
}
