TEMPLATE = subdirs

SUBDIRS += libsynqclient

!synqclient_with_no_tests {
    SUBDIRS += tests
    tests.depends += libsynqclient
}

OTHER_FILES += \
    $$files(model/*) \
    .gitlab-ci.yml \
    .gitignore \
    Doxyfile \
    $$files(ci/*.sh) \
    $$files(utils/*.*) \
    $$files(doc/*,true)
