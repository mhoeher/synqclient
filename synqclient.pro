TEMPLATE = subdirs

SUBDIRS += \
    libsynqclient \
    tests

tests.depends += libsynqclient

OTHER_FILES += \
    $$files(model/*) \
    .gitlab-ci.yml \
    .gitignore \
    Doxyfile \
    $$files(ci/*.sh) \
    $$files(utils/*.*) \
    $$files(doc/*,true)
