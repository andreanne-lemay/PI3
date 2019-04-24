#ifndef MOTORTHRESHOLDDETERMINATION_H
#define MOTORTHRESHOLDDETERMINATION_H

#include <vector>
#include <stdlib.h>
#include <iostream>

class MotorThresholdDetermination
{
    static const uint16_t kPowerArraySize = 1001;

    public:
        MotorThresholdDetermination();
        virtual ~MotorThresholdDetermination();

        double GetConfidenceInterval() const;
        void GetPosteriorStatistics(double& outMean, double& outStandardDeviation);
        std::vector<double> GetPosteriorFunction();
        std::vector<double> GetLikelihoodFunction();
        std::vector<double> GetPriorFunction();
        std::vector<double> GetMotorThresholdVector();
        void UpdateMotorThresholdCandidate();
        uint16_t GetNumberOfPulses() const;
        bool MotorThresholdSimulation(double inMTCandidate, double inTrueMT);
        void AddPowerCandidate(double inPowerCandidate);
        void AddMEPResult(bool inMEPResult);
        void UpdateBayesianFunctions();
        void SetPriorMT(double inPriorMT);
        double GetMotorThresholdCandidate();
        void ChangeAlgorithm(bool inIsPEST);
        std::vector<double> GetLogLikelihoodFunction();

    private:
        std::vector<double> GetCumulativeProbabilityFunction(double inMT);
        void UpdateLikelihoodFunction();
        void UpdateConfidenceInterval();
        void UpdatePriorFunction();

        double _numberOfPulses;
        double _motorThreshold;
        double _confidenceInterval;
        double _priorMean;
        double _priorStandardDeviation;
        bool _isPEST;
        std::vector<double> _logLikelihoodFunction;
        std::vector<bool> _MEPResults;
        std::vector<double> _powerCandidates;
        std::vector<double> _priorFunction;
        std::vector<double> _likelihoodFunction;
        std::vector<double> _posteriorFunction;

};

#endif // MOTORTHRESHOLDDETERMINATION_H
