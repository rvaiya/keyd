#!/bin/sh

# TODO: make this more robust

sudo cp test.conf /etc/keyd
sudo pkill keyd
sleep 1s

cd $(dirname $0)
sudo ../bin/keyd -d
if [ $? -ne 0 ]; then
	echo "Failed to start keyd"
	sudo systemctl restart keyd
	exit -1
fi
sleep 1s

sudo ./runner.py -v *.t
sudo ./runner.py -ev $(seq 100|sed -e 's@.*@overload-timeout.t@')
sudo pkill keyd
sleep 1s
sudo systemctl restart keyd
