// STD
#include <iostream>

// THIS
#include "hello_world_service.h"

HelloWorldService::HelloWorldService()
{
    LockGuard lock(m_lock);
    std::cout << "HelloWorldService" << std::endl;
}

HelloWorldService::~HelloWorldService()
{
    LockGuard lock(m_lock);
    std::cout << "~HelloWorldService" << std::endl;
}

void HelloWorldService::AddWord(std::string const &word)
{
    LockGuard lock(m_lock);
    std::cout << "AddWord: " << word << std::endl;
    m_words.push_back(word);
}

std::string HelloWorldService::GetText() const
{
    std::string text;
    {
        LockGuard lock(m_lock);
        std::cout << "GetText. Creating text in " << m_words.size() << " words." << std::endl;
        for (auto const &word : m_words)
        {
            if (!text.empty())
                text += " ";
            text += word;
        }
        std::cout << "GetText. Created text in " << m_words.size() << " words. Text \"" << text << "\"" << std::endl;
    }
    return text;
}

void HelloWorldService::Clean()
{
    LockGuard lock(m_lock);
    std::cout << "Clean. Remove " << m_words.size() << " words." << std::endl;
    decltype(m_words){}.swap(m_words);
}
