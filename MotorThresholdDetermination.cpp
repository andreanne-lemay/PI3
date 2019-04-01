#include <vector>
#include <stdlib.h>
#include <iostream>
#include <math.h>

#include "MotorThresholdDetermination.h"


MotorThresholdDetermination::MotorThresholdDetermination()
{
    _numberOfPulses = 0;
    _motorThreshold = 0;
    _confidenceInterval = 0;
    _priorMean = 50;
    _priorStandardDeviation = 8;
    _MEPResults.push_back(false);
    _MEPResults.push_back(true);
    _powerCandidates.push_back(0.1);
    _powerCandidates.push_back(100);
    _priorFunction = std::vector<double>(kPowerArraySize, 0.0);
    _likelihoodFunction = std::vector<double>(kPowerArraySize, 1.0);
    _posteriorFunction = std::vector<double>(kPowerArraySize, 0.0);


    for (size_t idx = 0; idx < kPowerArraySize; idx++)
    {
        _priorFunction[idx] = (exp(-pow(0.1*idx - _priorMean, 2) / (2 * pow(_priorStandardDeviation,2))) / (_priorStandardDeviation * sqrt(2*M_PI)));
    }
}

MotorThresholdDetermination::~MotorThresholdDetermination()
{
    //dtor
}

std::vector<double> MotorThresholdDetermination::GetCumulativeProbabilityFunction(double inMT)
{
    std::vector<double> cumulativeGaussian(kPowerArraySize);
    double x[kPowerArraySize] = {};
    double s = 0.07 * inMT;

    for (size_t idx = 0; idx < kPowerArraySize; idx++)
    {
        x[idx] = 0.1 * idx;
        cumulativeGaussian[idx] = 0.5 * erfc(-(x[idx] - inMT)/(s*sqrt(2)));
    }

    return cumulativeGaussian;
}

void MotorThresholdDetermination::UpdateMotorThresholdCandidate()
{
    double logLikelihoodFunction[kPowerArraySize] = {};

    uint16_t n = _MEPResults.size();

    for (size_t idxCandidate = 0; idxCandidate < n; idxCandidate++)
    {
        std::vector<double> MEPProbabilityFunction = this->GetCumulativeProbabilityFunction(_powerCandidates[idxCandidate]);

        for (size_t idx = 0; idx < kPowerArraySize; idx++)
        {
            if (_MEPResults[idxCandidate])
            {
                logLikelihoodFunction[idx] += log(1 - MEPProbabilityFunction[idx]);
            }

            else
            {
                logLikelihoodFunction[idx] += log(MEPProbabilityFunction[idx]);
            }
        }
    }

    double x[kPowerArraySize] = {};

    uint16_t powerCandidateIdx = 0;
    double functionMaximum = logLikelihoodFunction[powerCandidateIdx];

    for (size_t idx = 0; idx < kPowerArraySize; idx++)
    {
        x[idx] = 0.1 * idx;
        logLikelihoodFunction[idx] -= pow(x[idx] - _priorMean, 2) / (2 * pow(_priorStandardDeviation, 2));

        if (logLikelihoodFunction[idx] > functionMaximum)
        {
            powerCandidateIdx = idx;
            functionMaximum = logLikelihoodFunction[idx];
        }
    }

    _motorThreshold = powerCandidateIdx / 10.0;
}

double MotorThresholdDetermination::GetMotorThresholdCandidate()
{
    return _motorThreshold;
}

void MotorThresholdDetermination::UpdateLikelihoodFunction()
{
    uint16_t n = _MEPResults.size();

   std::fill(_likelihoodFunction.begin(), _likelihoodFunction.end(), 1.0);

    for (size_t idxCandidate = 0; idxCandidate < n; idxCandidate++)
    {
        std::vector<double> powerCPF = this->GetCumulativeProbabilityFunction(_powerCandidates[idxCandidate]);


        if (_MEPResults[idxCandidate])
        {
            for (size_t idx = 0; idx < kPowerArraySize; idx++)
            {
                _likelihoodFunction[idx] *= (1 - powerCPF[idx]);
            }
        }

        else
        {
            for (size_t idx = 0; idx < kPowerArraySize; idx++)
            {
                _likelihoodFunction[idx] *= powerCPF[idx];
            }
        }
    }

    double normalisationFactor = 0;
    for (size_t idx = 0; idx < kPowerArraySize - 1; idx++)
    {
        normalisationFactor += 0.1 * (_likelihoodFunction[idx] + _likelihoodFunction[idx + 1]) / (2);
    }

    for (size_t idx = 0; idx < kPowerArraySize; idx++)
    {
        _likelihoodFunction[idx] = _likelihoodFunction[idx]/normalisationFactor;
    }

}

void MotorThresholdDetermination::UpdateBayesianFunctions()
{
    double normalisationFactor = 0;
    this->UpdateLikelihoodFunction();

    for (size_t idx = 0; idx < kPowerArraySize; idx++)
    {
        normalisationFactor += _likelihoodFunction[idx] * _priorFunction[idx] * 0.1;
    }

    for (size_t idx = 0; idx < kPowerArraySize; idx++)
    {
        _posteriorFunction[idx] = _likelihoodFunction[idx] * _priorFunction[idx] / normalisationFactor;
    }

    this->UpdatePriorFunction();
    this->UpdateConfidenceInterval();
}

void MotorThresholdDetermination::GetPosteriorStatistics(double& outMean, double& outStandardDeviation)
{
    double numerator = 0;
    double denominator = 0;
    double numeratorSTD = 0;

    for (size_t idx = 0; idx < kPowerArraySize; idx++)
    {
        numerator += _posteriorFunction[idx] * 0.1 * idx;
        numeratorSTD += _posteriorFunction[idx] * pow((0.1 * idx), 2);
        denominator += _posteriorFunction[idx];
    }

    double squareMean = numeratorSTD / (denominator);
    outMean = numerator / (denominator);
    outStandardDeviation = sqrt(squareMean - pow(outMean,2));
}

void MotorThresholdDetermination::UpdatePriorFunction()
{
    for (size_t idx = 0; idx < kPowerArraySize; idx++)
    {
        _priorFunction[idx] = _posteriorFunction[idx];
    }

    double posteriorMean = 0;
    double posteriorStandardDeviation = 0;
    this->GetPosteriorStatistics(posteriorMean, posteriorStandardDeviation);
    _priorMean = posteriorMean;
    _priorStandardDeviation = posteriorStandardDeviation;
}

void MotorThresholdDetermination::UpdateConfidenceInterval()
{
    uint16_t minIdx = round(_priorMean*10) - 25;
    uint16_t maxIdx = round(_priorMean*10) + 25;

    if (minIdx < 0)
    {
        minIdx = 0;
    }

    if (maxIdx > 1000)
    {
        maxIdx = 1000;
    }

    _confidenceInterval = 0;

    for (size_t idx = minIdx; idx < maxIdx; idx++)
    {
        _confidenceInterval += 0.1 * (_posteriorFunction[idx] + _posteriorFunction[idx + 1]) / (2);
    }
}

void MotorThresholdDetermination::SetPriorMT(double inPriorMT)
{
    _priorMean = inPriorMT;
    for (size_t idx = 0; idx < kPowerArraySize; idx++)
    {
        _priorFunction[idx] = exp(-pow(0.1*idx - _priorMean, 2) / (2 * pow(_priorStandardDeviation,2))) / (_priorStandardDeviation * sqrt(2*M_PI));
    }
}

std::vector<double> MotorThresholdDetermination::GetLikelihoodFunction()
{
    return _likelihoodFunction;
}

std::vector<double> MotorThresholdDetermination::GetPriorFunction()
{
    return _priorFunction;
}

std::vector<double> MotorThresholdDetermination::GetPosteriorFunction()
{
    return _posteriorFunction;
}

double MotorThresholdDetermination::GetConfidenceInterval() const
{
    return _confidenceInterval;
}

uint16_t MotorThresholdDetermination::GetNumberOfPulses() const
{
    return _numberOfPulses;
}

void MotorThresholdDetermination::AddMEPResult(bool inMEPResult)
{
    _MEPResults.push_back(inMEPResult);
}

void MotorThresholdDetermination::AddPowerCandidate(double inPowerCandidate)
{
    _powerCandidates.push_back(inPowerCandidate);
}

bool MotorThresholdDetermination::MotorThresholdSimulation(double inMTCandidate, double inTrueMT)
{
    std::vector<double> MTModel = this->GetCumulativeProbabilityFunction(inTrueMT);
    uint16_t MTModelIdx = inMTCandidate * 10;
    double MEPProbability = MTModel[MTModelIdx];
    bool isMEP = (rand() % 1000) < round(MEPProbability*1000);
    _numberOfPulses++;
    return isMEP;
}
