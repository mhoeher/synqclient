#include "abstractwebdavjobprivate.h"

#include <QDir>
#include <QDomDocument>
#include <QLoggingCategory>

namespace SynqClient {

static Q_LOGGING_CATEGORY(log, "SynqClient.AbstractWebDAVJob", QtWarningMsg);

const char* AbstractWebDAVJobPrivate::DefaultEncoding = "text/xml; charset=utf-8";
const char* AbstractWebDAVJobPrivate::OctetStreamEncoding = "application/octet-stream";
const char* AbstractWebDAVJobPrivate::PROPFIND = "PROPFIND";
const char* AbstractWebDAVJobPrivate::MKCOL = "MKCOL";

const char* AbstractWebDAVJobPrivate::DefaultUserAgent = "SynqClient";

const QByteArray AbstractWebDAVJobPrivate::PropFindRequestData = "<?xml version=\"1.0\"?>"
                                                                 "<a:propfind xmlns:a=\"DAV:\">"
                                                                 "<a:prop>"
                                                                 "<a:getetag/>"
                                                                 "<a:resourcetype/>"
                                                                 "</a:prop>"
                                                                 "</a:propfind>";

AbstractWebDAVJobPrivate::AbstractWebDAVJobPrivate(AbstractWebDAVJob* q)
    : q_ptr(q),
      userAgent(DefaultUserAgent),
      networkAccessManager(nullptr),
      url(),
      serverType(WebDAVServerType::Generic),
      numManualRedirects(0),
      nextUrl(),
      reply(nullptr)
{
}

AbstractWebDAVJobPrivate::~AbstractWebDAVJobPrivate()
{
    if (reply) {
        reply->deleteLater();
    }
}

QUrl AbstractWebDAVJobPrivate::urlFromPath(const QString& path)
{
    if (nextUrl.isValid()) {
        // We are here because of strange redirect handling for non-standard HTTP
        // verbs. Return the "next" URL, which we got as a response to our request
        // to the server:
        nextUrl.setUserName(url.userName());
        nextUrl.setPassword(url.password());
        return nextUrl;
    }
    QUrl result = url;
    auto basePath = result.path();
    if (basePath.isEmpty()) {
        basePath = "/";
    }
    switch (serverType) {
    case WebDAVServerType::NextCloud:
    case WebDAVServerType::OwnCloud:
        basePath += "/remote.php/webdav/";
        break;
    case WebDAVServerType::Generic:
        break;
    }
    basePath += "/" + path;
    QDir dir(basePath);
    result.setPath(dir.absolutePath());
    return result;
}

void AbstractWebDAVJobPrivate::prepareNetworkRequest(QNetworkRequest& request)
{
    request.setRawHeader("User-Agent", userAgent.toUtf8());
}

/**
 * @brief Check if the reply contains a redirect not handled by Qt.
 *
 * For details, refer to
 * - https://gitlab.com/rpdev/opentodolist/-/issues/339
 * - https://bugreports.qt.io/browse/QTBUG-84162
 *
 * Background: At least for "unusual" HTTP verbs Qt seems to handle redirects strangely,
 * so we check if the QNAM is set up to redirect and - if so - follow the redirect
 * if not already resolved by Qt.
 *
 * Returns true if we need to follow the redirect, false otherwise.
 */
bool AbstractWebDAVJobPrivate::shouldFollowUnhandledRedirect()
{
    if (reply && networkAccessManager
        && networkAccessManager->redirectPolicy() == QNetworkRequest::NoLessSafeRedirectPolicy) {
        auto redirectTargetAttr = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        if (redirectTargetAttr.isValid()) {
            nextUrl = redirectTargetAttr.toUrl();
            ++numManualRedirects;
            if (numManualRedirects <= MaxRedirects) {
                return true;
            }
        }
    }
    return false;
}

FileInfos AbstractWebDAVJobPrivate::parseEntryList(const QUrl& url, const QByteArray& reply,
                                                   bool& ok)
{
    FileInfos result;
    QDomDocument doc;
    QString errorMsg;
    int errorLine;
    ok = false;
    if (doc.setContent(reply, true, &errorMsg, &errorLine)) {
        ok = true;
        result = parsePropFindResponse(url, doc, ok);
    } else {
        qCWarning(log) << "Failed to parse WebDAV response:" << errorMsg << "in line" << errorLine;
    }
    return result;
}

FileInfos AbstractWebDAVJobPrivate::parsePropFindResponse(const QUrl& baseUrl,
                                                          const QDomDocument& response, bool& ok)
{
    FileInfos result;
    auto baseDir = QDir::cleanPath("/" + baseUrl.path());
    auto root = response.documentElement();
    auto rootTagName = root.tagName();
    if (rootTagName == "multistatus") {
        auto resp = root.firstChildElement("response");
        while (resp.isElement()) {
            auto entry = parseResponseEntry(baseUrl, resp, baseDir, ok);
            if (entry.isValid()) {
                result << entry;
            }
            resp = resp.nextSiblingElement("response");
        }
    } else {
        qCWarning(log) << "Received invalid WebDAV response from"
                          "server starting with element"
                       << rootTagName;
        ok = false;
    }
    return result;
}

FileInfo AbstractWebDAVJobPrivate::parseResponseEntry(const QUrl& url, const QDomElement& element,
                                                      const QString& baseDir, bool& ok)
{
    FileInfo result;
    result.setIsFile();

    auto propstats = element.elementsByTagName("propstat");
    for (int i = 0; i < propstats.length(); ++i) {
        auto propstat = propstats.at(i).toElement();
        auto status = propstat.firstChildElement("status");
        if (status.text().endsWith("200 OK")) {
            auto prop = propstat.firstChildElement("prop");
            auto child = prop.firstChildElement();
            while (child.isElement()) {
                if (child.tagName() == "resourcetype") {
                    if (child.firstChildElement().tagName() == "collection") {
                        result.setIsDirectory();
                    }
                } else if (child.tagName() == "getetag") {
                    result.setSyncAttribute(child.text());
                } else {
                    qCWarning(log) << "Unknown DAV Property:" << child.tagName();
                }
                child = child.nextSiblingElement();
            }
        } else {
            qCWarning(log) << "Properties not retrieved -" << status.text();
            ok = false;
        }
    }

    QString path =
            QByteArray::fromPercentEncoding(element.firstChildElement("href").text().toUtf8());
    auto itemUrl = url;
    itemUrl.setUserName(QString());
    itemUrl.setPassword(QString());
    itemUrl.setPath(path);
    result.setUrl(itemUrl);
    path = QDir(baseDir).relativeFilePath(path);
    result.setName(path);
    return result;
}

} // namespace SynqClient
