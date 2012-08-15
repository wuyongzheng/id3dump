id3dump: id3dump.c
	gcc -Wall -o id3dump id3dump.c utf-util.c

.PHONY: clean
clean:
	rm -f id3dump
