/*
 * Copyright 2020-2021 Martin Hoeher <martin@rpdev.net>
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

#ifndef SYNQCLIENT_ABSTRACTJOB_H
#define SYNQCLIENT_ABSTRACTJOB_H

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>

#include "libsynqclient.h"
#include "libsynqclient_global.h"

class QNetworkReply;

namespace SynqClient {

class AbstractJobPrivate;

class LIBSYNQCLIENT_EXPORT AbstractJob : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int transferTimeout READ transferTimeout WRITE setTransferTimeout NOTIFY
                       transferTimeoutChanged)
public:
    explicit AbstractJob(QObject* parent = nullptr);
    ~AbstractJob() override;

    virtual void start() = 0;
    virtual void stop() = 0;

    JobError error() const;
    QString errorString() const;
    JobState state() const;
    int transferTimeout() const;
    void setTransferTimeout(int transferTimeout);

signals:

    void finished();
    void transferTimeoutChanged();

protected:
    explicit AbstractJob(AbstractJobPrivate* d, QObject* parent = nullptr);

    QScopedPointer<AbstractJobPrivate> d_ptr;
    Q_DECLARE_PRIVATE(AbstractJob);

    void setError(JobError error, const QString& errorString);
    void setState(JobState state);
    void finishLater();

    static JobError fromNetworkError(const QNetworkReply& reply);
};

} // namespace SynqClient

#endif // SYNQCLIENT_ABSTRACTJOB_H
