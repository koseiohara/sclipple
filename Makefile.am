SUBDIRS = src

TESTS = \
    tests/cli/basic.sh \
    tests/cli/tags.sh \
    tests/cli/selectors.sh \
    tests/cli/tag-match.sh \
    tests/cli/config.sh \
    tests/cli/mv-rm.sh \
    tests/cli/storage.sh \
    tests/cli/git.sh

TESTS_ENVIRONMENT = \
	SCLIPPLE_TEST_BIN='$(abs_top_builddir)/src/sclipple'

EXTRA_DIST = \
    LICENSE \
    tests/cli.sh \
    tests/cli/common.sh \
    $(TESTS)


