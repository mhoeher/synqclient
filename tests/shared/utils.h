#ifndef SYNQCLIENT_UT_UTILS_H_
#define SYNQCLIENT_UT_UTILS_H_

#include <tuple>

#include <QString>
#include <QTest>
#include <QUrl>
#include <QtGlobal>

#include "SynqClient/AbstractWebDAVJob"

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

enum class WebDAVServerFlag { Empty = 0, NoIfMatch = 1 << 0, NoEtagOnDownload = 1 << 2 };

static const QMap<QByteArray, WebDAVServerFlag> WebDavServerFlagsMap = {
    { "NoIfMatch", WebDAVServerFlag::NoIfMatch },
    { "NoEtagOnDownload", WebDAVServerFlag::NoEtagOnDownload },
};

inline QList<QPair<QUrl, int>> getWebDAVServersFromEnv()
{
    QByteArray env = qgetenv("SYNQCLIENT_UT_WEBDAV_SERVERS");
    decltype(getWebDAVServersFromEnv()) result;
    if (!env.isEmpty()) {
        auto list = env.split(';');
        for (const auto& entry : list) {
            auto parts = entry.split('|');
            QUrl url(parts[0]);
            if (url.isValid()) {
                int flags = static_cast<int>(WebDAVServerFlag::Empty);
                for (int i = 1; i < parts.length(); ++i) {
                    auto flagName = parts[i];
                    flags |= static_cast<int>(
                            WebDavServerFlagsMap.value(flagName, WebDAVServerFlag::Empty));
                }
                result.append({ url, flags });
            }
        }
    }
    return result;
}

inline bool hasWebDAVServersFromEnv()
{
    return !getWebDAVServersFromEnv().isEmpty();
}

inline QByteArray getDropboxTokenFromEnv()
{
    return qgetenv("SYNQCLIENT_UT_DROPBOX_TOKEN");
}

inline bool hasDropboxTokenFromEnv()
{
    return !getDropboxTokenFromEnv().isEmpty();
}

inline QVector<std::tuple<QUrl, SynqClient::WebDAVServerType, int>> enumerateWebDAVTestServers()
{
    auto urls = getWebDAVServersFromEnv();
    decltype(enumerateWebDAVTestServers()) result;

    for (const auto& entry : urls) {
        auto url = entry.first;
        auto flags = entry.second;
        auto proto = url.scheme();
        if (proto == "nextcloud") {
            auto fixedUrl = url;
            fixedUrl.setScheme("http");
            result << std::make_tuple(fixedUrl, SynqClient::WebDAVServerType::NextCloud, flags);
        } else if (proto == "owncloud") {
            auto fixedUrl = url;
            fixedUrl.setScheme("http");
            result << std::make_tuple(fixedUrl, SynqClient::WebDAVServerType::OwnCloud, flags);
        } else if (proto == "generic") {
            auto fixedUrl = url;
            fixedUrl.setScheme("http");
            result << std::make_tuple(fixedUrl, SynqClient::WebDAVServerType::Generic, flags);
        } else {
            result << std::make_tuple(url, SynqClient::WebDAVServerType::Generic, flags);
        }
    }
    return result;
}

inline void setupWebDAVTestServerData()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<SynqClient::WebDAVServerType>("type");
    QTest::addColumn<int>("flags");

    for (const auto& tuple : enumerateWebDAVTestServers()) {
        auto url = std::get<0>(tuple);
        auto type = std::get<1>(tuple);
        auto flags = std::get<2>(tuple);
        QTest::newRow(url.toString().toUtf8()) << url << type << flags;
    }
}

}
}

#endif // SYNQCLIENT_UT_UTILS_H_
