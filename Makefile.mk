CC = gcc
CFLAGS = -g -Wall

all: test_assign1_1

test_assign1_1: test_assign1_1.c storage_mgr.c dberror.c
	$(CC) $(CFLAGS) -o test_assign1_1 test_assign1_1.c storage_mgr.c dberror.c -I.

clean:
	rm -f test_assign1_1
