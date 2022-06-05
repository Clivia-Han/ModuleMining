#ifndef SETTINGS_H_
#define SETTINGS_H_


class Settings
{
public:
	static bool useSearchSpacePrediction;
	static int maxNumberOfSamples;
	static int minNumberOfSamples;

	static const bool fullCount = true;
	static bool divideBigTasks;
    constexpr static double validsPerTaskMargin = 0.8;
	static bool debugMSG;
	static long postponeNodesAfterIterations;
	static int maxSubgraphSize;
	static int maxNumNodes;
	static bool usePredictedInvColumn;
	static bool smartBreak;

	static int fixedNumSubtasks;
	static bool showNumCandids;//mainly used for showing how the number of candidates increase as the time goes
	static double minImbalance;
    static int givenType;
};

#endif /* SETTINGS_H_ */
