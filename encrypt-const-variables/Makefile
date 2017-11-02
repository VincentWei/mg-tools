CPPFLAGS=-g -Wall

all:	encrypt-const gen-table

encrypt-const:	encrypt-const.c
	$(CC) -o $@ $^ $(CPPFLAGS)

gen-table:	gen-table.c
	$(CC) -o $@ $^ $(CPPFLAGS)

clean:
	rm -f encrypt-const gen-table
