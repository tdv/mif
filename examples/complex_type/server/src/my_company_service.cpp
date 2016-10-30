//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     10.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

// STD
#include <iostream>

// THIS
#include "my_company_service.h"

MyCompanyService::MyCompanyService()
{
    LockGuard lock(m_lock);
    std::cout << "MyCompanyService" << std::endl;
}

MyCompanyService::~MyCompanyService()
{
    LockGuard lock(m_lock);
    std::cout << "~MyCompanyService" << std::endl;
}

Data::ID MyCompanyService::AddEmployee(Data::Employee const &employee)
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

void MyCompanyService::RemoveAccount(Data::ID const &id)
{
    LockGuard lock(m_lock);
    auto iter = m_employees.find(id);
    if (iter == std::end(m_employees))
    {
        std::cout << "RemoveAccount. "
            << "Employee with id " << id << " not found." << std::endl;
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

Data::Employees MyCompanyService::GetEmployees() const
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
