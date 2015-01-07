/*
 * Copyright 2013 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>
#include <cmath>

#include "BoolToFloat.h"

/**
 * Convert bool to float.
 */
Tracter::BoolToFloat::BoolToFloat(Component<BoolType>* iInput, const char* iObjectName)
{
    mObjectName = iObjectName;
    mInput = iInput;
    Connect(mInput);
    mFrame.size = iInput->Frame().size;
    assert(mFrame.size >= 0);

    mBool = false;
}

Tracter::BoolToFloat::~BoolToFloat() throw()
{
//    if (mFloored > 0)
//        Verbose(1, "floored %d values < %e\n", mFloored, mFloor);
}

bool Tracter::BoolToFloat::UnaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);

    // Read the input frame
    const BoolType* open = mInput->UnaryRead(iIndex);
    if (!open)
    {
        Verbose(2, "BoolToFloat: End Of Data at %ld\n", iIndex);
        return false;
    }
    mBool = *open;

    if (mBool == true)
    	oData[0] = (float) 1;
    else
    	oData[0] = (float) 0;
    // Done
    return true;
}
