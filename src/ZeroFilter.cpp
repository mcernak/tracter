/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "ZeroFilter.h"

Tracter::ZeroFilter::ZeroFilter(
    Component<float>* iInput, const char* iObjectName
)
{
    mObjectName = iObjectName;
    mInput = iInput;
    Connect(mInput);
    mZero = GetEnv("Zero", 0.97f);
}

void Tracter::ZeroFilter::MinSize(
    SizeType iSize, SizeType iReadBehind, SizeType iReadAhead
)
{
    // First call the base class to resize this cache
    assert(iSize > 0);
    ComponentBase::MinSize(iSize, iReadBehind, iReadAhead);

    // We expect the input buffer to be at least the size of each
    // request, then +1 for the possible read-behind.  However, don't
    // add on any extra read-ahead or back.
    assert(mInput);
    ComponentBase::MinSize(mInput, iSize+1, 1, 0);
}

Tracter::SizeType
Tracter::ZeroFilter::Fetch(IndexType iIndex, CacheArea& iOutputArea)
{
    assert(iIndex >= 0);
    CacheArea inputArea;
    float store = 0.0f;
    float* input = 0;

    // Do a single read behind the window to initialise the store
    // This makes the code afterwards identical for iIndex == 0 too.
    if (iIndex > 0)
    {
        SizeType one = mInput->Read(inputArea, iIndex-1);
        if (one == 0)
            return 0;
        input = mInput->GetPointer();
        store = input[inputArea.offset];
    }

    // The usual read and offset initialisation
    SizeType lenGot = mInput->Read(inputArea, iIndex, iOutputArea.Length());
    SizeType rOffset = inputArea.offset;
    SizeType wOffset = iOutputArea.offset;
    input = mInput->GetPointer();

    // For the edge effect, duplicate the first sample
    if (iIndex == 0)
        store = input[rOffset];

    // Main calculation
    float* output = GetPointer();
    for (SizeType i=0; i<lenGot; i++)
    {
        if (i == inputArea.len[0])
            rOffset = 0;
        if (i == iOutputArea.len[0])
            wOffset = 0;

        output[wOffset++] = input[rOffset] - mZero * store;
        store = input[rOffset++];
    }

    return lenGot;
}
