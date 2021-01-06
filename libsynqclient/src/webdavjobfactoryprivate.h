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

#ifndef SYNQCLIENT_WEBDAVJOBFACTORYPRIVATE_H
#define SYNQCLIENT_WEBDAVJOBFACTORYPRIVATE_H

#include "abstractjobfactoryprivate.h"
#include "webdavjobfactory.h"

#include <QNetworkAccessManager>
#include <QPointer>

namespace SynqClient {

class WebDAVJobFactoryPrivate : public AbstractJobFactoryPrivate
{
public:
    explicit WebDAVJobFactoryPrivate(WebDAVJobFactory* q);

    Q_DECLARE_PUBLIC(WebDAVJobFactory);

    QPointer<QNetworkAccessManager> networkAccessManager;
    QUrl url;
    QString userAgent;
    WebDAVServerType serverType;

    template<typename T>
    T* createJob(QObject* parent)
    {
        auto result = new T(parent);
        result->setNetworkAccessManager(networkAccessManager);
        result->setUrl(url);
        result->setUserAgent(userAgent);
        result->setServerType(serverType);
        return result;
    }
};

} // namespace SynqClient

#endif // SYNQCLIENT_WEBDAVJOBFACTORYPRIVATE_H