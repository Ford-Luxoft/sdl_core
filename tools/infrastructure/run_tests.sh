#/bin/bash
# Copyright (c) 2019, Ford Motor Company
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following
# disclaimer in the documentation and/or other materials provided with the
# distribution.
#
# Neither the name of the Ford Motor Company nor the names of its contributors
# may be used to endorse or promote products derived from this software
# without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

run_test() {
    full_test_name="$1"
    cur_test="$2"
    test_count="$3"
    testname=${1##*/}
    fullpathname=${full_test_name%/*}
    cur_dir=$PWD
    echo "      Start $cur_test: $testname"

    cd $fullpathname
    test_output=`./$testname 2>&1`
    test_result=`echo $?`
    cd $cur_dir

    testname=`echo $testname | sed -r ':l;/.{40}/q;s/.*/&./;bl'`
    cur_test_test_count=`echo $cur_test/$test_count | sed -r ':l;/.{5}/q;s/.*/& /;bl'`

    if [[ $test_result -eq 0 ]]; then
        test_result_text="   Passed"
    else
        test_result_text="***Failed"
        test_result=1
        if [[ $4 == "-L" ]]; then
            echo "\n $test_output \n"
        fi
    fi
    echo "$cur_test_test_count Test #$cur_test: $testname $test_result_text"
}

run_all_tests() {
    tests=$(find $1 -name '*_test');
    test_count=`echo "$tests" | wc -l`
    cur_test=1
    failed_test_count=0

    for i in $tests; do
        run_test "$i" "$cur_test" "$test_count" "$2"
        if [[ $test_result -ne 0 ]]; then
            failed_tests="$failed_tests\n      $testname"
            (( failed_test_count+=1 ))
        fi
        (( cur_test+=1 ))
    done

    echo "\n   $failed_test_count tests failed out of $test_count"
    echo "$failed_tests \n"
}

dir=`pwd`

echo "Test project $dir"
run_all_tests "$dir" "$1"

exit $failed_test_count
