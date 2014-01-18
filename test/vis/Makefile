obj = test.o
bin = test

font = linux-libertine.ttf

CC = gcc
CFLAGS = -pedantic -Wall -g -I../src -I/usr/local/include
LDFLAGS = -L.. -L/usr/local/lib $(libgl) -lrbtree -ldrawtext -lm

ifeq ($(shell uname -s), Darwin)
	libgl = -framework OpenGL -framework GLUT
else
	libgl = -lGL -lglut
endif

.PHONY: all
all: $(bin) $(font)

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

$(font):
	wget http://downloads.sourceforge.net/project/linuxlibertine/linuxlibertine/5.1.3-2/LinLibertineTTF_5.1.3_2011_06_21.tgz
	mkdir -p linlibertine
	cd linlibertine; tar xzvf ../LinLibertineTTF_5.1.3_2011_06_21.tgz
	rm -f LinLibertineTTF_5.1.3_2011_06_21.tgz
	cp linlibertine/LinLibertine_R.ttf $@


.PHONY: clean
clean:
	rm -f $(obj) $(bin)
