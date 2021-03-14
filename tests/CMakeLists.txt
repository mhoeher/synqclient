macro(synqclient_add_test TEST_NAME)
    set(TEST_SOURCES tst_${TEST_NAME}.cpp ../shared/utils.cpp)
    set(TEST_HEADERS ../shared/utils.h)
    add_executable(${TEST_NAME})
    target_sources(
        ${TEST_NAME}
        PRIVATE
            ${TEST_SOURCES}
            ${TEST_HEADERS}
    )
    target_link_libraries(
        ${TEST_NAME}
        PRIVATE
            Qt${QT_VERSION_MAJOR}::Core
            Qt${QT_VERSION_MAJOR}::Network
            Qt${QT_VERSION_MAJOR}::Test
            synqclient-qt${QT_VERSION_MAJOR}
    )
    add_test(
        NAME
            ${TEST_NAME}
        COMMAND
            ${TEST_NAME}
    )
endmacro(synqclient_add_test)


add_subdirectory(abstractjob)
add_subdirectory(compositejob)
add_subdirectory(directorysynchronizer)
add_subdirectory(syncstatedatabase)
add_subdirectory(webdavcreatedirectoryjob)
add_subdirectory(webdavdeletejob)
add_subdirectory(webdavdownloadfilejob)
add_subdirectory(webdavgetfileinfojob)
add_subdirectory(webdavjobfactory)
add_subdirectory(webdavlistfilesjob)
add_subdirectory(webdavuploadfilejob)
