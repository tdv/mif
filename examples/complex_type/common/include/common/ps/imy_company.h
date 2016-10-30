//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __COMPLEX_TYPE_COMMON_PS_IMYCOMPANY_H__
#define __COMPLEX_TYPE_COMMON_PS_IMYCOMPANY_H__

// STD
#include <string>

// MIF
#include <mif/remote/ps.h>

// THIS
#include "common/interface/imy_company.h"

MIF_REMOTE_PS_BEGIN(IMyCompany)
    MIF_REMOTE_METHOD(AddEmployee)
    MIF_REMOTE_METHOD(RemoveAccount)
    MIF_REMOTE_METHOD(GetEmployees)
MIF_REMOTE_PS_END()

#endif  // !__COMPLEX_TYPE_COMMON_PS_IMYCOMPANY_H__
