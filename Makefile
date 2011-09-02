SRC=t0.c image.c
OUT=tmp

# Configs
CC=gcc
RM=rm
MV=mv
CFLAGS=-O2 -Wshadow -Wall `pkg-config gl --cflags` -I /usr/include/iup
LIBS=-l iup -l iupgl -l iupimglib

MAKEFILE=Makefile
OBJ=$(SRC:.c=.o)

.c.o:
	$(CC) -c $(CFLAGS) $<

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

clean:
	$(RM) $(OBJ) $(OUT)

depend:
	if grep '^# DO NOT DELETE' $(MAKEFILE) >/dev/null; \
	then \
		sed -e '/^# DO NOT DELETE/,$$d' $(MAKEFILE) > \
			$(MAKEFILE).$$$$ && \
		$(MV) $(MAKEFILE).$$$$ $(MAKEFILE); \
	fi
	echo '# DO NOT DELETE THIS LINE -- make depend depends on it.' \
		>> $(MAKEFILE); \
	$(CC) -M $(SRC) >> $(MAKEFILE)
