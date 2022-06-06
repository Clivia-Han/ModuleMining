#ifndef SETTINGS_H_
#define SETTINGS_H_


class Settings
{
public:
	static int maxNumberOfSamples;
	static int minNumberOfSamples;

	static bool divideBigTasks;
    constexpr static double validsPerTaskMargin = 0.8;
	static bool debugMSG;
	static long postponeNodesAfterIterations;
	static int maxSubgraphSize;
	static int maxNumNodes;
	static bool usePredictedInvColumn;

	static int fixedNumSubtasks;
    static double minImbalance;
    static int givenType;
};

#endif /* SETTINGS_H_ */
