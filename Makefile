target = libstore.a
srcs = store.cpp singleDb.cpp redisImp.cpp address.cpp

libs = 

subdirs=
INCLUDE= -I./3rd-libs/include/
LIBRARY= 
CPPFLAGS=-g -O2 -Wall
all : $(target)

objs=$(patsubst %.cpp, %.o, $(srcs))

$(target) : $(objs) 
	for dir in $(subdirs); do make -C $$dir ; done
	ar cr $(target) $^ $(libs)
#g++ $(CPPFLAGS) $(INCLUDE) -o $(target) $^ $(libs) $(LIBRARY) 

%.o : %.cpp
	g++ $(INCLUDE) $(CPPFLAGS) $< -c -o $@

.PHONY : clean sclean
clean:
	for dir in $(subdirs); do make -C $$dir clean; done
	rm -f $(objs)
	rm -f $(target)
