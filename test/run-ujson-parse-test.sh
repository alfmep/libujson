#!/bin/sh

BASE_DIR=$(dirname $(readlink -f "$0"))
PWD_DIR=`pwd`

if [ "$BASE_DIR" = "$PWD_DIR" ]; then
    BASE_DIR=""
else
    BASE_DIR="$BASE_DIR/"
fi

TEST_RESULT_DIR=${BASE_DIR}result-parse-test
TEST_DATA_BASE_DIR=${BASE_DIR}test-data
TEST_DATA_DIR=$TEST_DATA_BASE_DIR/json-parse-test
TEST_ALL_PARSERS=no

CLONE_URL=https://github.com/nst/JSONTestSuite.git

#
# Option '--clean' will erase test data and exit
#
if [ "$1" = "--all" ]; then
    TEST_ALL_PARSERS=yes
elif [ "$1" = "--clean" ]; then
    echo "# "
    echo "# Removing patch test data (directory $TEST_DATA_DIR)"
    echo "# Removing patch result data (directory $TEST_RESULT_DIR)"
    echo "# "
    rm -rf $TEST_RESULT_DIR
    rm -rf $TEST_DATA_DIR
    rmdir $TEST_DATA_BASE_DIR >/dev/null 2>&1
    exit 0
elif [ "$1" = "--help" ]; then
    echo ""
    echo "Usage: `basename $0` [OPTION]"
    echo "    Download test suite for testing JSON parsing and run JSON parse test."
    echo "    Test data is cloned from $CLONE_URL,"
    echo "    and installed in directory '$TEST_DATA_DIR'"
    echo ""
    echo "    Parse result files are stored in directory '$TEST_RESULT_DIR'"
    echo ""
    echo "    Options:"
    echo "        --all      Run parsing test for all parsers in JSONTestSuite, not just libujson"
    echo "        --clean    Erase test data directory and test output files"
    echo "        --help     Print this help and exit"
    echo ""
    exit 0
fi


#
# Download test and install JSON parser test suite
#
if ! [ -d $TEST_DATA_DIR ]; then
    echo "# "
    echo "# Clone git repository $CLONE_URL" >&2
    echo "# "
    mkdir -p $TEST_DATA_BASE_DIR
    if ! git clone $CLONE_URL $TEST_DATA_DIR; then
        echo "# "
        echo "# Error: Unable to clone git repository $CLONE_URL"
        echo "# "
        rm -rf $TEST_DATA_DIR
        exit 1
    fi
    echo "# "
    echo "# Apply patch file JSONTestSuite-libujson.patch" >&2
    echo "# "
    if ! patch -d $TEST_DATA_DIR -p1 < ${BASE_DIR}JSONTestSuite-libujson.patch; then
        echo "# "
        echo "# Error: Unable to apply patch"
        echo "# "
        rm -rf $TEST_DATA_DIR
        exit 1
    fi
fi

cd $TEST_DATA_DIR
if [ "$TEST_ALL_PARSERS" = "yes" ];then
    TEST_ARGS=""
else
    TEST_ARGS="--filter=libujson-only.json"
fi
echo "# "
echo "# Running: python3 run_tests.py $TEST_ARGS" >&2
echo "# "
if ! python3 run_tests.py $TEST_ARGS; then
    echo "# "
    echo "# Error: JSON parser test suite failed"
    echo "# "
    exit 1
fi
cd ../..

mkdir -p $TEST_RESULT_DIR
cp $TEST_DATA_DIR/results/* $TEST_RESULT_DIR/
echo "# "
echo "# Parsing test result for libujson, see parsing.html and parsing_pruned.html in directory $TEST_RESULT_DIR"
echo "# "
echo "ls -l $TEST_RESULT_DIR"
ls -l $TEST_RESULT_DIR
