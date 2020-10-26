TEMPLATE = subdirs

SUBDIRS += \
    libsynqclient \
    tests

tests.depends += libsynqclient

OTHER_FILES += $$files(model/*)
