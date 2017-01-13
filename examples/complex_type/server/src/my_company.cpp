//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

// STD
#include <iostream>
#include <mutex>

// MIF
#include <mif/service/creator.h>

// COMMON
#include "common/id/service.h"
#include "common/interface/imy_company.h"

namespace Service
{
    namespace Detail
    {
        namespace
        {

            class MyCompany
                : public IMyCompany
            {
            public:
                MyCompany()
                {
                    LockGuard lock(m_lock);
                    std::cout << "MyCompany" << std::endl;
                }

                ~MyCompany()
                {
                    LockGuard lock(m_lock);
                    std::cout << "~MyCompany" << std::endl;
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
                        std::cout << "AddEmployee. "
                            << "Name: " << employee.name << " "
                            << "LastName: " << employee.lastName << " "
                            << "Age: " << employee.age << " "
                            << "Position: " << employee.position << std::endl;
                    }
                    return id;
                }

                virtual void RemoveAccount(Data::ID const &id) override final
                {
                    LockGuard lock(m_lock);
                    auto iter = m_employees.find(id);
                    if (iter == std::end(m_employees))
                    {
                        std::cout << "RemoveAccount. "
                            << "Employee with id " << id << " not found." << std::endl;
                        throw std::runtime_error{"RemoveAccount. Employee with id " + id + " not found."};
                    }
                    else
                    {
                        std::cout << "Removed employee account for Id: " << iter->first << " "
                            << "Name: " << iter->second.name << " "
                            << "LastName: " << iter->second.lastName << " "
                            << "Age: " << iter->second.age << " "
                            << "Position: " << iter->second.position << std::endl;

                        m_employees.erase(iter);
                    }
                }

                virtual Data::Employees GetEmployees() const override final
                {
                    Data::Employees employees;
                    {
                        LockGuard lock(m_lock);
                        employees = m_employees;
                        std::cout << "GetEmployees." << std::endl;
                        for (auto const &i : employees)
                        {
                            std::cout << "Id: " << i.first << " "
                                << "Name: " << i.second.name << " "
                                << "LastName: " << i.second.lastName << " "
                                << "Age: " << i.second.age << " "
                                << "Position: " << i.second.position << std::endl;
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
