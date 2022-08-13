#!/bin/sh

# TODO: make this more robust

if [ `whoami` != "root" ]; then
	echo "Must be run as root, restarting (sudo $0)"
	sudo "$0" "$@"
	exit $?
fi

pgrep keyd && { echo "Stop keyd before running tests"; exit -1; }

tmpdir=$(mktemp -d)

cleanup() {
	rm -rf "$tmpdir"
	kill $pid

	trap - EXIT
	exit
}

trap cleanup INT

cd "$(dirname "$0")"
cp test.conf "$tmpdir"

(cd ..;make CONFIG_DIR="$tmpdir") || exit -1
../bin/keyd > test.log 2>&1 &

pid=$!

sleep .7s
if [ $# -ne 0 ]; then
	test_files="$(echo "$@"|sed -e 's/ /.t /g').t"
	./runner.py -v $test_files
	cleanup
fi

./runner.py -ev *.t
cleanup
