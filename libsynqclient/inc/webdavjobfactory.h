/*
 * Copyright 2020 Martin Hoeher <martin@rpdev.net>
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

#ifndef SYNQCLIENT_WEBDAVJOBFACTORY_H
#define SYNQCLIENT_WEBDAVJOBFACTORY_H

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QUrl>
#include <QtGlobal>

#include "AbstractJobFactory"
#include "libsynqclient.h"
#include "libsynqclient_global.h"

class QNetworkAccessManager;

namespace SynqClient {

class WebDAVJobFactoryPrivate;

class LIBSYNQCLIENT_EXPORT WebDAVJobFactory : public AbstractJobFactory
{
    Q_OBJECT
public:
    explicit WebDAVJobFactory(QObject* parent = nullptr);
    ~WebDAVJobFactory() override;

    QNetworkAccessManager* networkAccessManager() const;
    void setNetworkAccessManager(QNetworkAccessManager* networkAccessManager);

    QUrl url() const;
    void setUrl(const QUrl& url);

    QString userAgent() const;
    void setUserAgent(const QString& userAgent);

    WebDAVServerType serverType() const;
    void setServerType(WebDAVServerType serverType);

protected:
    explicit WebDAVJobFactory(WebDAVJobFactoryPrivate* d, QObject* parent = nullptr);

    Q_DECLARE_PRIVATE(WebDAVJobFactory);

    // AbstractJobFactory interface
    AbstractJob* createJob(JobType type, QObject* parent) override;
};

} // namespace SynqClient

#endif // SYNQCLIENT_WEBDAVJOBFACTORY_H
