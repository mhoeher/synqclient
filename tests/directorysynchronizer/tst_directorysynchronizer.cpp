#include <QFile>
#include <QMap>
#include <QRandomGenerator>
#include <QSharedPointer>
#include <QTextStream>
#include <QtTest>

// add necessary includes here
#include "AbstractJobFactory"
#include "DirectorySynchronizer"
#include "WebDAVJobFactory"
#include "../shared/utils.h"

using SynqClient::AbstractJobFactory;
using SynqClient::DirectorySynchronizer;
using SynqClient::WebDAVJobFactory;

class DirectorySynchronizerTest : public QObject
{
    Q_OBJECT

public:
    DirectorySynchronizerTest();
    ~DirectorySynchronizerTest();

private slots:
    void initTestCase();
    void sync();
    void sync_data();
    void cleanupTestCase();

private:
    void fillTestFolder(const QString& path);
    void createRandomFiles(const QString& path);
    QMap<QString, QByteArray> readDirectory(const QString& path);
};

DirectorySynchronizerTest::DirectorySynchronizerTest() {}

DirectorySynchronizerTest::~DirectorySynchronizerTest() {}

void DirectorySynchronizerTest::initTestCase() {}

void DirectorySynchronizerTest::sync()
{
    QFETCH(AbstractJobFactory*, jobFactory);

    QTemporaryDir tmpDir1;
    QTemporaryDir tmpDir2;

    //    auto uuid = QUuid::createUuid();
    //    auto path = "/DirectorySynchronizerTest-sync-" + uuid.toString();

    fillTestFolder(tmpDir1.path());
}

void DirectorySynchronizerTest::sync_data()
{
    QTest::addColumn<AbstractJobFactory*>("jobFactory");

    auto nam = new QNetworkAccessManager(this);

    for (const auto& tuple : SynqClient::UnitTest::enumerateWebDAVTestServers()) {
        auto url = std::get<0>(tuple);
        auto type = std::get<1>(tuple);
        auto webdavJobFactory = new WebDAVJobFactory(this);
        webdavJobFactory->setUrl(url);
        webdavJobFactory->setServerType(type);
        webdavJobFactory->setNetworkAccessManager(nam);
        QTest::newRow(url.toString().toUtf8())
                << static_cast<AbstractJobFactory*>(webdavJobFactory);
    }
}

void DirectorySynchronizerTest::cleanupTestCase() {}

void DirectorySynchronizerTest::fillTestFolder(const QString& path)
{
    // This fills the test folder with some sample data. We use the following structure (mimicking
    // what is used in the OpenTodoList app, from where the library has been split from): Each file
    // is stored in a folder structure <year>/<month>/ (where the path is determined by the date the
    // file is created on). OpenTodoList put files only on this nesting level, however, we will also
    // create files below the year folders. To test filtering, we create two types of files: Binary
    // ones and text files.
    QDir dir(path);
    createRandomFiles(path);
    for (int year = 2020; year <= 2030; ++year) {
        QVERIFY(dir.mkdir(QString::number(year)));
        QVERIFY(dir.cd(QString::number(year)));
        createRandomFiles(dir.path());
        for (int month = 1; month <= 12; ++month) {
            QVERIFY(dir.mkdir(QString::number(month)));
            QVERIFY(dir.cd(QString::number(month)));
            createRandomFiles(dir.path());
        }
        QVERIFY(dir.cdUp());
    }
}

void DirectorySynchronizerTest::createRandomFiles(const QString& path)
{
    auto prng = QRandomGenerator::global();
    auto n = 1 + prng->generate() % 10;
    for (unsigned int i = 0; i < n; ++i) {
        auto name = QUuid::createUuid().toString();
        bool text = prng->generate() % 2 == 0;
        if (text) {
            // Create a text file
            QFile file(path + "/" + name + ".txt");
            QVERIFY(file.open(QIODevice::WriteOnly));
            QTextStream stream(&file);
            auto wordCount = 10 + prng->generate() % 1000;
            while (wordCount > 0) {
                --wordCount;
                stream << prng->generate() << " ";
            }
            file.close();
        } else {
            // Create a binary file
            QFile file(path + "/" + name + ".dat");
            QVERIFY(file.open(QIODevice::WriteOnly));
            auto wordCount = 10 + prng->generate() % 1000;
            while (wordCount > 0) {
                --wordCount;
                auto word = prng->generate();
                file.write(reinterpret_cast<const char*>(&word), sizeof(word));
            }
            file.close();
        }
    }
}

QMap<QString, QByteArray> DirectorySynchronizerTest::readDirectory(const QString& path)
{
    decltype(readDirectory(".")) result;
    return result;
}

QTEST_MAIN(DirectorySynchronizerTest)

#include "tst_directorysynchronizer.moc"
