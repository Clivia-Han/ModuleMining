/**
 * This class is for the event-driven tasks simulation
 */

#ifndef QUEUESIMULATOR_H_
#define QUEUESIMULATOR_H_


class QueueSimulator
{
public:
	static long simulate(long* times, int numTimes, int nWorkers);
	static double evaluate(long* times, int numTimes, int nWorkers, long threshold, long avgTimePerWorker);
};


#endif /* QUEUESIMULATOR_H_ */
