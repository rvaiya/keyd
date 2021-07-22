.PHONY: clean all utils

all: clean utils
	-mkdir bin
	gcc -DGIT_COMMIT_HASH=\"$(shell git rev-parse HEAD)\" -DDEBUG -g -Wno-unused-parameter -Wall -Wextra -ludev src/*.c -o bin/keyd
man:
	pandoc -s -t man man.md | gzip > keyd.1.gz
clean:
	-rm -rf bin
install:
	-mkdir /etc/keyd
	-install -m755 keyd.service /etc/systemd/system/
	-install -m755 bin/keyd /usr/bin
	-install -m644 keyd.1.gz /usr/share/man/man1/
	-systemctl enable keyd
	-systemctl start keyd
