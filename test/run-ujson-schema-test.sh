#!/bin/sh
#
# Copyright (C) 2023,2024 Dan Arrhenius <dan@ultramarin.se>
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
    BASE_DIR="."
else
    BASE_DIR="$BASE_DIR"
fi

SCHEMA_VERSION=draft2020-12
REPO_URL=https://github.com/json-schema-org/JSON-Schema-Test-Suite.git
SHEMA_SPEC_REPO_BRANCH=2020-12
SHEMA_SPEC_REPO_URL=https://github.com/json-schema-org/json-schema-spec.git


UJSON_UTILS_DIR=$BASE_DIR/../utils
UJSON_GET=$UJSON_UTILS_DIR/ujson-get
UJSON_PRINT=$UJSON_UTILS_DIR/ujson-print
UJSON_TOOL=$UJSON_UTILS_DIR/ujson-tool


TEST_DATA_BASE_DIR=$BASE_DIR/test-data
TEST_REPO_DIR=$TEST_DATA_BASE_DIR/JSON-Schema-Test-Suite
SCHEMA_SPEC_DIR=$TEST_DATA_BASE_DIR/json-schema-spec
TEST_RESULT_DIR=$BASE_DIR/result-schema-test

TEST_DIR=$TEST_REPO_DIR/tests/$SCHEMA_VERSION
REMOTE_DIR=$TEST_REPO_DIR/remotes

SCHEMA_FILE=$TEST_RESULT_DIR/tmp-test-schema.json
INSTANCE_FILE=$TEST_RESULT_DIR/tmp-test-instance.json
RESULT_FILE=$TEST_RESULT_DIR/tmp-test-result.json
ERROR_FILE=$TEST_RESULT_DIR/tmp-test-error.json

LOAD_META_SCHEMAS=no
FULL_VALIDATION=no
VERBOSE=no
DEBUG=no
ARGS=""


download_test_data() {
    #
    # Download test and install data
    #
    if ! [ -r $TEST_REPO_DIR/README.md ]; then
        echo "# "
        echo "# Clone git repository $REPO_URL" >&2
        echo "# "
        rm -rf $TEST_REPO_DIR
        mkdir -p $TEST_DATA_BASE_DIR
        if ! git clone --depth=1 $REPO_URL $TEST_REPO_DIR; then
            echo "# "
            echo "# Error: Unable to clone git repository $REPO_URL"
            echo "# "
            exit 1
        fi
    fi
    if ! [ -r $SCHEMA_SPEC_DIR/README.md ]; then
        echo "# "
        echo "# Clone git repository $REPO_URL" >&2
        echo "# "
        rm -rf $SCHEMA_SPEC_DIR
        mkdir -p $SCHEMA_SPEC_DIR
        if ! git clone --depth=1 --branch=$SHEMA_SPEC_REPO_BRANCH $SHEMA_SPEC_REPO_URL $SCHEMA_SPEC_DIR; then
            echo "# "
            echo "# Error: Unable to clone git repository $REPO_URL"
            echo "# "
            exit 1
        fi
    fi
}

initialize_test() {
    if ! [ -d $TEST_RESULT_DIR ]; then
        mkdir -p $TEST_RESULT_DIR
    fi
    download_test_data
}


print_help() {
    echo "Usage:"
    echo "    run-ujson-schema-test.sh ls"
    echo "        List JSON test files."
    echo ""
    echo "    run-ujson-schema-test.sh clean"
    echo "        Remove test data and result files."
    echo ""
    echo "    run-ujson-schema-test.sh [-v|-m] all"
    echo "        -f    Validate all values in a JSON instance even if some fails validation."
    echo "        -v    Verbose (print test descriptions)"
    echo "        -m    Also load meta schemas"
    echo "        Test everything."
    echo ""
    echo "    run-ujson-schema-test.sh num-schemas <test-file>"
    echo "        Print the numbers of schema tests in a test file."
    echo ""
    echo "    run-ujson-schema-test.sh print-schema <test-file> <schema-index>"
    echo "        Print the schema at a specified index in a test file."
    echo ""
    echo "    run-ujson-schema-test.sh num-tests <test-file> <schema-index>"
    echo "        Print the numbers of tests for a specific schema."
    echo ""
    echo "    run-ujson-schema-test.sh print-test <test-file> <schema-index> [test-index]"
    echo "        Print the specified test, or all tests if no index."
    echo ""
    echo "    run-ujson-schema-test.sh describe-test <test-file> <schema-index> <test-index>"
    echo "        Show the description, schema and test."
    echo ""
    echo "    run-ujson-schema-test.sh  [-v|-d|-m] test <test-file> <schema-index> <test-index>"
    echo "        -f    Validate all values in a JSON instance even if some fails validation."
    echo "        -v    Verbose (print test descriptions)"
    echo "        -d    Debug (print standard output even if no error)"
    echo "        -m    Also load meta schemas"
    echo "        Do a test."
    echo ""
}


cleanup() {
    if ! [ $DEBUG = "yes" ]; then
        rm -f $SCHEMA_FILE
        rm -f $TEST_FILE
        rm -f $INSTANCE_FILE
        rm -f $RESULT_FILE
        rm -f $ERROR_FILE
    fi
    echo ""
}


make_test() {
    local SCHEMA_INDEX=$1
    local TEST_INDEX=$2

    local SCHEMA_NUMBER=$3
    local TOTAL_SCHEMAS=$4

    local TEST_NUMBER=$5
    local TOTAL_TESTS=$6

    local RESULT_TXT=""
    local RETVAL=0

    if [ -n "$TOTAL_TESTS" ]; then
        echo "Testing schema $SCHEMA_NUMBER of $TOTAL_SCHEMAS, test $TEST_NUMBER of $TOTAL_TESTS"
    else
        echo "Test case:"
    fi
    echo ""
    echo "Schema: `$UJSON_GET -ru $FILE /$SCHEMA_INDEX/description 2>/dev/null`"
    if ! $UJSON_GET -r $FILE /$SCHEMA_INDEX/schema >$SCHEMA_FILE 2>/dev/null; then
        echo "Error getting schema #$SCHEMA_INDEX in $2"
        cleanup
        exit 1
    fi
    $UJSON_PRINT -ao $SCHEMA_FILE
    echo ""
    echo "Instance: `$UJSON_GET -ru $FILE /$SCHEMA_INDEX/tests/$TEST_INDEX/description 2>/dev/null`"
    if ! $UJSON_GET -r $FILE /$SCHEMA_INDEX/tests/$TEST_INDEX/data >$INSTANCE_FILE 2>/dev/null; then
        echo "Error getting test #$TEST_INDEX for schema #$SCHEMA_INDEX in $2"
        cleanup
        exit 1
    fi
    $UJSON_PRINT -ao $INSTANCE_FILE
    echo ""
    EXPECTED_RESULT=`$UJSON_GET -r $FILE /$SCHEMA_INDEX/tests/$TEST_INDEX/valid`
    echo "Expected validity: $EXPECTED_RESULT"
    echo ""
    echo "Result:"

    FULL_ARG=""
    if [ "$FULL_VALIDATION" = "yes" ]; then
        FULL_ARG="-f"
    fi

    if [ "$LOAD_META_SCHEMAS" = "yes" ]; then
        $BASE_DIR/ujson-schema-test $FULL_ARG \
                                    -d $REMOTE_DIR $SCHEMA_FILE \
                                    $SCHEMA_SPEC_DIR/schema.json \
                                    $SCHEMA_SPEC_DIR/meta/applicator.json \
                                    $SCHEMA_SPEC_DIR/meta/content.json \
                                    $SCHEMA_SPEC_DIR/meta/core.json \
                                    $SCHEMA_SPEC_DIR/meta/format-assertion.json \
                                    $SCHEMA_SPEC_DIR/meta/format-annotation.json \
                                    $SCHEMA_SPEC_DIR/meta/meta-data.json \
                                    $SCHEMA_SPEC_DIR/meta/unevaluated.json \
                                    $SCHEMA_SPEC_DIR/meta/validation.json \
                                    $INSTANCE_FILE >$RESULT_FILE 2>$ERROR_FILE
    else
        $BASE_DIR/ujson-schema-test $FULL_ARG -d $REMOTE_DIR $SCHEMA_FILE $INSTANCE_FILE >$RESULT_FILE 2>$ERROR_FILE
    fi
    if ! $UJSON_PRINT -ao $RESULT_FILE 2>/dev/null; then
        cat $RESULT_FILE
    fi


    if ! $UJSON_GET -r $RESULT_FILE /valid >/dev/null 2>&1; then
        echo "Error performing test"
        cleanup
        RESULT_TXT="ERROR"
        RETVAL=2
    else
        ACTUAL_RESULT=`$UJSON_GET -r $RESULT_FILE /valid`
        echo ""
        cleanup
        if [ "$EXPECTED_RESULT" = "$ACTUAL_RESULT" ]; then
            RESULT_TXT="OK"
            RETVAL=0
        else
            RESULT_TXT="FAILED"
            RETVAL=1
        fi
    fi

    if [ -n "$TOTAL_TESTS" ]; then
        echo "Schema $SCHEMA_NUMBER/$TOTAL_SCHEMAS, test $TEST_NUMBER/$TOTAL_TESTS: $RESULT_TXT"
    else
        echo "Test $RESULT_TXT"
    fi

    if [ $DEBUG = "yes" ]; then
        echo "== D E B U G ==================================================================="
        cat $ERROR_FILE
        echo "================================================================================"
    fi

    return $RETVAL
}


make_all_tests_in_schema() {
    local TOTAL_SCHEMAS=1
    local SCHEMA_INDEX=$1

    local TOTAL_TESTS=0
    local TEST_INDEX=0

    local RESULT_TXT=""
    local RETVAL=0

    if [ -z "$SCHEMA_INDEX" ]; then
        TOTAL_SCHEMAS=`$UJSON_TOOL -r size "$FILE" 2>/dev/null`
        SCHEMA_INDEX=0
        TOTAL_SCHEMAS=$(( ${TOTAL_SCHEMAS}-1 ))
    else
        TOTAL_SCHEMAS=$SCHEMA_INDEX
    fi

    for SCHEMA_INDEX in `seq $SCHEMA_INDEX $TOTAL_SCHEMAS`; do
        TOTAL_TESTS=`$UJSON_TOOL -rp /$SCHEMA_INDEX/tests size "$FILE" 2>/dev/null`
        TOTAL_TESTS=$(( ${TOTAL_TESTS}-1 ))

        if [ $VERBOSE = "yes" ]; then
            SCHEMA_DESC="`$UJSON_GET -ru $FILE /$SCHEMA_INDEX/description 2>/dev/null`"
        fi

        for TEST_INDEX in `seq 0 $TOTAL_TESTS`; do
            if [ $VERBOSE = "yes" ]; then
                TEST_DESC="`$UJSON_GET -ru $FILE /$SCHEMA_INDEX/tests/$TEST_INDEX/description 2>/dev/null`"
            fi

            make_test $SCHEMA_INDEX $TEST_INDEX >/dev/null 2>&1
            RETVAL=$?
            if [ "$RETVAL" = "0" ]; then
                RESULT_TXT="OK    "
            elif [ "$RETVAL" = "1" ]; then
                RESULT_TXT="FAILED"
            else
                RESULT_TXT="ERROR"
                RETVAL=1
            fi

            if [ $VERBOSE = "yes" ]; then
                echo "Schema $SCHEMA_INDEX/$TOTAL_SCHEMAS, \ttest $TEST_INDEX/$TOTAL_TESTS: \t$RESULT_TXT \t$SCHEMA_DESC - $TEST_DESC"
            else
                echo "Schema $SCHEMA_INDEX/$TOTAL_SCHEMAS, \ttest $TEST_INDEX/$TOTAL_TESTS: \t$RESULT_TXT"
            fi

        done
    done

    return $RETVAL
}


#
# Get options
#
DONE=false
while [ "$DONE" = "false" ]; do
    if [ "$1" = "-v" ]; then
        VERBOSE=yes
        ARGS="$ARGS -v"
        shift
    elif [ "$1" = "-f" ]; then
        FULL_VALIDATION=yes
        ARGS="$ARGS -f"
        shift
    elif [ "$1" = "-d" ]; then
        DEBUG=yes
        ARGS="$ARGS -d"
        shift
    elif [ "$1" = "-m" ]; then
        LOAD_META_SCHEMAS=yes
        ARGS="$ARGS -m"
        shift
    else
        DONE=true
    fi
done


#
# Get arguments
#
CMD=$1
FILE=$TEST_DIR/$2
if ! [ -r $FILE ]; then
    FILE=$TEST_DIR/$2.json
fi
SCHEMA_INDEX=$3
TEST_INDEX=$4




case "$CMD" in
    ls)
        initialize_test
        echo "TEST_DIR: $TEST_DIR"
        ls $TEST_DIR
        ;;

    all)
        initialize_test
        TESTS=`ls $TEST_DIR/*.json`
        for FILE in $TESTS; do
            TEST=`basename $FILE`
            echo "Testing $TEST"
            $BASE_DIR/run-ujson-schema-test.sh $ARGS test $TEST
            echo ""
        done
        ;;

    clean)
        rm -rf $TEST_REPO_DIR
        rm -rf $TEST_RESULT_DIR
        rmdir $TEST_DATA_BASE_DIR >/dev/null 2>&1
        ;;

    cat)
        initialize_test
        cat $FILE
        ;;

    more)
        initialize_test
        more $FILE
        ;;

    num-schemas)
        initialize_test
        $UJSON_TOOL -r size $FILE
        ;;

    print-schema)
        initialize_test
        $UJSON_GET -r $FILE /$SCHEMA_INDEX/schema
        ;;

    num-tests)
        initialize_test
        $UJSON_TOOL -rp /$SCHEMA_INDEX/tests size $FILE
        ;;

    print-test)
        initialize_test
        if [ -n "$TEST_INDEX" ]; then
            $UJSON_GET -r $FILE /$SCHEMA_INDEX/tests/$TEST_INDEX
        else
            $UJSON_GET -r $FILE /$SCHEMA_INDEX/tests
        fi
        ;;

    describe-test)
        initialize_test
        echo "Schema: `$UJSON_GET -ru $FILE /$SCHEMA_INDEX/description`"
        $UJSON_GET -r $FILE /$SCHEMA_INDEX/schema
        echo ""
        echo "Test: `$UJSON_GET -ru $FILE /$SCHEMA_INDEX/tests/$TEST_INDEX/description`"
        $UJSON_GET -r $FILE /$SCHEMA_INDEX/tests/$TEST_INDEX/data
        echo ""
        echo "Expected validity: `$UJSON_GET -r $FILE /$SCHEMA_INDEX/tests/$TEST_INDEX/valid`"
        ;;

    test)
        initialize_test
        if [ -n "$TEST_INDEX" ]; then
            TOTAL_SCHEMAS=`$UJSON_TOOL -r size "$FILE" 2>/dev/null`
            if [ -z "$TOTAL_SCHEMAS" ]; then
                echo "Error: No schemas to test in file $FILE" >&2
                exit 1
            fi
            TOTAL_SCHEMAS=$(( ${TOTAL_SCHEMAS}-1 ))
            if [ $SCHEMA_INDEX -gt $TOTAL_SCHEMAS ]; then
                echo "Error: Schema index too big" >&2
                exit 1
            fi

            TOTAL_TESTS=`$UJSON_TOOL -rp /$SCHEMA_INDEX/tests size "$FILE" 2>/dev/null`
            if [ -z "$TOTAL_TESTS" ]; then
                TOTAL_TESTS=0
            fi
            TOTAL_TESTS=$(( ${TOTAL_TESTS}-1 ))
            if [ $TEST_INDEX -gt $TOTAL_TESTS ]; then
                echo "Error: Test index too big" >&2
                exit 1
            fi

            make_test $SCHEMA_INDEX $TEST_INDEX $SCHEMA_INDEX $TOTAL_SCHEMAS $TEST_INDEX $TOTAL_TESTS
            exit $?
        else
            make_all_tests_in_schema $SCHEMA_INDEX
            exit $?
        fi
        ;;

    "-h"|"--help")
        print_help
        ;;

    *)
        FULL_ARG=""
        if [ "$FULL_VALIDATION" = "yes" ]; then
            FULL_ARG="-f"
        fi

        initialize_test
        rm -f $TEST_RESULT_DIR/test-schema-result.txt
        TESTS=`ls $TEST_DIR/*.json`
        for FILE in $TESTS; do
            TEST=`basename $FILE`
            if [ -n "$FULL_ARG" ]; then
               echo "Testing full validation: $TEST" 2>&1 | tee -a $TEST_RESULT_DIR/test-schema-result.txt
            else
               echo "Testing validation: $TEST" 2>&1 | tee -a $TEST_RESULT_DIR/test-schema-result.txt
            fi
            $BASE_DIR/run-ujson-schema-test.sh $FULL_ARG -v -m test $TEST 2>&1 | tee -a $TEST_RESULT_DIR/test-schema-result.txt
            echo "" | tee -a $TEST_RESULT_DIR/test-schema-result.txt
        done
        echo ""
        echo "Test result logged to file $TEST_RESULT_DIR/test-schema-result.txt"
        echo ""
        ;;
esac
