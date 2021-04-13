/*
 * Copyright 2021 Martin Hoeher <martin@rpdev.net>
 *
 * This file is part of SynqClient.
 *
 * SynqClient is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * SynqClient is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SynqClient.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SYNQCLIENT_DROPBOXJOBFACTORY_H
#define SYNQCLIENT_DROPBOXJOBFACTORY_H

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>
#include <QNetworkAccessManager>
#include <QString>

#include "SynqClient/AbstractJobFactory"
#include "SynqClient/libsynqclient_global.h"

namespace SynqClient {

class DropboxJobFactoryPrivate;

class LIBSYNQCLIENT_EXPORT DropboxJobFactory : public AbstractJobFactory
{
    Q_OBJECT
public:
    explicit DropboxJobFactory(QObject* parent = nullptr);
    ~DropboxJobFactory() override;

    QNetworkAccessManager* networkAccessManager() const;
    void setNetworkAccessManager(QNetworkAccessManager* networkAccessManager);

    QString userAgent() const;
    void setUserAgent(const QString& userAgent);

    QString token() const;
    void setToken(const QString& token);

protected:
    explicit DropboxJobFactory(DropboxJobFactoryPrivate* d, QObject* parent = nullptr);

    Q_DECLARE_PRIVATE(DropboxJobFactory);

    // AbstractJobFactory interface
    AbstractJob* createJob(JobType type, QObject* parent) override;
};

} // namespace SynqClient

#endif // SYNQCLIENT_DROPBOXJOBFACTORY_H
