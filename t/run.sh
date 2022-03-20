#!/bin/sh

# TODO: make this more robust

if [ `whoami` != "root" ]; then
	echo "Must be run as root, restarting (sudo $0)"
	sudo "$0" "$@"
	exit $?
fi

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

KEYD_NAME="keyd test device" \
KEYD_DEBUG=1 \
KEYD_CONFIG_DIR="$tmpdir" \
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
