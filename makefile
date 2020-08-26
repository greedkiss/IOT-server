TARGET = server
FILES = $(wildcard *.c)
OBJS = $(addprefix obj/, $(patsubst %.c, %.o, $(FILES)))
CFLAGS = -lpthread -L/usr/lib/mysql -lmysqlclient


$(TARGET):$(OBJS)
	rm -f $(TARGET)
	gcc -g -o $(TARGET) $(OBJS) $(CFLAGS) 

obj/%.o:%.c
	@if test ! -e obj; then\
		mkdir obj;\
	fi;
	gcc -c -o $@ $<

clean:
	rm -rf obj
	rm -f $(TARGET)