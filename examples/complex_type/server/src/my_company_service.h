//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

#ifndef __COMPLEX_TYPE_SERVER_SERVICE_H__
#define __COMPLEX_TYPE_SERVER_SERVICE_H__

// STD
#include <mutex>

// THIS
#include "common/interface/imy_company.h"

class MyCompanyService final
    : public IMyCompany
{
public:
    MyCompanyService();
    ~MyCompanyService();

private:
    using LockType = std::mutex;
    using LockGuard = std::lock_guard<LockType>;

    mutable LockType m_lock;

    Data::Employees m_employees;

    // IMyCompany
    virtual Data::ID AddEmployee(Data::Employee const &employee) override final;
    virtual void RemoveAccount(Data::ID const &id) override final;
    virtual Data::Employees GetEmployees() const override final;
};

#endif  // !__COMPLEX_TYPE_SERVER_SERVICE_H__
