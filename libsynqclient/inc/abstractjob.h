#ifndef SYNQCLIENT_ABSTRACTJOB_H
#define SYNQCLIENT_ABSTRACTJOB_H

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>

#include "libsynqclient.h"
#include "libsynqclient_global.h"

namespace SynqClient {

class AbstractJobPrivate;

class LIBSYNQCLIENT_EXPORT AbstractJob : public QObject
{
    Q_OBJECT
public:
    explicit AbstractJob(QObject* parent = nullptr);
    ~AbstractJob() override;

    virtual void start() = 0;
    virtual void stop() = 0;

    JobError error() const;
    QString errorString() const;
    JobState state() const;

signals:

    void finished();

protected:
    explicit AbstractJob(AbstractJobPrivate* d, QObject* parent = nullptr);

    QScopedPointer<AbstractJobPrivate> d_ptr;
    Q_DECLARE_PRIVATE(AbstractJob);

    void setError(JobError error, const QString& errorString);
    void setState(JobState state);
    void finishLater();
};

} // namespace SynqClient

#endif // SYNQCLIENT_ABSTRACTJOB_H
