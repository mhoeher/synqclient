#include <QDirIterator>
#include <QFile>
#include <QMap>
#include <QRandomGenerator>
#include <QSharedPointer>
#include <QSignalSpy>
#include <QTextStream>
#include <QtTest>

// add necessary includes here
#include "../shared/utils.h"
#include "SynqClient/AbstractJobFactory"
#include "SynqClient/DirectorySynchronizer"
#include "SynqClient/DropboxJobFactory"
#include "SynqClient/FileInfo"
#include "SynqClient/JSONSyncStateDatabase"
#include "SynqClient/WebDAVJobFactory"
#include "SynqClient/CreateDirectoryJob"
#include "SynqClient/GetFileInfoJob"
#include "SynqClient/ListFilesJob"
#include "SynqClient/UploadFileJob"
#include "SynqClient/DownloadFileJob"

using SynqClient::AbstractJobFactory;
using SynqClient::DirectorySynchronizer;
using SynqClient::FileInfo;
using SynqClient::JSONSyncStateDatabase;
using SynqClient::SyncConflictStrategy;
using SynqClient::SynchronizerError;
using SynqClient::SynchronizerFlag;
using SynqClient::SynchronizerFlags;
using SynqClient::SynchronizerState;
using SynqClient::WebDAVJobFactory;

class DirectorySynchronizerTest : public QObject
{
    Q_OBJECT

public:
    DirectorySynchronizerTest();
    ~DirectorySynchronizerTest();

private slots:
    void initTestCase();

    void serverCharacteristics();
    void serverCharacteristics_data() { prepareTestData(); }

    // Test individual sync corner cases
    void failIfNotCreatingRemoteFolders();
    void failIfNotCreatingRemoteFolders_data() { prepareTestData(); }
    void simpleSyncAndConflictResolution();
    void simpleSyncAndConflictResolution_data() { prepareTestData(); }
    void editVsDeleteConflictResolution();
    void editVsDeleteConflictResolution_data() { prepareTestData(); }

    // More complex sync of larger directory
    void sync();
    void sync_data() { prepareTestData(); }
    void cleanupTestCase();

private:
    void prepareTestData();

    bool fillTestFolder(const QString& path);
    bool createRandomFiles(const QString& path);
    QMap<QString, QByteArray> readDirectory(const QString& path);
    bool editDirectory(const QString& path, QMap<QString, QByteArray> contents);
    template<SyncConflictStrategy strategy = SyncConflictStrategy::RemoteWins>
    bool syncDir(const QString& localPath, const QString& remotePath, const QString& syncDbPath,
                 AbstractJobFactory* jobFactory);

    bool writeFile(const QString& fileName, const QByteArray& data) const;
    QByteArray readFile(const QString& fileName) const;
};

DirectorySynchronizerTest::DirectorySynchronizerTest() {}

DirectorySynchronizerTest::~DirectorySynchronizerTest() {}

void DirectorySynchronizerTest::initTestCase() {}

void DirectorySynchronizerTest::serverCharacteristics()
{
    if (!SynqClient::UnitTest::hasWebDAVServersFromEnv()
        && !SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No servers configured - skipping test");
    }

    QFETCH(AbstractJobFactory*, jobFactory);

    QTemporaryDir tmpDir;
    QTemporaryDir metaTmpDir;

    auto uuid = QUuid::createUuid();
    auto path = "DirectorySynchronizerTest-serverCharacteristics-" + uuid.toString();

    {
        auto job = jobFactory->createDirectory();
        job->setPath(path);
        job->start();
        QSignalSpy spy(job, &SynqClient::AbstractJob::finished);
        QVERIFY(spy.wait());
        delete job;
    }
    {
        auto job = jobFactory->uploadFile();
        job->setRemoteFilename(path + "/test.txt");
        job->setData("Hello World");
        job->start();
        QSignalSpy spy(job, &SynqClient::AbstractJob::finished);
        QVERIFY(spy.wait());
        delete job;
    }
    {
        auto job = jobFactory->getFileInfo();
        job->setPath(path + "/test.txt");
        job->start();
        QSignalSpy spy(job, &SynqClient::AbstractJob::finished);
        QVERIFY(spy.wait());
        qDebug() << job->fileInfo().name() << job->fileInfo().path()
                 << job->fileInfo().syncAttribute();
        delete job;
    }
    {
        auto job = jobFactory->listFiles();
        job->setPath(path);
        job->start();
        QSignalSpy spy(job, &SynqClient::AbstractJob::finished);
        QVERIFY(spy.wait());
        for (const auto& entry : job->entries()) {
            qDebug() << entry.name() << entry.path() << entry.syncAttribute();
        }
        delete job;
    }
    {
        auto job = jobFactory->downloadFile();
        job->setRemoteFilename(path + "/test.txt");
        job->start();
        QSignalSpy spy(job, &SynqClient::AbstractJob::finished);
        QVERIFY(spy.wait());
        qDebug() << job->fileInfo().name() << job->fileInfo().path()
                 << job->fileInfo().syncAttribute();
        delete job;
    }
    {
        auto job = jobFactory->createDirectory();
        job->setPath(path + "/top");
        job->start();
        QSignalSpy spy(job, &SynqClient::AbstractJob::finished);
        QVERIFY(spy.wait());
        delete job;
    }
    {
        auto job = jobFactory->createDirectory();
        job->setPath(path + "/top/sub");
        job->start();
        QSignalSpy spy(job, &SynqClient::AbstractJob::finished);
        QVERIFY(spy.wait());
        delete job;
    }
    {
        auto job = jobFactory->getFileInfo();
        job->setPath(path + "/top");
        job->start();
        QSignalSpy spy(job, &SynqClient::AbstractJob::finished);
        QVERIFY(spy.wait());
        qDebug() << "File info of top:" << job->fileInfo().name() << job->fileInfo().path()
                 << job->fileInfo().syncAttribute();
        delete job;
    }
    {
        auto job = jobFactory->uploadFile();
        job->setRemoteFilename(path + "/top/sub/test.txt");
        job->setData("Hello World");
        job->start();
        QSignalSpy spy(job, &SynqClient::AbstractJob::finished);
        QVERIFY(spy.wait());
        delete job;
    }
    QThread::sleep(2);
    {
        auto job = jobFactory->getFileInfo();
        job->setPath(path + "/top");
        job->start();
        QSignalSpy spy(job, &SynqClient::AbstractJob::finished);
        QVERIFY(spy.wait());
        qDebug() << "File info of top after change:" << job->fileInfo().name()
                 << job->fileInfo().path() << job->fileInfo().syncAttribute();
        delete job;
    }
}

void DirectorySynchronizerTest::failIfNotCreatingRemoteFolders()
{
    if (!SynqClient::UnitTest::hasWebDAVServersFromEnv()
        && !SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No servers configured - skipping test");
    }

    QFETCH(AbstractJobFactory*, jobFactory);

    QTemporaryDir tmpDir;
    QTemporaryDir metaTmpDir;

    auto uuid = QUuid::createUuid();
    auto path = "DirectorySynchronizerTest-failIfNotCreatingRemoteFolders-" + uuid.toString()
            + "/foo/bar/baz";
    auto dbPath = metaTmpDir.path() + "/syncdb.json";
    QVERIFY(writeFile(tmpDir.path() + "/top/sub/test.txt", "Hello World!\n"));

    DirectorySynchronizer sync;
    sync.setJobFactory(jobFactory);
    auto flags = sync.flags();
    flags = flags & ~SynchronizerFlags(SynchronizerFlag::CreateRemoteFolderOnFirstSync);
    sync.setFlags(flags);
    sync.setLocalDirectoryPath(tmpDir.path());
    sync.setRemoteDirectoryPath(path);
    sync.setSyncStateDatabase(new JSONSyncStateDatabase(dbPath, &sync));
    sync.start();
    QSignalSpy spy(&sync, &DirectorySynchronizer::finished);
    QVERIFY(spy.wait());
    QCOMPARE(sync.error(), SynchronizerError::FailedListingRemoteFolder);
}

void DirectorySynchronizerTest::simpleSyncAndConflictResolution()
{
    if (!SynqClient::UnitTest::hasWebDAVServersFromEnv()
        && !SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No servers configured - skipping test");
    }

    QFETCH(AbstractJobFactory*, jobFactory);
    QFETCH(int, flags);

    if (flags & static_cast<int>(SynqClient::UnitTest::WebDAVServerFlag::NoIfMatch)) {
        QSKIP("WebDAV server does not support If-Match properly - skipping test");
    }

    QTemporaryDir tmpDir1;
    QTemporaryDir tmpDir2;
    QTemporaryDir metaTmpDir;

    auto uuid = QUuid::createUuid();
    auto path = "DirectorySynchronizerTest-failIfNotCreatingRemoteFolders-" + uuid.toString()
            + "/foo/bar/baz";
    auto dbPath1 = metaTmpDir.path() + "/syncdb1.json";
    auto dbPath2 = metaTmpDir.path() + "/syncdb2.json";
    QVERIFY(writeFile(tmpDir1.path() + "/top/sub/test.txt", "Hello World!\n"));

    // Sync from path1 -> server -> path2
    QVERIFY(syncDir(tmpDir1.path(), path, dbPath1, jobFactory));
    QVERIFY(syncDir(tmpDir2.path(), path, dbPath2, jobFactory));

    // Check if file is present in second path and if the content matches:
    QCOMPARE(readFile(tmpDir2.path() + "/top/sub/test.txt"), "Hello World!\n");

    // Generate a sync conflict: Both clients modify the file, the first one wins (via server-first
    // conflict resolution):
    QVERIFY(writeFile(tmpDir1.path() + "/top/sub/test.txt", "Edited by client 1\n"));
    QVERIFY(writeFile(tmpDir2.path() + "/top/sub/test.txt", "Edited by client 2\n"));
    QVERIFY(syncDir(tmpDir1.path(), path, dbPath1, jobFactory));
    QVERIFY(syncDir(tmpDir2.path(), path, dbPath2, jobFactory));
    QVERIFY(syncDir(tmpDir1.path(), path, dbPath1, jobFactory));
    QCOMPARE(readFile(tmpDir1.path() + "/top/sub/test.txt"), "Edited by client 1\n");
    QCOMPARE(readFile(tmpDir2.path() + "/top/sub/test.txt"), "Edited by client 1\n");

    // Generate another sync conflict: This time, use Local Wins, so we expect the change from the
    // second folder to win:
    QVERIFY(writeFile(tmpDir1.path() + "/top/sub/test.txt", "Edited again by client 1\n"));
    QVERIFY(writeFile(tmpDir2.path() + "/top/sub/test.txt", "Edited again by client 2\n"));
    QVERIFY(syncDir<SyncConflictStrategy::LocalWins>(tmpDir1.path(), path, dbPath1, jobFactory));
    QVERIFY(syncDir<SyncConflictStrategy::LocalWins>(tmpDir2.path(), path, dbPath2, jobFactory));
    QVERIFY(syncDir<SyncConflictStrategy::LocalWins>(tmpDir1.path(), path, dbPath1, jobFactory));
    QCOMPARE(readFile(tmpDir1.path() + "/top/sub/test.txt"), "Edited again by client 2\n");
    QCOMPARE(readFile(tmpDir2.path() + "/top/sub/test.txt"), "Edited again by client 2\n");

    // Test sync of deletions
    QVERIFY(QFile::remove(tmpDir1.path() + "/top/sub/test.txt"));
    QVERIFY(syncDir(tmpDir1.path(), path, dbPath1, jobFactory));
    QVERIFY(syncDir(tmpDir2.path(), path, dbPath2, jobFactory));
    QVERIFY(syncDir(tmpDir1.path(), path, dbPath1, jobFactory));
    QVERIFY(!QFile::exists(tmpDir1.path() + "/top/sub/test.txt"));
    QVERIFY(!QFile::exists(tmpDir2.path() + "/top/sub/test.txt"));
}

void DirectorySynchronizerTest::editVsDeleteConflictResolution()
{
    if (!SynqClient::UnitTest::hasWebDAVServersFromEnv()
        && !SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No servers configured - skipping test");
    }

    QFETCH(AbstractJobFactory*, jobFactory);
    QFETCH(int, flags);

    if (flags & static_cast<int>(SynqClient::UnitTest::WebDAVServerFlag::NoIfMatch)) {
        QSKIP("WebDAV server does not support If-Match properly - skipping test");
    }

    QTemporaryDir tmpDir1;
    QTemporaryDir tmpDir2;
    QTemporaryDir metaTmpDir;

    auto uuid = QUuid::createUuid();
    auto path = "DirectorySynchronizerTest-failIfNotCreatingRemoteFolders-" + uuid.toString()
            + "/foo/bar/baz";
    auto dbPath1 = metaTmpDir.path() + "/syncdb1.json";
    auto dbPath2 = metaTmpDir.path() + "/syncdb2.json";
    QVERIFY(writeFile(tmpDir1.path() + "/top/sub/test.txt", "Hello World!\n"));

    // Sync from path1 -> server -> path2
    QVERIFY(syncDir(tmpDir1.path(), path, dbPath1, jobFactory));
    QVERIFY(syncDir(tmpDir2.path(), path, dbPath2, jobFactory));

    // Check if file is present in second path and if the content matches:
    QCOMPARE(readFile(tmpDir2.path() + "/top/sub/test.txt"), "Hello World!\n");

    // Generate a sync conflict: Client 1 edits and syncs first, client 2 deletes. We expect the
    // edit to propagate.
    QVERIFY(writeFile(tmpDir1.path() + "/top/sub/test.txt", "Edited by client 1\n"));
    QVERIFY(QFile::remove(tmpDir2.path() + "/top/sub/test.txt"));
    QVERIFY(syncDir(tmpDir1.path(), path, dbPath1, jobFactory));
    QVERIFY(syncDir(tmpDir2.path(), path, dbPath2, jobFactory));
    QVERIFY(syncDir(tmpDir1.path(), path, dbPath1, jobFactory));
    QCOMPARE(readFile(tmpDir1.path() + "/top/sub/test.txt"), "Edited by client 1\n");
    QCOMPARE(readFile(tmpDir2.path() + "/top/sub/test.txt"), "Edited by client 1\n");

    // Generate another conflict: Same order, but local wins. We expect the deletion to win.
    QVERIFY(writeFile(tmpDir1.path() + "/top/sub/test.txt", "Edited again by client 1\n"));
    QVERIFY(QFile::remove(tmpDir2.path() + "/top/sub/test.txt"));
    QVERIFY(syncDir<SyncConflictStrategy::LocalWins>(tmpDir1.path(), path, dbPath1, jobFactory));
    QVERIFY(syncDir<SyncConflictStrategy::LocalWins>(tmpDir2.path(), path, dbPath2, jobFactory));
    QVERIFY(syncDir<SyncConflictStrategy::LocalWins>(tmpDir1.path(), path, dbPath1, jobFactory));
    QVERIFY(!QFile::exists(tmpDir1.path() + "/top/sub/test.txt"));
    QVERIFY(!QFile::exists(tmpDir2.path() + "/top/sub/test.txt"));

    // Generate another delete/edit conflict. This time, do not delete a single file but the whole
    // folder structure in folder 2:
    QVERIFY(writeFile(tmpDir1.path() + "/top/sub/test.txt", "Re-created by client 1\n"));
    QVERIFY(QDir(tmpDir2.path() + "/top").removeRecursively());
    QVERIFY(syncDir(tmpDir1.path(), path, dbPath1, jobFactory));
    QVERIFY(syncDir(tmpDir2.path(), path, dbPath2, jobFactory));
    QVERIFY(syncDir(tmpDir1.path(), path, dbPath1, jobFactory));
    QCOMPARE(readFile(tmpDir1.path() + "/top/sub/test.txt"), "Re-created by client 1\n");
    QCOMPARE(readFile(tmpDir2.path() + "/top/sub/test.txt"), "Re-created by client 1\n");

    // Same story again, but with local-wins we expect the deletion to win:
    QVERIFY(writeFile(tmpDir1.path() + "/top/sub/test.txt", "Re-created again by client 1\n"));
    QVERIFY(QDir(tmpDir2.path() + "/top").removeRecursively());
    QVERIFY(syncDir<SyncConflictStrategy::LocalWins>(tmpDir1.path(), path, dbPath1, jobFactory));
    QVERIFY(syncDir<SyncConflictStrategy::LocalWins>(tmpDir2.path(), path, dbPath2, jobFactory));
    QVERIFY(syncDir<SyncConflictStrategy::LocalWins>(tmpDir1.path(), path, dbPath1, jobFactory));
    QVERIFY(!QFile::exists(tmpDir1.path() + "/top/sub/test.txt"));
    QVERIFY(!QFile::exists(tmpDir2.path() + "/top/sub/test.txt"));
    QVERIFY(!QDir(tmpDir1.path() + "/top").exists());
    QVERIFY(!QDir(tmpDir2.path() + "/top").exists());
}

void DirectorySynchronizerTest::sync()
{
    if (!SynqClient::UnitTest::hasWebDAVServersFromEnv()
        && !SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No servers configured - skipping test");
    }

    QFETCH(AbstractJobFactory*, jobFactory);
    QFETCH(int, flags);

    if (flags & static_cast<int>(SynqClient::UnitTest::WebDAVServerFlag::NoIfMatch)) {
        QSKIP("WebDAV server does not support If-Match properly - skipping test");
    }

    QTemporaryDir tmpDir1;
    QTemporaryDir tmpDir2;
    QTemporaryDir meta;

    auto uuid = QUuid::createUuid();
    auto path = "/DirectorySynchronizerTest-sync-" + uuid.toString() + "/foo/bar/baz";

    auto syncStateDb1Path = meta.path() + "/sync1.json";
    auto syncStateDb2Path = meta.path() + "/sync2.json";

    QVERIFY(fillTestFolder(tmpDir1.path()));
    auto files1 = readDirectory(tmpDir1.path());

    // Run first sync: dir1 -> server -> dir2
    QVERIFY(syncDir(tmpDir1.path(), path, syncStateDb1Path, jobFactory));
    QVERIFY(syncDir(tmpDir2.path(), path, syncStateDb2Path, jobFactory));

    auto files2 = readDirectory(tmpDir2.path());

    // If the sync went well, we should have all text files from the first folder in the second one
    // (synced via the server).
    for (auto syncedFilePath : files1.keys()) {
        if (syncedFilePath.endsWith(".dat")) {
            // Binary files are excluded by filter
            QVERIFY(!files2.contains(syncedFilePath));
        } else {
            // If files are synced, make sure they are equal:
            auto msg = QString("File %1 missing").arg(syncedFilePath);
            (void)msg;
            QVERIFY2(files2.contains(syncedFilePath), qUtf8Printable(msg));
            msg = QString("File %1 differs: Expected content - %3. Got content - %2")
                          .arg(syncedFilePath, files2.value(syncedFilePath),
                               files1.value(syncedFilePath));
            QVERIFY2(files2.value(syncedFilePath) == files1.value(syncedFilePath),
                     qUtf8Printable(msg));
        }
    }

    // Rerun sync, this should have no effect (as we didn't change anything):
    QVERIFY(syncDir(tmpDir1.path(), path, syncStateDb1Path, jobFactory));
    QVERIFY(syncDir(tmpDir2.path(), path, syncStateDb2Path, jobFactory));

    for (auto syncedFilePath : files1.keys()) {
        if (syncedFilePath.endsWith(".dat")) {
            // Binary files are excluded by filter
            QVERIFY(!files2.contains(syncedFilePath));
        } else {
            // If files are synced, make sure they are equal:
            QVERIFY(files2.contains(syncedFilePath));
            QCOMPARE(files2.value(syncedFilePath), files1.value(syncedFilePath));
        }
    }

    // Edit files and rerun sync
    QVERIFY(editDirectory(tmpDir1.path(), files1));
    files1 = readDirectory(tmpDir1.path());

    QVERIFY(syncDir(tmpDir1.path(), path, syncStateDb1Path, jobFactory));
    QVERIFY(syncDir(tmpDir2.path(), path, syncStateDb2Path, jobFactory));

    files2 = readDirectory(tmpDir2.path());

    for (auto syncedFilePath : files1.keys()) {
        if (syncedFilePath.endsWith(".dat")) {
            // Binary files are excluded by filter
            QVERIFY(!files2.contains(syncedFilePath));
        } else {
            // If files are synced, make sure they are equal:
            QVERIFY(files2.contains(syncedFilePath));
            QCOMPARE(files2.value(syncedFilePath), files1.value(syncedFilePath));
        }
    }

    // Edit files in both folders - as we run with "server wins", we expect the versions
    // from the folder which is first synced
    auto baseFiles = files1;
    QVERIFY(editDirectory(tmpDir1.path(), files1));
    files1 = readDirectory(tmpDir1.path());

    QVERIFY(editDirectory(tmpDir2.path(), files2));
    files2 = readDirectory(tmpDir2.path());

    // Build the list of expected contents: If we have a conflict, after the sync we
    // expect the version from path1 to be taken:
    decltype(files1) expectedFiles;
    for (const auto& filePath : files1.keys()) {
        auto contentBase = baseFiles.value(filePath);
        auto content1 = files1.value(filePath);
        auto content2 = files2.value(filePath);
        if (contentBase != content1) {
            expectedFiles[filePath] = content1;
            qDebug() << filePath << "changed in dir1";
        } else if (content2 != contentBase) {
            qDebug() << filePath << "changed in dir2";
            expectedFiles[filePath] = content2;
        } else {
            expectedFiles[filePath] = contentBase;
        }
    }

    qDebug() << "===================================";
    QVERIFY(syncDir(tmpDir1.path(), path, syncStateDb1Path, jobFactory));
    QVERIFY(syncDir(tmpDir2.path(), path, syncStateDb2Path, jobFactory));
    QVERIFY(syncDir(tmpDir1.path(), path, syncStateDb1Path, jobFactory)); // Ensure both are in sync

    files1 = readDirectory(tmpDir1.path());
    files2 = readDirectory(tmpDir2.path());

    for (const auto& syncedFilePath : expectedFiles.keys()) {
        if (syncedFilePath.endsWith(".dat")) {
            // Binary files are excluded by filter
            QVERIFY(!files2.contains(syncedFilePath));
        } else {
            auto msg = QString("Content mismatch of %1 in dir1").arg(syncedFilePath);
            (void)msg;
            QVERIFY2(files1.value(syncedFilePath) == expectedFiles.value(syncedFilePath),
                     qUtf8Printable(msg));

            // If files are synced, make sure they are equal:
            QVERIFY(files2.contains(syncedFilePath));
            msg = QString("File %1 differs: Expected content - %3. Got content - %2")
                          .arg(syncedFilePath, files2.value(syncedFilePath),
                               files1.value(syncedFilePath));
            QVERIFY2(files2.value(syncedFilePath) == files1.value(syncedFilePath),
                     qUtf8Printable(msg));
        }
    }

    // Same story, but this time sync with "client wins" -> we expect files from path2 to have
    // precedence.
    baseFiles = files1;
    QVERIFY(editDirectory(tmpDir1.path(), files1));
    files1 = readDirectory(tmpDir1.path());

    QVERIFY(editDirectory(tmpDir2.path(), files2));
    files2 = readDirectory(tmpDir2.path());

    // Build the list of expected contents: If we have a conflict, after the sync we
    // expect the version from path1 to be taken:
    expectedFiles.clear();
    for (const auto& filePath : files1.keys()) {
        auto contentBase = baseFiles.value(filePath);
        auto content1 = files1.value(filePath);
        auto content2 = files2.value(filePath);
        if (contentBase != content2) {
            expectedFiles[filePath] = content2;
        } else if (content1 != contentBase) {
            expectedFiles[filePath] = content1;
        } else {
            expectedFiles[filePath] = contentBase;
        }
    }

    QVERIFY(syncDir<SyncConflictStrategy::LocalWins>(tmpDir1.path(), path, syncStateDb1Path,
                                                     jobFactory));
    QVERIFY(syncDir<SyncConflictStrategy::LocalWins>(tmpDir2.path(), path, syncStateDb2Path,
                                                     jobFactory));
    QVERIFY(syncDir<SyncConflictStrategy::LocalWins>(tmpDir1.path(), path, syncStateDb1Path,
                                                     jobFactory)); // Ensure both are in sync

    files1 = readDirectory(tmpDir1.path());
    files2 = readDirectory(tmpDir2.path());

    for (auto syncedFilePath : expectedFiles.keys()) {
        if (syncedFilePath.endsWith(".dat")) {
            // Binary files are excluded by filter
            QVERIFY(!files2.contains(syncedFilePath));
        } else {
            // If files are synced, make sure they are equal:
            QCOMPARE(files1.value(syncedFilePath), expectedFiles.value(syncedFilePath));
            QVERIFY(files2.contains(syncedFilePath));

            auto msg = QString("File %1 differs: Expected content - %3. Got content - %2")
                               .arg(syncedFilePath, files2.value(syncedFilePath),
                                    files1.value(syncedFilePath));
            QVERIFY2(files2.value(syncedFilePath) == files1.value(syncedFilePath),
                     qUtf8Printable(msg));
        }
    }
}

void DirectorySynchronizerTest::cleanupTestCase() {}

void DirectorySynchronizerTest::prepareTestData()
{
    QTest::addColumn<AbstractJobFactory*>("jobFactory");
    QTest::addColumn<int>("flags");

    auto nam = new QNetworkAccessManager(this);
    nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    for (const auto& tuple : SynqClient::UnitTest::enumerateWebDAVTestServers()) {
        auto url = std::get<0>(tuple);
        auto type = std::get<1>(tuple);
        auto flags = std::get<2>(tuple);
        auto webdavJobFactory = new WebDAVJobFactory(this);
        webdavJobFactory->setUrl(url);
        webdavJobFactory->setServerType(type);
        webdavJobFactory->setNetworkAccessManager(nam);
        QTest::newRow(url.toString().toUtf8())
                << static_cast<AbstractJobFactory*>(webdavJobFactory) << flags;
    }

    if (SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        auto factory = new SynqClient::DropboxJobFactory(this);
        factory->setNetworkAccessManager(nam);
        factory->setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        QTest::newRow("Dropbox") << static_cast<AbstractJobFactory*>(factory) << 0;
    }
}

bool DirectorySynchronizerTest::fillTestFolder(const QString& path)
{
    // This fills the test folder with some sample data. We use the following structure (mimicking
    // what is used in the OpenTodoList app, from where the library has been split from): Each file
    // is stored in a folder structure <year>/<month>/ (where the path is determined by the date the
    // file is created on). OpenTodoList put files only on this nesting level, however, we will also
    // create files below the year folders. To test filtering, we create two types of files: Binary
    // ones and text files.
    QDir dir(path);
    SQ_VERIFY(createRandomFiles(path));
    for (int year = 2020; year <= 2021; ++year) {
        SQ_VERIFY(dir.mkdir(QString::number(year)));
        SQ_VERIFY(dir.cd(QString::number(year)));
        SQ_VERIFY(createRandomFiles(dir.path()));
        for (int month = 1; month <= 12; ++month) {
            SQ_VERIFY(dir.mkdir(QString::number(month)));
            SQ_VERIFY(dir.cd(QString::number(month)));
            SQ_VERIFY(createRandomFiles(dir.path()));
            SQ_VERIFY(dir.cdUp());
        }
        SQ_VERIFY(dir.cdUp());
    }
    return true;
}

bool DirectorySynchronizerTest::createRandomFiles(const QString& path)
{
    auto prng = QRandomGenerator::global();
    auto n = 1 + prng->generate() % 10;
    for (unsigned int i = 0; i < n; ++i) {
        auto name = QUuid::createUuid().toString();
        bool text = prng->generate() % 2 == 0;
        if (text) {
            // Create a text file
            QFile file(path + "/" + name + ".txt");
            SQ_VERIFY(file.open(QIODevice::WriteOnly));
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
            SQ_VERIFY(file.open(QIODevice::WriteOnly));
            auto wordCount = 10 + prng->generate() % 1000;
            while (wordCount > 0) {
                --wordCount;
                auto word = prng->generate();
                file.write(reinterpret_cast<const char*>(&word), sizeof(word));
            }
            file.close();
        }
    }
    return true;
}

QMap<QString, QByteArray> DirectorySynchronizerTest::readDirectory(const QString& path)
{
    QDir dir(path);
    decltype(readDirectory(".")) result;
    QDirIterator it(path, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        auto filePath = it.next();
        if (!it.fileInfo().isFile()) {
            continue;
        }
        QFile file(filePath);
        Q_ASSERT(file.open(QIODevice::ReadOnly));
        auto relativePath = dir.relativeFilePath(filePath);
        result[relativePath] = file.readAll();
        file.close();
    }
    return result;
}

bool DirectorySynchronizerTest::editDirectory(const QString& path,
                                              QMap<QString, QByteArray> contents)
{
    auto prng = QRandomGenerator::global();
    for (const auto& filePath : contents.keys()) {
        if (prng->generate() % 2 == 0) {
            // Edit the file
            if (filePath.endsWith(".txt")) {
                // Edit text file
                QFile file(path + "/" + filePath);
                SQ_VERIFY(file.open(QIODevice::WriteOnly));
                QTextStream stream(&file);
                auto wordCount = 10 + prng->generate() % 1000;
                while (wordCount > 0) {
                    --wordCount;
                    stream << prng->generate() << " ";
                }
                file.close();
            } else {
                // Edit binary file
                QFile file(path + "/" + filePath);
                SQ_VERIFY(file.open(QIODevice::WriteOnly));
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
    return true;
}

bool DirectorySynchronizerTest::writeFile(const QString& fileName, const QByteArray& data) const
{
    QFileInfo fi(fileName);
    if (fi.dir().mkpath(".")) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(data);
            file.close();
            return true;
        }
    }
    return false;
}

QByteArray DirectorySynchronizerTest::readFile(const QString& fileName) const
{
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        return file.readAll();
    }
    return QByteArray();
}

template<SyncConflictStrategy strategy>
bool DirectorySynchronizerTest::syncDir(const QString& localPath, const QString& remotePath,
                                        const QString& syncDbPath,
                                        SynqClient::AbstractJobFactory* jobFactory)
{
    JSONSyncStateDatabase syncDb(syncDbPath);
    DirectorySynchronizer sync;
    sync.setJobFactory(jobFactory);
    sync.setFilter([](const QString& path, const FileInfo&) { return !path.endsWith(".dat"); });
    sync.setLocalDirectoryPath(localPath);
    sync.setRemoteDirectoryPath(remotePath);
    sync.setSyncStateDatabase(&syncDb);
    sync.setSyncConflictStrategy(strategy);
    SQ_COMPARE(sync.state(), SynchronizerState::Ready);
    SQ_COMPARE(sync.error(), SynchronizerError::NoError);
    sync.start();
    SQ_COMPARE(sync.state(), SynchronizerState::Running);
    SQ_COMPARE(sync.error(), SynchronizerError::NoError);
    QSignalSpy spy(&sync, &DirectorySynchronizer::finished);
    SQ_VERIFY(spy.wait(1000 * 60 * 10));
    SQ_COMPARE(sync.state(), SynchronizerState::Finished);
    qDebug() << sync.errorString();
    SQ_COMPARE(sync.errorString(), QString());
    SQ_COMPARE(sync.error(), SynchronizerError::NoError);
    // We need to have a minimal delay between syncs. This is because otherwise sync properties
    // might not yet be regenerated and hence we won't see the updates. In real life, this is less
    // of a problem, because usually the user will have pauses between syncs (and even if not,
    // changes will be catched up on the next sync run). However, for unit tests we need the delay
    // to ensure the server is ready to go on.
    QThread::sleep(1);
    return true;
}

QTEST_MAIN(DirectorySynchronizerTest)

#include "tst_directorysynchronizer.moc"
