target = test
srcs = test.cpp

libs = ../libhiredis.a

subdirs=
INCLUDE= -I../
LIBRARY= 
CPPFLAGS=-g -ggdb
all : $(target)

objs=$(patsubst %.c, %.o, $(srcs))

$(target) : $(objs) 
	for dir in $(subdirs); do make -C $$dir ; done
	g++ $(CPPFLAGS) $(INCLUDE) -o $(target) $^ $(libs) $(LIBRARY) 

%.o : %.c
	g++ $(INCLUDE) $(CPPFLAGS) $< -c -o $@

.PHONY : clean sclean
clean:
	for dir in $(subdirs); do make -C $$dir clean; done
	rm -f $(objs)
	rm -f $(target)
