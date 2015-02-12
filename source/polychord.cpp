#ifdef COSMO_MPI
#include <mpi.h>
#endif

#include <fstream>
#include <sstream>
#include <cmath>
#include <iomanip>

#include <macros.hpp>
#include <exception_handler.hpp>
#include <math_constants.hpp>
#include <polychord.hpp>

#include <polychord_wrapper.h>

bool PolyChord::running_ = false;

PolyChord::PolyChord(int nPar, Math::LikelihoodFunction& like, int nLive, std::string fileRoot) : n_(nPar), like_(like), nLive_(nLive), paramsStarting_(nPar, 0), paramNames_(nPar), paramsBest_(nPar, 0), paramsMean_(nPar, 0), paramsStd_(nPar, 0), paramsCurrent_(nPar, 0), priorTypes_(nPar, 1), priorMins_(nPar, 0), priorMaxs_(nPar, 1), paramsFixed_(nPar, 0), isFixed_(nPar, false), fileRoot_(fileRoot)
{
}

void
PolyChord::setParam(int i, const std::string& name, double min, double max)
{
    check(i >= 0 && i < n_, "invalid index " << i);
    check(max >= min, "");

    if(min == max)
    {
        setParamFixed(i, name, min);
        return;
    }

    paramNames_[i] = name;
    priorTypes_[i] = 1;
    priorMins_[i] = min;
    priorMaxs_[i] = max;
    isFixed_[i] = false;
}

void
PolyChord::setParamFixed(int i, const std::string& name, double val)
{
    check(i >= 0 && i < n_, "invalid index " << i);
    paramNames_[i] = name;
    paramsFixed_[i] = val;
    isFixed_[i] = true;
}

void
PolyChord::setParamGauss(int i, const std::string& name, double mean, double sigma)
{
    check(i >= 0 && i < n_, "invalid index " << i);
    check(sigma >= 0, "");

    if(sigma == 0)
    {
        setParamFixed(i, name, mean);
        return;
    }

    paramNames_[i] = name;
    priorTypes_[i] = 2;
    priorMins_[i] = mean;
    priorMaxs_[i] = sigma;
    isFixed_[i] = false;
}

namespace
{

PolyChord *myPolyChordScanner;

double myLogLike(double *theta, double *phi, int context)
{
    return myPolyChordScanner->logLike(theta);
}

} // namespace

double
PolyChord::logLike(double *theta)
{
    //output_screen1("polychord c++ likelihood call" << std::endl);
    int j = 0;

    for(int i = 0; i < n_; ++i)
    {
        if(isFixed_[i])
        {
            paramsCurrent_[i] = paramsFixed_[i];
        }
        else
        {
            //output_screen1("param " << j << " = " << theta[j] << std::endl);
            paramsCurrent_[i] = theta[j];
            ++j;

        }
    }

    return -like_.calculate(&(paramsCurrent_[0]), n_) / 2.0;
}

void
PolyChord::run(bool res)
{
    check(!running_, "an instance of PolyChord is currently running");
    running_ = true;
    myPolyChordScanner = this;

    StandardException exc;

    nFixed_ = 0;
    for(int i = 0; i < n_; ++i)
    {
        if(isFixed_[i])
            ++nFixed_;
    }

    check(nFixed_ < n_, "cannot have all of the parameters fixed");

	const int nDims = n_ - nFixed_;
    const int nDerived = 0;
    const int numRepeats = 1;
    const bool doClustering = false;
    const int nCluster = 30;
    int feedback = 0;
#ifdef VERBOSE1
    feedback = 1;
#endif
#ifdef VERBOSE2
    feedback = 2;
#endif
    const bool calculatePost = true;
    const int sigmaPost = 5;
    const double thinPost = 1;
    std::string baseDir, root;
    const int lastSlash = fileRoot_.rfind('/');
    if(lastSlash == fileRoot_.npos)
    {
        baseDir = ".";
        root = fileRoot_;
    }
    else if(lastSlash == 0)
    {
        baseDir = "";
        root = fileRoot_.substr(1, fileRoot_.size() - 1);
    }
    else
    {
        baseDir = fileRoot_.substr(0, lastSlash);
        root = fileRoot_.substr(lastSlash + 1, fileRoot_.size() - lastSlash - 1);
    }
    const int updateResume = nLive_;
    const bool writeLive = true;

    void *context = NULL;

    std::vector<int> priorTypes;
    std::vector<double> priorMins, priorMaxs;
    
    for(int i = 0; i < n_; ++i)
    {
        if(!isFixed_[i])
        {
            priorTypes.push_back(priorTypes_[i]);
            priorMins.push_back(priorMins_[i]);
            priorMaxs.push_back(priorMaxs_[i]);
        }
    }

#ifdef COSMO_MPI
    int hasMpiInit;
    MPI_Initialized(&hasMpiInit);
    if(!hasMpiInit)
        MPI_Init(NULL, NULL);
#endif

    double logZ, errorZ, nDead, nLike, logZPlusLogP;

	// calling PolyChord
    poly::run(nDims, nDerived, nLive_, numRepeats, doClustering, nCluster, feedback, calculatePost, sigmaPost, thinPost, &(priorTypes[0]), &(priorMins[0]), &(priorMaxs[0]), baseDir.c_str(), root.c_str(), res, res, updateResume, writeLive, myLogLike, context, &logZ, &errorZ, &nDead, &nLike, &logZPlusLogP);

    running_ = false;

    int processRank = 0;

#ifdef COSMO_MPI
#ifdef CHECKS_ON
    MPI_Initialized(&hasMpiInit);
    check(hasMpiInit, "MPI not initialized");

    int hasMpiFinalized;
    MPI_Finalized(&hasMpiFinalized);
    check(!hasMpiFinalized, "MPI already finalized");
#endif
    MPI_Comm_rank(MPI_COMM_WORLD, &processRank);
#endif

    if(processRank)
        return;

    output_screen_clean("PolyChord has successfully finished" << std::endl);
    output_screen_clean("log(Z) = " << logZ << " +/- " << errorZ << std::endl);
    output_screen_clean("Number of dead points = " << nDead << std::endl);
    output_screen_clean("Number of likelihood evaluations = " << nLike << std::endl);
    output_screen_clean("log(Z) + log(prior vol) = " << logZPlusLogP << std::endl);
}
