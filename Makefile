TARGET=Simulador
CXX=g++
INCLUDES=-I../libcppsim-0.2.5/src/
LIBRARY=-L../libcppsim-0.2.5/src/
CFLAGS=-O3 -ggdb -Wall -lm $(INCLUDES) -g -lcppsim $(LIBRARY) -std=c++11

OBJS=obj/main.o obj/node.o obj/spout.o obj/bolt.o obj/processor.o obj/prob.o obj/core.o obj/net_iface.o obj/comm_switch.o obj/generator.o

$(TARGET): $(OBJS) 
	$(CXX) -o $@ $^ $(CFLAGS) $(LIBRARY) 

obj/main.o:main.cc
obj/node.o:node.cc node.h
obj/spout.o:spout.cc spout.h
obj/bolt.o:bolt.cc bolt.h
obj/processor.o:processor/processor.cc processor/processor.h
obj/core.o:processor/core.cc processor/core.h
obj/prob.o:distributions/prob.cc distributions/prob.h
obj/net_iface.o:network/net_iface.cc network/net_iface.h
obj/comm_switch.o:network/comm_switch.cc network/comm_switch.h
obj/generator.o:generator/generator.cc generator/generator.h

obj/%.o:
	$(CXX) -o $@ -c $< $(CFLAGS)

clean:
	${RM} $(TARGET)
	${RM} obj/*.o
	${RM} *~

all:
	make clean
	make
