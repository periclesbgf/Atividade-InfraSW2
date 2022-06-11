# -*- Makefile -*-

vm: pbgf.c
	gcc $< -o $@

clean:
	rm vm

testfifofifo:
	./vm addresses.txt fifo fifo
testfifolru:
	./vm addresses.txt fifo lru
testlrufifo:
	./vm addresses.txt lru fifo
testlrulru:
	./vm addresses.txt lru lru
