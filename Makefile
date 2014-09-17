CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS =		assignment1.o runnable.o server.o helper.o client.o

LIBS = 

DEPS = runnable.h host.h

TARGET =	assignment1

IDIR = -I./inc

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS) $(IDIR)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
