PROGRAM := a.out
SOURCES := $(wildcard ./*.cpp)
SOURCES += $(wildcard ./src/*.cpp)
OBJS    := $(patsubst %.cpp,%.o,$(SOURCES))

CC      := g++
CFLAGS  := -ggdb -Wall 
INCLUDE := -I .
LIB     := -lpthread -ldl 


.PHONY: clean install

$(PROGRAM): $(OBJS)
	$(CC) -o $@ $^ $(LIB)
%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDE)

clean:
	rm $(OBJS) $(PROGRAM) -f

install: $(PROGRAM)
	cp $(PROGRAM) ./bin/
	
