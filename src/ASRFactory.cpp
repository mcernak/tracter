/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "ASRFactory.h"

#include "FileSource.h"
#include "ALSASource.h"
#include "Normalise.h"

#include "Mean.h"
#include "Subtract.h"
#include "Concatenate.h"
#include "Delta.h"
#include "Variance.h"
#include "Divide.h"
#include "ZeroFilter.h"
#include "Periodogram.h"
#include "MelFilter.h"
#include "Cepstrum.h"

#include "Energy.h"
#include "ModulationVAD.h"
#include "VADGate.h"

#include "PLP.h"
#include "WarpedPeriodogram.h"
#include "Noise.h"
#include "GeometricNoise.h"
#include "MAPSpectrum.h"

Tracter::ASRFactory::ASRFactory(const char* iObjectName)
{
    mObjectName = iObjectName;

    // List all sources
    mSource["File"] = &Tracter::ASRFactory::fileSource;
    mSource["ALSA"] = &Tracter::ASRFactory::alsaSource;

    // List all available front-ends
    mFrontend["Basic"] = &Tracter::ASRFactory::basicFrontend;
    mFrontend["Noise"] = &Tracter::ASRFactory::noiseFrontend;
    mFrontend["PLP"] = &Tracter::ASRFactory::plpFrontend;
    mFrontend["Complex"] = &Tracter::ASRFactory::complexFrontend;
    mFrontend["BasicVAD"] = &Tracter::ASRFactory::basicVADFrontend;
}

Plugin<float>* Tracter::ASRFactory::CreateSource(Source*& iSource)
{
    Plugin<float> *plugin = 0;

    const char* source = GetEnv("Source", "File");
    if (mSource[source])
        plugin = (this->*mSource[source])(iSource);
    else
    {
        printf("ASRFactory: Unknown source %s\n", source);
        exit(EXIT_FAILURE);
    }

    return plugin;
}

Plugin<float>* Tracter::ASRFactory::CreateFrontend(Plugin<float>* iPlugin)
{
    Plugin<float> *plugin = 0;

    const char* frontend = GetEnv("Frontend", "Basic");
    if (mFrontend[frontend])
        plugin = (this->*mFrontend[frontend])(iPlugin);
    else
    {
        printf("ASRFactory: Unknown frontend %s\n", frontend);
        exit(EXIT_FAILURE);
    }

    bool cmn = GetEnv("NormaliseMean", 1);
    if (cmn)
    {
        Mean* m = new Mean(plugin);
        Subtract* s = new Subtract(plugin, m);
        plugin = s;
    }

    int deltaOrder = GetEnv("DeltaOrder", 0);
    if (deltaOrder > 0)
    {
        Concatenate* c = new Concatenate();
        c->Add(plugin);
        for (int i=0; i<deltaOrder; i++)
        {
            Delta* d = new Delta(plugin);
            c->Add(d);
            plugin = d;
        }
        plugin = c;
    }

    bool cvn = GetEnv("NormaliseVariance", 0);
    if (cvn)
    {
        Variance* v = new Variance(plugin);
        Divide* d = new Divide(plugin, v);
        plugin = d;
    }

    return plugin;
}


Plugin<float>* Tracter::ASRFactory::fileSource(Source*& iSource)
{
    FileSource<short>* s = new FileSource<short>();
    Normalise* n = new Normalise(s);
    iSource = s;
    return n;
}

Plugin<float>* Tracter::ASRFactory::alsaSource(Source*& iSource)
{
    ALSASource* s = new ALSASource();
    Normalise* n = new Normalise(s);
    iSource = s;
    return n;
}

Plugin<float>* Tracter::ASRFactory::basicFrontend(Plugin<float>* iPlugin)
{
    /* Basic signal processing chain */
    ZeroFilter* zf = new ZeroFilter(iPlugin);
    Periodogram* p = new Periodogram(zf);
    MelFilter* mf = new MelFilter(p);
    Cepstrum* c = new Cepstrum(mf);
    return c;
}

Plugin<float>* Tracter::ASRFactory::plpFrontend(Plugin<float>* iPlugin)
{
    ZeroFilter* zf = new ZeroFilter(iPlugin);
    Periodogram* p = new Periodogram(zf);
    MelFilter* mf = new MelFilter(p);
    PLP* l = new PLP(mf);
    return l;
}

Plugin<float>* Tracter::ASRFactory::complexFrontend(Plugin<float>* iPlugin)
{
    ZeroFilter* zf = new ZeroFilter(iPlugin);
    WarpedPeriodogram* p = new WarpedPeriodogram(zf);
    Cepstrum* c = new Cepstrum(p);
    return c;
}

Plugin<float>* Tracter::ASRFactory::noiseFrontend(Plugin<float>* iPlugin)
{
    ZeroFilter* zf = new ZeroFilter(iPlugin);
    Periodogram* p = new Periodogram(zf);
    GeometricNoise* gn = new GeometricNoise(p);
    Noise* nn = new Noise(p);
    MAPSpectrum *mp = new MAPSpectrum(p, nn, gn);
    MelFilter* mf = new MelFilter(mp);
    Cepstrum* c = new Cepstrum(mf);
    return c;
}

Plugin<float>* Tracter::ASRFactory::basicVADFrontend(Plugin<float>* iPlugin)
{
    /* Basic signal processing chain */
    ZeroFilter* zf = new ZeroFilter(iPlugin);
    Periodogram* p = new Periodogram(zf);
    MelFilter* mf = new MelFilter(p);
    Cepstrum* c = new Cepstrum(mf);

    Energy* e = new Energy(iPlugin);
    ModulationVAD* v = new ModulationVAD(e);
    VADGate* g = new VADGate(c, v);

    return g;
}
