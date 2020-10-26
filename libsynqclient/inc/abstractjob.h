#ifndef SYNQCLIENT_ABSTRACTJOB_H
#define SYNQCLIENT_ABSTRACTJOB_H

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>

#include "libsynqclient_global.h"

namespace SynqClient {

class AbstractJobPrivate;

enum class LIBSYNQCLIENT_EXPORT JobError : quint32 {
    NoError = 0, //!< Indicates that no error occurred.
    Stopped, //!< The job has been stopped by the user.
    MissingParameter, //!< Some parameters required to run the job are missing.
    InvalidParameter, //!< Some parameters have values which are invalid.
    InvalidResponse //!< Received an invalid response during an operation
};

enum class LIBSYNQCLIENT_EXPORT JobState : quint32 { Ready = 0, Running, Finished };

class LIBSYNQCLIENT_EXPORT ItemProperty
{
public:
    static const char* Valid;
    static const char* Type;
    static const char* SyncAttribute;
    static const char* Name;
    static const char* URL;
};

class LIBSYNQCLIENT_EXPORT ItemType
{
public:
    static const char* Directory;
    static const char* File;
    static const char* Invalid;
};

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
    AbstractJob(AbstractJobPrivate* d, QObject* parent = nullptr);

    QScopedPointer<AbstractJobPrivate> d_ptr;
    Q_DECLARE_PRIVATE(AbstractJob);

    void setError(JobError error, const QString& errorString);
    void setState(JobState state);
    void finishLater();
};

} // namespace SynqClient

#endif // SYNQCLIENT_ABSTRACTJOB_H
