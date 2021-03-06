#ifndef COSMO_PP_COSMOLOGICAL_PARAMS_HPP
#define COSMO_PP_COSMOLOGICAL_PARAMS_HPP

#include <cmath>
#include <string>

#include <macros.hpp>
#include <phys_constants.hpp>
#include <unit_conversions.hpp>
#include <power_spectrum.hpp>

/// An abstract class for cosmological parameters. This serves as a base class for different models.
class CosmologicalParams
{
public:
    /// Constructor.
    CosmologicalParams() : temp_(2.726) {}

    /// Destructor.
    virtual ~CosmologicalParams() {}

    /// Omega baryon times h^2 (unitless).
    virtual double getOmBH2() const = 0;

    /// Omega cdm times h^2 (unitless).
    virtual double getOmCH2() const = 0;

    /// h (unitless). H = 100h km / s / Mpc.
    virtual double getH() const = 0;

    /// Omega baryon (unitless).
    virtual double getOmB() const = 0;

    /// Omega cdm (unitless).
    virtual double getOmC() const = 0;

    /// Omega matter, sum of baryons and cdm (unitless).
    virtual double getOmM() const { return getOmB() + getOmC(); }

    /// Omega Lambda (unitless).
    virtual double getOmLambda() const = 0;

    /// Omega curvature (unitless).
    virtual double getOmK() const = 0;

    /// The scalar spectral index (unitless).
    virtual double getNs() const = 0;

    /// The scalar power spectrum amplitude at the pivot point (unitless).
    virtual double getAs() const = 0;

    /// The pivot point for defining power spectrum parameters (Mpc^-1).
    virtual double getPivot() const = 0;

    /// Reionization optical depth.
    virtual double getTau() const = 0;

    /// Number of effective degrees of freedom for relativistic particles (neutrinos for example).
    virtual double getNEff() const = 0;

    /// The number of non-cdm particles (such as massive neutrinos).
    virtual int getNumNCDM() const = 0;

    /// The mass of a non-cdm particle in eV.
    /// \param i The index of the particle, should be 0 <= i < getNumNCDM().
    virtual double getNCDMParticleMass(int i) const = 0;

    /// The temperature of a non-cdm particle (unitless). This is the fraction of the temperature divided by the photon temperature.
    /// \param i The index of the particle, should be 0 <= i < getNumNCDM().
    virtual double getNCDMParticleTemp(int i) const = 0;

    /// The helium fraction (unitless). A value of 0 will imply using BBN.
    virtual double getYHe() const = 0;

    /// Tensor-to-scalar ratio (unitless).
    virtual double getR() const = 0;

    /// The tensor spectral index (unitless).
    virtual double getNt() const = 0;

    /// The primordial scalar power spectrum function.
    virtual const Math::RealFunction& powerSpectrum() const = 0;

    /// The primordial tensor power spectrum function.
    virtual const Math::RealFunction& powerSpectrumTensor() const = 0;

    /// The name of the parameters model.
    virtual std::string name() const = 0; 

    /// All of the relevant parameters in one vector.
    virtual void getAllParameters(std::vector<double>& v) const { v.clear(); }

    /// Set all of the relevant parameters.
    /// \param v A vector containing all of the cosmological parameters.
    /// \param badLike If the result is false, this number will be set to a large value, which can show how bad it is. This will be used to assess a very large negative likelihood. This parameter can be set to NULL in which case it will be ignored.
    /// \return true if successful, false otherwise.
    virtual bool setAllParameters(const std::vector<double>& v, double *badLike = NULL) = 0;

    /// The Hubble constant without units (the reduced Planck mass is assumed to be 1, together with c and hbar).
    virtual double getHubbleUnitless() const
    {
        double hubble = getH() * 100;
        hubble /= Phys::MegaParsec;
        hubble *= 1e3; // inverse sec;
        return Phys::inverseSecToUnitless(hubble);
    }

    /// Omega gamma, i.e. photons (unitless).
    virtual double getOmG() const
    {
        const double tUnitless = Phys::kelvinToUnitless(temp_);
        const double rhoGamma = Math::pi * Math::pi * tUnitless * tUnitless * tUnitless * tUnitless / 15.0;
        const double hubbleUnitless = getHubbleUnitless();
        const double rhoCr = 3 * hubbleUnitless * hubbleUnitless;
        return rhoGamma / rhoCr;
    }

    /// Omega neutrinos, relativistic only, determined from N_eff (unitless).
    virtual double getOmNeutrino() const
    {
        return getNEff() * 7.0 / 8.0 * std::pow(double(4) / 11, double(4) / 3) * getOmG();
    }

    /// Omega radiation, i.e. sum of gamma and neutrinos (unitless).
    virtual double getOmR() const
    {
        return getOmG() + getOmNeutrino();
    }

    /// The temperature of photons in K. Default value is 2.726.
    double getTemperature() const { return temp_; }

    /// Set the temperature of photons in K.
    void setTemperature(double temp) { check(temp > 0, "invalid temperature " << temp); temp_ = temp; }

private:
    double temp_;
};

class LambdaCDMParams : public CosmologicalParams
{
    class DummyPS : public Math::RealFunction
    {
    public:
        DummyPS() { }
        double evaluate(double x) const { return 0.0; }
    };

public:
    LambdaCDMParams(double omBH2, double omCH2, double h, double tau, double ns, double as, double pivot, double run = 0.0) : CosmologicalParams(), omBH2_(omBH2), omCH2_(omCH2), h_(h), tau_(tau), ps_(as, ns, pivot, run) {}
    virtual ~LambdaCDMParams() {}

    virtual double getOmBH2() const { return omBH2_; }
    virtual double getOmCH2() const { return omCH2_; }
    virtual double getH() const { return h_; }
    virtual double getOmB() const { return omBH2_ / (h_ * h_); }
    virtual double getOmC() const { return omCH2_ / (h_ * h_); }
    virtual double getOmLambda() const { return 1.0 - CosmologicalParams::getOmM(); }
    virtual double getOmK() const { return 0.0; }
    virtual double getNs() const { return ps_.getNs(); }
    virtual double getAs() const { return ps_.getAs(); }
    virtual double getPivot() const { return ps_.getPivot(); }
    virtual double getTau() const { return tau_; }
    virtual double getNEff() const { return 3.046; }
    virtual int getNumNCDM() const { return 0; }
    virtual double getNCDMParticleMass(int i) const { check(false, ""); }
    virtual double getNCDMParticleTemp(int i) const { check(false, ""); }
    virtual double getYHe() const { return 0.0; } // means use BBN
    virtual double getR() const { return 0.0; }
    virtual double getNt() const { return 0.0; }

    virtual const Math::RealFunction& powerSpectrum() const { return ps_; }
    virtual const Math::RealFunction& powerSpectrumTensor() const { return psTensor_; }

    virtual std::string name() const { return "LambdaCDM"; }
    virtual void getAllParameters(std::vector<double>& v) const
    {
        v.resize(6);
        v[0] = getOmBH2();
        v[1] = getOmCH2();
        v[2] = getH();
        v[3] = getTau();
        v[4] = getNs();
        v[5] = std::log(getAs() * 1e10);
    }

    virtual bool setAllParameters(const std::vector<double>& v, double *badLike = NULL)
    {
        check(v.size() >= 6, "");
        omBH2_ = v[0];
        omCH2_ = v[1];
        h_ = v[2];
        tau_ = v[3];
        ps_.setNs(v[4]);
        ps_.setAs(std::exp(v[5]) / 1e10);

        if(badLike)
            *badLike = 0;

        return true;
    }

protected:
    double omBH2_;
    double omCH2_;
    double h_;
    double tau_;
    StandardPowerSpectrum ps_;
    DummyPS psTensor_;
};

class LCDMWithTensorParams : public LambdaCDMParams
{
public:
    LCDMWithTensorParams(double omBH2, double omCH2, double h, double tau, double ns, double as, double pivot, double r, double nt, double pivotTensor) : LambdaCDMParams(omBH2, omCH2, h, tau, ns, as, pivot), r_(r), nt_(nt)
    {
        psT_ = new StandardPowerSpectrumTensor(powerSpectrum(), r, nt, pivotTensor);
        check(r_ >= 0, "invalid r");
    }

    ~LCDMWithTensorParams() { delete psT_; }

    virtual double getR() const { return r_; }
    virtual double getNt() const { return nt_; }

    virtual const Math::RealFunction& powerSpectrumTensor() const { return *psT_; }

    virtual std::string name() const { return "LCDMWithTensor"; }
    virtual void getAllParameters(std::vector<double>& v) const
    {
        LambdaCDMParams::getAllParameters(v);
        v.push_back(getR());
    }

    virtual bool setAllParameters(const std::vector<double>& v, double *badLike = NULL)
    {
        check(v.size() >= 7, "");

        LambdaCDMParams::setAllParameters(v, badLike);
        r_ = v[6];
        nt_ = 0.0;
        const double piv = psT_->getPivot();
        psT_->set(powerSpectrum(), r_, nt_, piv);

        return true;
    }

private:
    double r_;
    double nt_;
    StandardPowerSpectrumTensor* psT_;
};

class LCDMWithDegenerateNeutrinosParams : public LambdaCDMParams
{
public:
    LCDMWithDegenerateNeutrinosParams(double omBH2, double omCH2, double h, double tau, double ns, double as, double pivot, double nEff, int nMassive, double sumMNu) : LambdaCDMParams(omBH2, omCH2, h, tau, ns, as, pivot), nEff_(nEff), nMassive_(nMassive), sumMNu_(sumMNu)
    {
        check(nEff > 0, "invalid nEff = " << nEff);
        check(sumMNu >= 0, "invalid sumMNu = " << sumMNu);
        check(nMassive >= 0, "number of massive neutrinos is negative: " << nMassive);
        check(nEff > nMassive, "nEff needs to be more than the number of massive neutrinos");
    }

    ~LCDMWithDegenerateNeutrinosParams() {}

    virtual double getNEff() const { return nEff_ - nMassive_; }
    virtual int getNumNCDM() const { return nMassive_; }
    virtual double getNCDMParticleMass(int i) const
    {
        check(i >= 0 && i < nMassive_, "invalid index = " << i);
        return sumMNu_ / nMassive_;
    }

    virtual double getNCDMParticleTemp(int i) const
    {
        check(i >= 0 && i < nMassive_, "invalid index = " << i);
        //return 0.715985;
        return 0.713765855506013;
    }

    virtual std::string name() const { return "LCDMWithDegenerateNeutrinos"; }
    virtual void getAllParameters(std::vector<double>& v) const { check(false, "not implemented"); }
    virtual bool setAllParameters(const std::vector<double>& v, double *badLike = NULL) { check(false, "not implemented"); return false; }

private:
    double nEff_;
    int nMassive_;
    double sumMNu_;
};

class LCDMWithTensorAndDegenerateNeutrinosParams : public LambdaCDMParams
{
public:
    LCDMWithTensorAndDegenerateNeutrinosParams(double omBH2, double omCH2, double h, double tau, double ns, double as, double pivot, double r, double nt, double pivotTensor, double nEff, int nMassive, double sumMNu, double run = 0.0) : LambdaCDMParams(omBH2, omCH2, h, tau, ns, as, pivot, run), r_(r), nt_(nt), nEff_(nEff), nMassive_(nMassive), sumMNu_(sumMNu)
    {
        check(nEff > 0, "invalid nEff = " << nEff);
        check(sumMNu >= 0, "invalid sumMNu = " << sumMNu);
        check(nMassive >= 0, "number of massive neutrinos is negative: " << nMassive);
        check(nEff > nMassive, "nEff needs to be more than the number of massive neutrinos");
        
        psT_ = new StandardPowerSpectrumTensor(powerSpectrum(), r, nt, pivotTensor);
        check(r_ >= 0, "invalid r");
    }

    ~LCDMWithTensorAndDegenerateNeutrinosParams() { delete psT_; }

    virtual double getR() const { return r_; }
    virtual double getNt() const { return nt_; }

    virtual const Math::RealFunction& powerSpectrumTensor() const { return *psT_; }

    virtual double getNEff() const { return nEff_ - nMassive_; }
    virtual int getNumNCDM() const { return nMassive_; }
    virtual double getNCDMParticleMass(int i) const
    {
        check(i >= 0 && i < nMassive_, "invalid index = " << i);
        return sumMNu_ / nMassive_;
    }

    virtual double getNCDMParticleTemp(int i) const
    {
        check(i >= 0 && i < nMassive_, "invalid index = " << i);
        //return 0.715985;
        return 0.713765855506013;
    }

    virtual std::string name() const { return "LCDMWithTensorAndDegenerateNeutrinos"; }
    virtual void getAllParameters(std::vector<double>& v) const { check(false, "not implemented"); }
    virtual bool setAllParameters(const std::vector<double>& v, double *badLike = NULL) { check(false, "not implemented"); return false; }

private:
    double nEff_;
    int nMassive_;
    double sumMNu_;
    double r_;
    double nt_;
    StandardPowerSpectrumTensor* psT_;
};

class LCDMWithCutoffTensorDegenerateNeutrinosParams : public LCDMWithTensorAndDegenerateNeutrinosParams
{
public:
    LCDMWithCutoffTensorDegenerateNeutrinosParams(double omBH2, double omCH2, double h, double tau, double kCut, double ns, double as, double pivot, double r, double nt, double pivotTensor, double nEff, int nMassive, double sumMNu) : LCDMWithTensorAndDegenerateNeutrinosParams(omBH2, omCH2, h, tau, ns, as, pivot, r, nt, pivotTensor, nEff, nMassive, sumMNu), psC_(kCut, as, ns, pivot)
    {
    }

    ~LCDMWithCutoffTensorDegenerateNeutrinosParams() {}

    virtual const Math::RealFunction& powerSpectrum() const { return psC_; }
    virtual double getNs() const { return psC_.getNs(); }
    virtual double getAs() const { return psC_.getAs(); }
    virtual double getPivot() const { return psC_.getPivot(); }

    virtual std::string name() const { return "LCDMWithCutoffTensorDegenerateNeutrinos"; }
    virtual void getAllParameters(std::vector<double>& v) const { check(false, "not implemented"); }
    virtual bool setAllParameters(const std::vector<double>& v, double *badLike = NULL) { check(false, "not implemented"); return false; }

private:
    CutoffPowerSpectrum psC_;
};

class LinearSplineParams : public CosmologicalParams
{
    class DummyPS : public Math::RealFunction
    {
    public:
        DummyPS() { }
        double evaluate(double x) const { return 0.0; }
    };

public:
    LinearSplineParams(double omBH2, double omCH2, double h, double tau, const std::vector<double>& kVals, const std::vector<double>& amplitudes) : CosmologicalParams(), omBH2_(omBH2), omCH2_(omCH2), h_(h), tau_(tau), ps_(kVals, amplitudes) {}
    ~LinearSplineParams() {}

    virtual double getOmBH2() const { return omBH2_; }
    virtual double getOmCH2() const { return omCH2_; }
    virtual double getH() const { return h_; }
    virtual double getOmB() const { return omBH2_ / (h_ * h_); }
    virtual double getOmC() const { return omCH2_ / (h_ * h_); }
    virtual double getOmLambda() const { return 1.0 - CosmologicalParams::getOmM(); }
    virtual double getOmK() const { return 0.0; }
    virtual double getNs() const { return ps_.getNs(); }
    virtual double getAs() const { return ps_.getAs(); }
    virtual double getPivot() const { return ps_.getPivot(); }
    virtual double getTau() const { return tau_; }
    virtual double getNEff() const { return 3.046; }
    virtual int getNumNCDM() const { return 0; }
    virtual double getNCDMParticleMass(int i) { check(false, ""); }
    virtual double getYHe() const { return 0.0; } // means use BBN
    virtual double getR() const { return 0.0; }
    virtual double getNt() const { return 0.0; }

    virtual const Math::RealFunction& powerSpectrum() const { return ps_; }
    virtual const Math::RealFunction& powerSpectrumTensor() const { return psTensor_; }

    virtual std::string name() const { return "LinearSpline"; }
    virtual void getAllParameters(std::vector<double>& v) const { check(false, "not implemented"); }
    virtual bool setAllParameters(const std::vector<double>& v, double *badLike = NULL) { check(false, "not implemented"); return false; }

private:
    double omBH2_;
    double omCH2_;
    double h_;
    double tau_;
    LinearSplinePowerSpectrum ps_;
    DummyPS psTensor_;
};

class CubicSplineParams : public CosmologicalParams
{
    class DummyPS : public Math::RealFunction
    {
    public:
        DummyPS() { }
        double evaluate(double x) const { return 0.0; }
    };

public:
    CubicSplineParams(double omBH2, double omCH2, double h, double tau, const std::vector<double>& kVals, const std::vector<double>& amplitudes) : CosmologicalParams(), omBH2_(omBH2), omCH2_(omCH2), h_(h), tau_(tau), ps_(kVals, amplitudes) {}
    ~CubicSplineParams() {}

    virtual double getOmBH2() const { return omBH2_; }
    virtual double getOmCH2() const { return omCH2_; }
    virtual double getH() const { return h_; }
    virtual double getOmB() const { return omBH2_ / (h_ * h_); }
    virtual double getOmC() const { return omCH2_ / (h_ * h_); }
    virtual double getOmLambda() const { return 1.0 - CosmologicalParams::getOmM(); }
    virtual double getOmK() const { return 0.0; }
    virtual double getNs() const { return ps_.getNs(); }
    virtual double getAs() const { return ps_.getAs(); }
    virtual double getPivot() const { return ps_.getPivot(); }
    virtual double getTau() const { return tau_; }
    virtual double getNEff() const { return 3.046; }
    virtual int getNumNCDM() const { return 0; }
    virtual double getNCDMParticleMass(int i) { check(false, ""); }
    virtual double getYHe() const { return 0.0; } // means use BBN
    virtual double getR() const { return 0.0; }
    virtual double getNt() const { return 0.0; }

    virtual const Math::RealFunction& powerSpectrum() const { return ps_; }
    virtual const Math::RealFunction& powerSpectrumTensor() const { return psTensor_; }

    virtual std::string name() const { return "CubicSpline"; }
    virtual void getAllParameters(std::vector<double>& v) const { check(false, "not implemented"); }
    virtual bool setAllParameters(const std::vector<double>& v, double *badLike = NULL) { check(false, "not implemented"); return false; }

private:
    double omBH2_;
    double omCH2_;
    double h_;
    double tau_;
    CubicSplinePowerSpectrum ps_;
    DummyPS psTensor_;
};

#endif

