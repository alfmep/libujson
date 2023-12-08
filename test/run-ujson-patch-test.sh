#!/bin/sh
#
# Copyright (C) 2022,2023 Dan Arrhenius <dan@ultramarin.se>
#
# This file is part of ujson.
#
# ujson is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

BASE_DIR=$(dirname $(readlink -f "$0"))
PWD_DIR=`pwd`

if [ "$BASE_DIR" = "$PWD_DIR" ]; then
    BASE_DIR=""
else
    BASE_DIR="$BASE_DIR/"
fi

TEST_APP_NAME=ujson-patch-test
if [ -z "$BASE_DIR" ]; then
    TEST_APP=./$TEST_APP_NAME
else
    TEST_APP=${BASE_DIR}$TEST_APP_NAME
fi

TEST_RESULT_DIR=${BASE_DIR}result-patch-test
TEST_DATA_BASE_DIR=${BASE_DIR}test-data
TEST_DATA_DIR=$TEST_DATA_BASE_DIR/json-patch-tests

GIT_REPO_TEST_FILE=$TEST_DATA_DIR/tests.json
TEST_FILE=$TEST_RESULT_DIR/tests.json

PASSED_FILE=$TEST_RESULT_DIR/passed.json
FAILED_FILE=$TEST_RESULT_DIR/failed.json
DISABLED_FILE=$TEST_RESULT_DIR/disabled.json
INVALID_FILE=$TEST_RESULT_DIR/invalid.json

CLONE_URL=https://github.com/json-patch/json-patch-tests.git

#
# Option '--clean' will erase test data and exit
#
if [ "$1" = "-a" -o "$1" = "--allow-disabled" ]; then
    TEST_APP="$TEST_APP --allow-disabled"
elif [ "$1" = "-c" -o "$1" = "--clean" ]; then
    echo "# "
    echo "# Removing patch test data (directory $TEST_DATA_DIR)"
    echo "# Removing patch result data (directory $TEST_RESULT_DIR)"
    echo "# "
    rm -rf $TEST_RESULT_DIR
    rm -rf $TEST_DATA_DIR
    rmdir $TEST_DATA_BASE_DIR >/dev/null 2>&1
    exit 0
elif [ "$1" = "-h" -o "$1" = "--help" ]; then
    echo ""
    echo "Usage: `basename $0` [OPTION]"
    echo "    Download test data for testing JSON patches and run test application '$TEST_APP'."
    echo "    Test data is cloned from $CLONE_URL,"
    echo "    and installed in directory '$TEST_DATA_DIR'"
    echo ""
    echo "    Patch test file and result files are stored in directory '$TEST_RESULT_DIR'"
    echo ""
    echo "    Options:"
    echo "        -a,--allow-disabled    Perform a patch test even if it is marked as disabled."
    echo "        -c,--clean             Erase test data directory and test output files"
    echo "        -h,--help           Print this help and exit"
    echo ""
    exit 0
fi

#
# Download test and install data for test application ujson-patch-test
#
if ! [ -r $TEST_FILE ]; then
    if ! [ -r $GIT_REPO_TEST_FILE ]; then
        echo "# "
        echo "# Clone git repository $CLONE_URL" >&2
        echo "# "
        rm -rf $TEST_DATA_DIR
        mkdir -p $TEST_DATA_BASE_DIR
        if ! git clone --depth=1 $CLONE_URL $TEST_DATA_DIR; then
            echo "# "
            echo "# Error: Unable to clone git repository $CLONE_URL"
            echo "# "
            exit 1
        fi
    fi
    echo "# "
    echo "# Copy test file to '$TEST_FILE'" >&2
    echo "# "
    mkdir -p $TEST_RESULT_DIR
    echo "cp $GIT_REPO_TEST_FILE $TEST_FILE"
    if ! cp $GIT_REPO_TEST_FILE $TEST_FILE; then
        echo "# "
        echo "# Error: Unable to copy test file '$GIT_REPO_TEST_FILE'"
        echo "# "
        exit 1
    fi
fi


#
# Run test application
#
echo "# "
echo "# Run JSON patch test application:"
echo "# $TEST_APP -o -s $PASSED_FILE -f $FAILED_FILE -d $DISABLED_FILE -i $INVALID_FILE $TEST_FILE"
echo "# Results stored in directory '$TEST_RESULT_DIR'"
echo "# "
mkdir -p $TEST_RESULT_DIR
if ! $TEST_APP -o -s $PASSED_FILE -f $FAILED_FILE -d $DISABLED_FILE -i $INVALID_FILE $TEST_FILE; then
    echo "# "
    echo "# Error: $TEST_APP_NAME exited with error"
    echo "# "
    exit 1
fi
