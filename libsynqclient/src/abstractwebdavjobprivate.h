#ifndef SYNQCLIENT_ABSTRACTWEBDAVJOBPRIVATE_H
#define SYNQCLIENT_ABSTRACTWEBDAVJOBPRIVATE_H

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QVariantList>

#include "abstractjobprivate.h"
#include "abstractwebdavjob.h"
#include "fileinfo.h"

class QDomDocument;
class QDomElement;

namespace SynqClient {

class AbstractWebDAVJobPrivate
{
public:
    static const char* DefaultEncoding;
    static const char* OctetStreamEncoding;
    static const char* PROPFIND;
    static const char* MKCOL;
    static const int HTTPOkay = 200;
    static const int HTTPCreated = 201;
    static const int HTTPNoContent = 204;
    static const int HTTPForbidden = 403;
    static const int HTTPPreconditionFailed = 412;
    static const int WebDAVMultiStatus = 207;
    static const int WebDAVCreated = 201;

    static const char* DefaultUserAgent;

    static const QByteArray PropFindRequestData;

    explicit AbstractWebDAVJobPrivate(AbstractWebDAVJob* q);
    virtual ~AbstractWebDAVJobPrivate();

    AbstractWebDAVJob* q_ptr;
    Q_DECLARE_PUBLIC(AbstractWebDAVJob);

    QString userAgent;
    QNetworkAccessManager* networkAccessManager;
    QUrl url;
    WebDAVServerType serverType;
    int numManualRedirects;
    QUrl nextUrl;
    QNetworkReply* reply;

    const int MaxRedirects = 30;

    QUrl urlFromPath(const QString& path);
    void prepareNetworkRequest(QNetworkRequest& request);
    bool shouldFollowUnhandledRedirect();
    FileInfos parseEntryList(const QUrl& url, const QByteArray& reply, bool& ok);

private:
    FileInfos parsePropFindResponse(const QUrl& baseUrl, const QDomDocument& response, bool& ok);
    FileInfo parseResponseEntry(const QUrl& url, const QDomElement& element, const QString& baseDir,
                                bool& ok);
};

} // namespace SynqClient

#endif // SYNQCLIENT_ABSTRACTWEBDAVJOBPRIVATE_H
