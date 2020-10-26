#include "../inc/abstractwebdavjob.h"

#include "abstractwebdavjobprivate.h"

namespace SynqClient {

AbstractWebDAVJob::AbstractWebDAVJob() : d_ptr2(new AbstractWebDAVJobPrivate(this)) {}

AbstractWebDAVJob::~AbstractWebDAVJob() {}

QNetworkAccessManager* AbstractWebDAVJob::networkAccessManager() const
{
    Q_D(const AbstractWebDAVJob);
    return d->networkAccessManager;
}

void AbstractWebDAVJob::setNetworkAccessManager(QNetworkAccessManager* networkAccessManager)
{
    Q_D(AbstractWebDAVJob);
    d->networkAccessManager = networkAccessManager;
}

QUrl AbstractWebDAVJob::url() const
{
    Q_D(const AbstractWebDAVJob);
    return d->url;
}

void AbstractWebDAVJob::setUrl(const QUrl& url)
{
    Q_D(AbstractWebDAVJob);
    d->url = url;
}

WebDAVServerType AbstractWebDAVJob::serverType() const
{
    Q_D(const AbstractWebDAVJob);
    return d->serverType;
}

void AbstractWebDAVJob::setServerType(WebDAVServerType serverType)
{
    Q_D(AbstractWebDAVJob);
    d->serverType = serverType;
}

QString AbstractWebDAVJob::userAgent() const
{
    Q_D(const AbstractWebDAVJob);
    return d->userAgent;
}

void AbstractWebDAVJob::setUserAgent(const QString& userAgent)
{
    Q_D(AbstractWebDAVJob);
    d->userAgent = userAgent;
}

AbstractWebDAVJob::AbstractWebDAVJob(AbstractWebDAVJobPrivate* d) : d_ptr2(d) {}

} // namespace SynqClient
