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

#ifndef SYNQCLIENT_ABSTRACTDROPBOXJOB_H
#define SYNQCLIENT_ABSTRACTDROPBOXJOB_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>

#include "SynqClient/libsynqclient_global.h"

namespace SynqClient {

class AbstractDropboxJobPrivate;

class LIBSYNQCLIENT_EXPORT AbstractDropboxJob
{
public:
    static const QString DropboxFileInfoKey;

    AbstractDropboxJob();
    virtual ~AbstractDropboxJob();

    QNetworkAccessManager* networkAccessManager() const;
    void setNetworkAccessManager(QNetworkAccessManager* networkAccessManager);

    QString userAgent() const;
    void setUserAgent(const QString& userAgent);

    QString token() const;
    void setToken(const QString& token);

protected:
    explicit AbstractDropboxJob(AbstractDropboxJobPrivate* d);

    QScopedPointer<AbstractDropboxJobPrivate> d_ptr2;
    Q_DECLARE_PRIVATE_D(d_ptr2, AbstractDropboxJob);
};

} // namespace SynqClient

#endif // SYNQCLIENT_ABSTRACTDROPBOXJOB_H
