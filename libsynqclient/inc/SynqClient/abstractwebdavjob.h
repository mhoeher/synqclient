#ifndef SYNQCLIENT_ABSTRACTWEBDAVJOB_H
#define SYNQCLIENT_ABSTRACTWEBDAVJOB_H

#include <QNetworkAccessManager>
#include <QUrl>
#include <QScopedPointer>

#include "AbstractJob"
#include "libsynqclient.h"
#include "libsynqclient_global.h"

namespace SynqClient {

class AbstractWebDAVJobPrivate;

class LIBSYNQCLIENT_EXPORT AbstractWebDAVJob
{
public:
    AbstractWebDAVJob();
    AbstractWebDAVJob(const AbstractWebDAVJob& other) = delete;
    ~AbstractWebDAVJob();
    AbstractWebDAVJob& operator=(AbstractWebDAVJob& other) = delete;

    QNetworkAccessManager* networkAccessManager() const;
    void setNetworkAccessManager(QNetworkAccessManager* networkAccessManager);

    QUrl url() const;
    void setUrl(const QUrl& url);

    WebDAVServerType serverType() const;
    void setServerType(WebDAVServerType serverType);

    WebDAVWorkarounds workarounds() const;
    void setWorkarounds(WebDAVWorkarounds workarounds);

    QString userAgent() const;
    void setUserAgent(const QString& userAgent);

protected:
    explicit AbstractWebDAVJob(AbstractWebDAVJobPrivate* d);

    QScopedPointer<AbstractWebDAVJobPrivate> d_ptr2;
    Q_DECLARE_PRIVATE_D(d_ptr2, AbstractWebDAVJob);
};

} // namespace SynqClient

#endif // SYNQCLIENT_ABSTRACTWEBDAVJOB_H
