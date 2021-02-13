#ifndef SYNQCLIENT_UT_UTILS_H_
#define SYNQCLIENT_UT_UTILS_H_

#include <tuple>

#include <QString>
#include <QTest>
#include <QUrl>
#include <QtGlobal>

#include "AbstractWebDAVJob"

/**
 * @brief Utility macro to verify statements.
 *
 * This macro is supposed to be used in helper functions called in unit tests. These helper
 * functions are supposed to return a boolean indicating success.
 */
#define SQ_VERIFY(statement)                                                                       \
    do {                                                                                           \
        if (!QTest::qVerify(static_cast<bool>(statement), #statement, "", __FILE__, __LINE__)) {   \
            return false;                                                                          \
        }                                                                                          \
        break;                                                                                     \
    } while (false)

/**
 * @brief Utility macro to compare values.
 *
 * Similarly, this is used to compare values inside helper functions called in a unit test.
 */
#define SQ_COMPARE(actual, expected)                                                               \
    do {                                                                                           \
        if (!QTest::qCompare(actual, expected, #actual, #expected, __FILE__, __LINE__)) {          \
            return false;                                                                          \
        }                                                                                          \
        break;                                                                                     \
    } while (false)

namespace SynqClient {
namespace UnitTest {

inline QList<QUrl> getWebDAVServersFromEnv()
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

inline bool hasWebDAVServersFromEnv()
{
    return !getWebDAVServersFromEnv().isEmpty();
}

inline QVector<std::tuple<QUrl, SynqClient::WebDAVServerType>> enumerateWebDAVTestServers()
{
    auto urls = getWebDAVServersFromEnv();
    decltype(enumerateWebDAVTestServers()) result;

    for (const auto& url : urls) {
        auto proto = url.scheme();
        if (proto == "nextcloud") {
            auto fixedUrl = url;
            fixedUrl.setScheme("http");
            result << std::make_tuple(fixedUrl, SynqClient::WebDAVServerType::NextCloud);
        } else if (proto == "owncloud") {
            auto fixedUrl = url;
            fixedUrl.setScheme("http");
            result << std::make_tuple(fixedUrl, SynqClient::WebDAVServerType::OwnCloud);
        } else if (proto == "generic") {
            auto fixedUrl = url;
            fixedUrl.setScheme("http");
            result << std::make_tuple(fixedUrl, SynqClient::WebDAVServerType::Generic);
        } else {
            result << std::make_tuple(url, SynqClient::WebDAVServerType::Generic);
        }
    }
    return result;
}

inline void setupWebDAVTestServerData()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<SynqClient::WebDAVServerType>("type");

    for (const auto& tuple : enumerateWebDAVTestServers()) {
        auto url = std::get<0>(tuple);
        auto type = std::get<1>(tuple);
        QTest::newRow(url.toString().toUtf8()) << url << type;
    }
}

}
}

#endif // SYNQCLIENT_UT_UTILS_H_
