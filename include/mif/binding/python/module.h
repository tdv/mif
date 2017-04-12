//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     04.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_BINDING_PYTHON_MODULE_H__
#define __MIF_BINDING_PYTHON_MODULE_H__

// MIF
#include "mif/binding/python/detail/module.h"

#define MIF_PYTHON_MODULE(name__) \
    BOOST_PYTHON_MODULE(name__) \
    { \
        ::Mif::Binding::Python::Detail::ImportStructs(); \
    }

#endif  // !__MIF_BINDING_PYTHON_MODULE_H__
