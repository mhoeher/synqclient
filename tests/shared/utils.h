#ifndef SYNQCLIENT_UT_UTILS_H_
#define SYNQCLIENT_UT_UTILS_H_

#include <QString>
#include <QTest>
#include <QUrl>
#include <QtGlobal>

#include "AbstractWebDAVJob"

namespace SynqClient {
namespace UnitTest {

QList<QUrl> getWebDAVServersFromEnv()
{
    QByteArray env = qgetenv("SYNQCLIENT_UT_WEBDAV_SERVERS");
    QList<QUrl> result;
    if (!env.isEmpty()) {
        auto list = env.split(';');
        for (const auto& entry : list) {
            QUrl url(entry);
            if (url.isValid()) {
                result << url;
            }
        }
    }
    return result;
}

void setupWebDAVTestServerData()
{
    auto urls = getWebDAVServersFromEnv();
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<SynqClient::WebDAVServerType>("type");

    for (const auto& url : urls) {
        auto proto = url.scheme();
        if (proto == "nextcloud") {
            auto fixedUrl = url;
            fixedUrl.setScheme("http");
            QTest::newRow(url.toString().toUtf8())
                    << fixedUrl << SynqClient::WebDAVServerType::NextCloud;
        } else if (proto == "owncloud") {
            auto fixedUrl = url;
            fixedUrl.setScheme("http");
            QTest::newRow(url.toString().toUtf8())
                    << fixedUrl << SynqClient::WebDAVServerType::OwnCloud;
        } else if (proto == "generic") {
            auto fixedUrl = url;
            fixedUrl.setScheme("http");
            QTest::newRow(url.toString().toUtf8())
                    << fixedUrl << SynqClient::WebDAVServerType::Generic;
        } else {
            QTest::newRow(url.toString().toUtf8()) << url << SynqClient::WebDAVServerType::Generic;
        }
    }
}

}
}

#endif // SYNQCLIENT_UT_UTILS_H_
