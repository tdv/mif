//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2016
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// STD
#include <mutex>

// MIF
#include <mif/common/log.h>
#include <mif/reflection/reflection.h>
#include <mif/service/creator.h>

// COMMON
#include "common/id/service.h"
#include "common/interface/imy_company.h"
#include "common/meta/data.h"

namespace Service
{
    namespace Detail
    {
        namespace
        {

            class MyCompany
                : public Mif::Service::Inherit<IMyCompany>
            {
            public:
                MyCompany()
                {
                    MIF_LOG(Info) << "MyCompany";
                }

                ~MyCompany()
                {
                    MIF_LOG(Info) << "~MyCompany";
                }

            private:
                using LockType = std::mutex;
                using LockGuard = std::lock_guard<LockType>;

                mutable LockType m_lock;

                Data::Employees m_employees;

                // IMyCompany
                virtual Data::ID AddEmployee(Data::Employee const &employee) override final
                {
                    Data::ID id;
                    {
                        LockGuard lock(m_lock);
                        id = std::to_string(m_employees.size());
                        m_employees[id] = employee;
                    }
                    MIF_LOG(Info) << "AddEmployee. "
                        << "Name: " << employee.name << " "
                        << "LastName: " << employee.lastName << " "
                        << "Age: " << employee.age << " "
                        << "Position: " << Mif::Reflection::ToString(employee.position);
                    return id;
                }

                virtual void RemoveAccount(Data::ID const &id) override final
                {
                    LockGuard lock(m_lock);
                    auto iter = m_employees.find(id);
                    if (iter == std::end(m_employees))
                    {
                        MIF_LOG(Warning) << "RemoveAccount. " << "Employee with id " << id << " not found.";
                        throw std::runtime_error{"RemoveAccount. Employee with id " + id + " not found."};
                    }
                    else
                    {
                        MIF_LOG(Info) << "Removed employee account for Id: " << iter->first << " "
                            << "Name: " << iter->second.name << " "
                            << "LastName: " << iter->second.lastName << " "
                            << "Age: " << iter->second.age << " "
                            << "Position: " << Mif::Reflection::ToString(iter->second.position);

                        m_employees.erase(iter);
                    }
                }

                virtual Data::Employees GetEmployees() const override final
                {
                    Data::Employees employees;
                    {
                        LockGuard lock(m_lock);
                        employees = m_employees;
                        MIF_LOG(Info) << "GetEmployees.";
                        for (auto const &i : employees)
                        {
                            MIF_LOG(Info) << "Id: " << i.first << " "
                                << "Name: " << i.second.name << " "
                                << "LastName: " << i.second.lastName << " "
                                << "Age: " << i.second.age << " "
                                << "Position: " << Mif::Reflection::ToString(i.second.position);
                        }
                    }
                    return employees;
                }
            };

        }   // namespace
    }   // namespace Detail
}   // namespace Service

MIF_SERVICE_CREATOR
(
    ::Service::Id::MyCompany,
    ::Service::Detail::MyCompany
)
