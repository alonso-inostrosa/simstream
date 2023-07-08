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

Node specs file:
#First, it needs the following data no matter the type of node:
## Node_Name: the name of a node from the topology
## Type: S for a spout or source of data for the topology (needs at least one), or B for a  bolt or processing element.
## Replication: the number of replicas for the node.
## Grouping: The grouping type for the tuples.
## Processor: IP Address of the simulated machine hosting the node.

#In the case of a SPOUT node, or a source of data for the topology, the fields that follow are:
## Arrival Rate: a string name of a statistic distribution for random number generation of the formArrivalRate(STRING)
## Average Service time: a string name of a statistic distribution for random number generation of the form AvgServiceTimeFunction(STRING) to represent the average service time for the node.

#In the case of a BOLT node, or a processing element of the topology, the fields that follow are:
##Bolt Type: the type of processing the node does to each tuple it processes. Bolt Type can be: Normal=0 (each tuple produces an output), Filter=1 (some tuples produce output), Splitter=2 (produces output for each connected node), Final=3 (produces no output).
## Average Service time: a string name of a statistic distribution for random number generation of the form AvgServiceTimeFunction(STRING) to represent the average service time for the node.

#Example file:
#Node_Name      Type    Replication     Grouping        Processor       AvgServiceTime(STRING)  Arrival Rate(STRING)
#SPOUTS
Spout_0 S       1	0	10.0.0.2	0.0001	10
Spout_1 S       1	0	10.0.0.2	0.0001	15
#BOLT
Bolt_0	B	1	0	10.0.0.3	0	0.012
Bolt_1	B	1	0	10.0.0.3	0	0.0213
Bolt_2	B	1	0	10.0.0.3	3	0.214
