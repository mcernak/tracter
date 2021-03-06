/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cmath>

#include "HTKSink.h"

Tracter::HTKSink::HTKSink(
    Component<float>* iInput,
    const char* iObjectName
)
{
    mObjectName = iObjectName;
    mInput = iInput;
    Connect(mInput);
    mFrame.size = mInput->Frame().size;
    Initialise();
    Reset();

    mFile = 0;
    Endian endian = (Endian)GetEnv(cEndian, ENDIAN_BIG);
    mByteOrder.SetTarget(endian);
    if (mByteOrder.WrongEndian())
        mTemp.resize(mFrame.size);

    /* Initial header values */
    float period = 1.0f / FrameRate();
    mNSamples = 0;
    mSampPeriod = (int)(period * 1e7f + 0.5);
    mSampSize = mFrame.size * sizeof(float);

    /* Parameter type is mutually exclusive, default to USER */
    const StringEnum cParmKind[] = {
        {"LPC",       1},
        {"LPCEPSTRA", 3},
        {"MFCC",      6},
        {"FBANK",     7},
        {"MELSPEC",   8},
        {"USER",      9},
        {"PLP",      11},
        {0,          -1}
    };
    mParmKind = GetEnv(cParmKind, 9);

    /* Modifiers can be mixed */
    if (GetEnv("E", 0)) mParmKind |= 0000100;
    if (GetEnv("N", 0)) mParmKind |= 0000200;
    if (GetEnv("D", 0)) mParmKind |= 0000400;
    if (GetEnv("A", 0)) mParmKind |= 0001000;
    if (GetEnv("Z", 0)) mParmKind |= 0004000;
    if (GetEnv("0", 0)) mParmKind |= 0020000;
    if (GetEnv("T", 0)) mParmKind |= 0100000;
}

void Tracter::HTKSink::WriteHeader(FILE* iFile)
{
    /* Copy header */
    int nSamples = mNSamples;
    int sampPeriod = mSampPeriod;
    short sampSize = mSampSize;
    short parmKind = mParmKind;

    /* Byte swap if necessary */
    if (mByteOrder.WrongEndian())
    {
        mByteOrder.Swap(&nSamples, 4, 1);
        mByteOrder.Swap(&sampPeriod, 4, 1);
        mByteOrder.Swap(&sampSize, 2, 1);
        mByteOrder.Swap(&parmKind, 2, 1);
    }

    /* Write */
    bool fail = false;
    fail |= (fwrite(&nSamples,   4, 1, iFile) != 1);
    fail |= (fwrite(&sampPeriod, 4, 1, iFile) != 1);
    fail |= (fwrite(&sampSize,   2, 1, iFile) != 1);
    fail |= (fwrite(&parmKind,   2, 1, iFile) != 1);
    if (fail)
        throw Exception("HTKSink: Failed to write HTK header");
}

/**
 * Opens the given file and sucks data into it.
 */
void Tracter::HTKSink::Open(const char* iFile)
{
    assert(iFile);
    assert(!mFile);

    Verbose(1, "%s\n", iFile);
    mFile = fopen(iFile, "w");
    if (!mFile)
        throw Exception("HTKSink: Failed to open file %s", iFile);

    WriteHeader(mFile);

    /* Processing loop */
    int index = 0;
    CacheArea cache;
    while (mInput->Read(cache, index++))
    {
        float* f = mInput->GetPointer(cache.offset);
        for (int i=0; i<mFrame.size; i++)
            if (!finite(f[i]))
                throw Exception("HTKSink: !finite at %s frame %d index %d",
                                iFile, index, i);
        if (mByteOrder.WrongEndian())
        {
            for (int i=0; i<mFrame.size; i++)
                mTemp[i] = f[i];
            f = &mTemp[0];
            mByteOrder.Swap(f, sizeof(float), mFrame.size);
        }
        if ((int)fwrite(f, sizeof(float), mFrame.size, mFile) != mFrame.size)
            throw Exception("HTKSink: Failed to write to file %s", iFile);
    }
    mNSamples = index - 1;
    rewind(mFile);
    WriteHeader(mFile);

    if (fclose(mFile) != 0)
        throw Exception("HTKSink: Could not close file %s", iFile);
    mFile = 0;
    Verbose(1, "Wrote %d frames\n", mNSamples);
}
