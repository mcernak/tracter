/*
 * Copyright 2010 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef BOOLTOFLOAT_H
#define BOOLTOFLOAT_H

#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Convert bool to float
     */
    class BoolToFloat : public CachedComponent<float>
    {
    public:
    	BoolToFloat(
            Component<BoolType>* iInput, const char* iObjectName = "BoolToFloat"
        );
        virtual ~BoolToFloat() throw();

    protected:
        bool UnaryFetch(IndexType iIndex, float* oData);

        void DotHook()
        {
            CachedComponent<float>::DotHook();
        }

    private:
        Component<BoolType>* mInput;
        bool mBool;
    };
}

#endif /* BOOLTOFLOAT_H */
