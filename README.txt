Requires the libcppsim library, it can be downloaded from https://www.moreno.marzolla.name/software/ or at: https://www.moreno.marzolla.name/software/libcppsim-0.2.5.tar.gz

To compile it needs the flag -std=c++11
./configure CPPFLAGS=-std=c++11
./make


To run it needs two files, the topologies and the specification of the nodes (spouts and bolts):
Topologies files:
It admits as many topologies as needed simultaneously, it consists of source-target pairs of nodes in each line, use a # symbol for comments:
#Each line consists of	Source	Target Spout/Bolts
#a line containing Spout_0 Bolt_0
#Means that Bolt_0 obtains its tuples (i.e. tweets, etc) from Spout_0
Spout_0 Bolt_0
Spout_1 Bolt_0
Bolt_0 Bolt_1
Bolt_0 Bolt_2
