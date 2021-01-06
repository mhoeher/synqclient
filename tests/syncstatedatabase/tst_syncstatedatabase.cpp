#include <algorithm>

#include <QSharedPointer>
#include <QTemporaryDir>
#include <QtTest>

// add necessary includes here
#include "JSONSyncStateDatabase"
#include "SQLSyncStateDatabase"
#include "SyncStateDatabase"
#include "SyncStateEntry"

using SynqClient::JSONSyncStateDatabase;
using SynqClient::SQLSyncStateDatabase;
using SynqClient::SyncStateDatabase;
using SynqClient::SyncStateEntry;

class SyncStateDatabaseTest : public QObject
{
    Q_OBJECT

public:
    SyncStateDatabaseTest();
    ~SyncStateDatabaseTest();

private slots:
    void initTestCase();
    void openAndClose();
    void openAndClose_data() { data(); }
    void addEntry();
    void addEntry_data() { data(); }
    void getEntry();
    void getEntry_data() { data(); }
    void findEntries();
    void findEntries_data() { data(); }
    void removeEntries();
    void removeEntries_data() { data(); }
    void removeEntry();
    void removeEntry_data() { data(); }
    void iterate();
    void iterate_data() { data(); }
    void cleanupTestCase();

private:
    void data();
    QSharedPointer<QTemporaryDir> tmpDir;
};

SyncStateDatabaseTest::SyncStateDatabaseTest() {}

SyncStateDatabaseTest::~SyncStateDatabaseTest() {}

void SyncStateDatabaseTest::initTestCase() {}

void SyncStateDatabaseTest::openAndClose()
{
    QFETCH(SyncStateDatabase*, db);
    QVERIFY(!db->isOpen());
    QVERIFY(db->openDatabase());
    QVERIFY(db->isOpen());
    QVERIFY(db->closeDatabase());
    QVERIFY(!db->isOpen());
    QVERIFY(db->openDatabase());
    QVERIFY(db->isOpen());
    QVERIFY(!db->openDatabase());
    QVERIFY(db->isOpen());
    QVERIFY(db->closeDatabase());
    QVERIFY(!db->isOpen());
    QVERIFY(db->openDatabase());
    QVERIFY(db->isOpen());
    QVERIFY(db->closeDatabase());
    QVERIFY(!db->isOpen());
    QVERIFY(!db->closeDatabase());
    QVERIFY(!db->isOpen());
}

void SyncStateDatabaseTest::addEntry()
{
    QFETCH(SyncStateDatabase*, db);
    QVERIFY(db->openDatabase());
    {
        SyncStateEntry entry("/", QDateTime::currentDateTime(), "v1");
        QVERIFY(db->addEntry(entry));
    }
    QVERIFY(db->closeDatabase());

    QVERIFY(db->openDatabase());
    {
        SyncStateEntry entry("/", QDateTime::currentDateTime(), "v2");
        QVERIFY(db->addEntry(entry));
    }
    QVERIFY(db->closeDatabase());

    QVERIFY(db->openDatabase());
    {
        SyncStateEntry entry("/foo/bar.txt", QDateTime::currentDateTime(), "v1");
        QVERIFY(db->addEntry(entry));
    }
    QVERIFY(db->closeDatabase());
}

void SyncStateDatabaseTest::getEntry()
{
    QFETCH(SyncStateDatabase*, db);
    QDateTime lastModTime, lastModTime2;
    QVERIFY(db->openDatabase());
    {
        SyncStateEntry entry("/", QDateTime::currentDateTime(), "v1");
        lastModTime = entry.modificationTime();
        QVERIFY(db->addEntry(entry));
    }
    QVERIFY(db->closeDatabase());

    QVERIFY(db->openDatabase());
    {
        auto entry = db->getEntry("/");
        QVERIFY(entry.isValid());
        QCOMPARE(entry.path(), "/");
        QCOMPARE(entry.modificationTime(), lastModTime);
        QCOMPARE(entry.syncProperty(), "v1");
    }
    QVERIFY(db->closeDatabase());

    QVERIFY(db->openDatabase());
    {
        SyncStateEntry entry("/", QDateTime::currentDateTime(), "v2");
        lastModTime = entry.modificationTime();
        QVERIFY(db->addEntry(entry));
    }
    QVERIFY(db->closeDatabase());

    QVERIFY(db->openDatabase());
    {
        SyncStateEntry entry("/foo/bar.txt", QDateTime::currentDateTime(), "v1");
        lastModTime2 = entry.modificationTime();
        QVERIFY(db->addEntry(entry));
    }
    QVERIFY(db->closeDatabase());

    QVERIFY(db->openDatabase());
    {
        auto entry = db->getEntry("/");
        QVERIFY(entry.isValid());
        QCOMPARE(entry.path(), "/");
        QCOMPARE(entry.syncProperty(), "v2");
        QCOMPARE(entry.modificationTime(), lastModTime);
    }
    QVERIFY(db->closeDatabase());

    QVERIFY(db->openDatabase());
    {
        auto entry = db->getEntry("/foo/bar.txt");
        QVERIFY(entry.isValid());
        QCOMPARE(entry.path(), "/foo/bar.txt");
        QCOMPARE(entry.modificationTime(), lastModTime2);
        QCOMPARE(entry.syncProperty(), "v1");
    }
    QVERIFY(db->closeDatabase());

    QVERIFY(db->openDatabase());
    {
        auto entry = db->getEntry("/baz/bar.txt");
        QVERIFY(!entry.isValid());
    }
    QVERIFY(db->closeDatabase());
}

void SyncStateDatabaseTest::findEntries()
{
    QFETCH(SyncStateDatabase*, db);
    QVERIFY(db->openDatabase());
    QDateTime modTimes[4] = { QDateTime::currentDateTime(), QDateTime::currentDateTime(),
                              QDateTime::currentDateTime(), QDateTime::currentDateTime() };
    QVERIFY(db->addEntry(SyncStateEntry("/foo/bar1.txt", modTimes[0], "v1")));
    QVERIFY(db->addEntry(SyncStateEntry("/foo/bar2.txt", modTimes[1], "v2")));
    QVERIFY(db->addEntry(SyncStateEntry("/foo/bar3.txt", modTimes[2], "v3")));
    QVERIFY(db->addEntry(SyncStateEntry("/foo/baz/bar1.txt", modTimes[3], "v4")));
    bool ok;
    auto entries = db->findEntries("/foo", &ok);
    QVERIFY(ok);
    QCOMPARE(entries.length(), 3);
    std::sort(entries.begin(), entries.end(),
              [](const SyncStateEntry& left, const SyncStateEntry& right) {
                  return left.path() < right.path();
              });
    QCOMPARE(entries[0].path(), "/foo/bar1.txt");
    QCOMPARE(entries[0].syncProperty(), "v1");
    QCOMPARE(entries[0].modificationTime(), modTimes[0]);
    QCOMPARE(entries[1].path(), "/foo/bar2.txt");
    QCOMPARE(entries[1].syncProperty(), "v2");
    QCOMPARE(entries[1].modificationTime(), modTimes[1]);
    QCOMPARE(entries[2].path(), "/foo/bar3.txt");
    QCOMPARE(entries[2].syncProperty(), "v3");
    QCOMPARE(entries[2].modificationTime(), modTimes[2]);
    QVERIFY(db->closeDatabase());
}

void SyncStateDatabaseTest::removeEntries()
{
    QFETCH(SyncStateDatabase*, db);
    QVERIFY(db->openDatabase());
    QDateTime modTimes[4] = { QDateTime::currentDateTime(), QDateTime::currentDateTime(),
                              QDateTime::currentDateTime(), QDateTime::currentDateTime() };
    QVERIFY(db->addEntry(SyncStateEntry("/foo", modTimes[0], "v1")));
    QVERIFY(db->addEntry(SyncStateEntry("/foo/bar1.txt", modTimes[0], "v1")));
    QVERIFY(db->addEntry(SyncStateEntry("/foo/bar2.txt", modTimes[1], "v2")));
    QVERIFY(db->addEntry(SyncStateEntry("/foo/bar3.txt", modTimes[2], "v3")));
    QVERIFY(db->addEntry(SyncStateEntry("/foo/baz/bar1.txt", modTimes[3], "v4")));
    bool ok;
    auto entries = db->findEntries("/foo", &ok);
    QVERIFY(ok);
    QCOMPARE(entries.length(), 3);
    QVERIFY(db->removeEntries("/foo"));
    entries = db->findEntries("/foo", &ok);
    QVERIFY(ok);
    QCOMPARE(entries.length(), 0);
    auto entry = db->getEntry("/foo/baz/bar1.txt");
    QVERIFY(!entry.isValid());
    entry = db->getEntry("/foo");
    QVERIFY(!entry.isValid());
    QVERIFY(db->closeDatabase());
}

void SyncStateDatabaseTest::removeEntry()
{
    QFETCH(SyncStateDatabase*, db);
    QVERIFY(db->openDatabase());
    QDateTime modTimes[4] = { QDateTime::currentDateTime(), QDateTime::currentDateTime(),
                              QDateTime::currentDateTime(), QDateTime::currentDateTime() };
    QVERIFY(db->addEntry(SyncStateEntry("/", modTimes[0], "v1")));
    QVERIFY(db->addEntry(SyncStateEntry("/foo", modTimes[0], "v1")));
    QVERIFY(db->addEntry(SyncStateEntry("/foo/bar1.txt", modTimes[0], "v1")));
    QVERIFY(db->addEntry(SyncStateEntry("/foo/bar2.txt", modTimes[1], "v2")));
    QVERIFY(db->addEntry(SyncStateEntry("/foo/bar3.txt", modTimes[2], "v3")));
    QVERIFY(db->addEntry(SyncStateEntry("/foo/baz/bar1.txt", modTimes[3], "v4")));
    bool ok;
    auto entries = db->findEntries("/", &ok);
    QVERIFY(ok);
    QCOMPARE(entries.length(), 1);
    QCOMPARE(entries[0].path(), "/foo");

    entries = db->findEntries("/foo", &ok);
    QVERIFY(ok);
    QCOMPARE(entries.length(), 3);
    QVERIFY(db->removeEntry("/foo/bar2.txt"));
    entries = db->findEntries("/foo", &ok);
    QVERIFY(ok);
    QCOMPARE(entries.length(), 2);
    std::sort(entries.begin(), entries.end(),
              [](const SyncStateEntry& left, const SyncStateEntry& right) {
                  return left.path() < right.path();
              });
    QCOMPARE(entries[0].path(), "/foo/bar1.txt");
    QCOMPARE(entries[0].syncProperty(), "v1");
    QCOMPARE(entries[0].modificationTime(), modTimes[0]);
    QCOMPARE(entries[1].path(), "/foo/bar3.txt");
    QCOMPARE(entries[1].syncProperty(), "v3");
    QCOMPARE(entries[1].modificationTime(), modTimes[2]);
    QVERIFY(db->closeDatabase());
}

void SyncStateDatabaseTest::iterate()
{
    QFETCH(SyncStateDatabase*, db);
    QVERIFY(db->openDatabase());
    QDateTime modTimes[4] = { QDateTime::currentDateTime(), QDateTime::currentDateTime(),
                              QDateTime::currentDateTime(), QDateTime::currentDateTime() };
    QVERIFY(db->addEntry(SyncStateEntry("/", modTimes[0], "v1")));
    QVERIFY(db->addEntry(SyncStateEntry("/foo", modTimes[0], "v1")));
    QVERIFY(db->addEntry(SyncStateEntry("/foo/bar1.txt", modTimes[0], "v1")));
    QVERIFY(db->addEntry(SyncStateEntry("/foo/bar2.txt", modTimes[1], "v2")));
    QVERIFY(db->addEntry(SyncStateEntry("/foo/baz", modTimes[0], "v1")));
    QVERIFY(db->addEntry(SyncStateEntry("/foo/baz/bar1.txt", modTimes[3], "v4")));

    {
        QStringList walkedNodes;
        QVERIFY(db->iterate([&](const SyncStateEntry& entry) { walkedNodes.append(entry.path()); },
                            "/"));
        std::sort(walkedNodes.begin(), walkedNodes.end());
        QCOMPARE(walkedNodes,
                 QStringList({ "/", "/foo", "/foo/bar1.txt", "/foo/bar2.txt", "/foo/baz",
                               "/foo/baz/bar1.txt" }));
    }

    {
        QStringList walkedNodes;
        QVERIFY(db->iterate([&](const SyncStateEntry& entry) { walkedNodes.append(entry.path()); },
                            "/foo"));
        std::sort(walkedNodes.begin(), walkedNodes.end());
        QCOMPARE(walkedNodes,
                 QStringList({ "/foo", "/foo/bar1.txt", "/foo/bar2.txt", "/foo/baz",
                               "/foo/baz/bar1.txt" }));
    }

    {
        QStringList walkedNodes;
        QVERIFY(db->iterate([&](const SyncStateEntry& entry) { walkedNodes.append(entry.path()); },
                            "/foo/baz"));
        std::sort(walkedNodes.begin(), walkedNodes.end());
        QCOMPARE(walkedNodes, QStringList({ "/foo/baz", "/foo/baz/bar1.txt" }));
    }
}

void SyncStateDatabaseTest::cleanupTestCase() {}

void SyncStateDatabaseTest::data()
{
    tmpDir.reset(new QTemporaryDir);
    QTest::addColumn<SyncStateDatabase*>("db");
    QTest::addRow("SQL") << static_cast<SyncStateDatabase*>(
            new SQLSyncStateDatabase(tmpDir->path() + "sync.db", this));
    QTest::addRow("JSON") << static_cast<SyncStateDatabase*>(
            new JSONSyncStateDatabase(tmpDir->path() + "/db.json", this));
}

QTEST_MAIN(SyncStateDatabaseTest)

#include "tst_syncstatedatabase.moc"
