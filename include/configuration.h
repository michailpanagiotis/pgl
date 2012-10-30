#ifndef CONFIGURATION_H
#define CONFIGURATION_H

typedef unsigned int PriorityQueueSizeType;

//-------------------------------- PRIORITY QUEUE STATISTICS --------------------------------

//queue statistics (can be set by compilerflag -DQUEUESTATS)
#ifdef QUEUESTATS
	#define _QUEUESTATS(x) x
#else
	#define _QUEUESTATS(x)
#endif


#ifdef MEMSTATS
	#define _MEMSTATS(x) x
#else
	#define _MEMSTATS(x)
#endif


#ifdef SHOWPROGRESS
    #define _SHOWPROGRESS(x) x
#else
	#define _SHOWPROGRESS(x)
#endif



static std::string nodeMemTransfersFile = "/home/michai/Projects/pgl/ResultGenerators/dijkstra/nodestats.csv";
static std::string edgeMemTransfersFile = "/home/michai/Projects/pgl/ResultGenerators/dijkstra/edgestats.csv";

static std::string queueStatsFile = "/home/michai/Projects/pgl/include/CorrectnessCheckers/priorityQueue/stats.csv";


#endif //CONFIGURATION_H
