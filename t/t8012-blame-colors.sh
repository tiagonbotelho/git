#!/bin/sh

test_description='colored git blame'
. ./test-lib.sh

PROG='git blame -c'
. "$TEST_DIRECTORY"/annotate-tests.sh

test_expect_success 'colored blame colors contiguous lines' '
	git blame --abbrev=12 --color-lines hello.c >actual.raw &&
	test_decode_color <actual.raw >actual &&
	grep "<CYAN>" <actual >darkened &&
	grep "(F" darkened > F.expect &&
	grep "(H" darkened > H.expect &&
	test_line_count = 2 F.expect &&
	test_line_count = 3 H.expect
'

test_expect_success 'colored blame colors contiguous lines via config' '
	git -c color.blame.repeatedLines=yellow blame --abbrev=12 hello.c >actual.raw &&
	test_decode_color <actual.raw >actual &&
	grep "<YELLOW>" <actual >darkened &&
	grep "(F" darkened > F.expect &&
	grep "(H" darkened > H.expect &&
	test_line_count = 2 F.expect &&
	test_line_count = 3 H.expect
'

test_done
