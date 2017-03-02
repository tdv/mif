//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016 tdv
//-------------------------------------------------------------------

// STD
#include <iostream>
#include <list>
#include <mutex>

// MIF
#include <mif/service/creator.h>

// THIS
#include "common/id/service.h"
#include "common/interface/ihello_world.h"

namespace Service
{
    namespace Detail
    {
        namespace
        {

            class HelloWorld
                : public IHelloWorld
            {
            public:
                HelloWorld()
                {
                    /*LockGuard lock(m_lock);
                    std::cout << "HelloWorld" << std::endl;*/
                }

                ~HelloWorld()
                {
                    /*LockGuard lock(m_lock);
                    std::cout << "~HelloWorld" << std::endl;*/
                }

            private:
                using LockType = std::mutex;
                using LockGuard = std::lock_guard<LockType>;

                mutable LockType m_lock;
                std::list<std::string> m_words;

                // IHelloWorld
                virtual void AddWord(std::string const &word) override final
                {
                    LockGuard lock(m_lock);
                    std::cout << "AddWord: " << word << std::endl;
                    m_words.push_back(word);
                }

                virtual std::string GetText() const override final
                {
                    std::string text;
                    {
                        //LockGuard lock(m_lock);
                        //std::cout << "GetText. Creating text in " << m_words.size() << " words." << std::endl;
                        for (auto const &word : m_words)
                        {
                            if (!text.empty())
                                text += " ";
                            text += word;
                        }
                        //std::cout << "GetText. Created text in " << m_words.size() << " words. Text \"" << text << "\"" << std::endl;
                    }
                    return text;
                }


                virtual void Clean() override final
                {
                    LockGuard lock(m_lock);
                    std::cout << "Clean. Remove " << m_words.size() << " words." << std::endl;
                    decltype(m_words){}.swap(m_words);
                }

            };

        }   // namespace
    }   // namespace Detail
}   // namespace Service

MIF_SERVICE_CREATOR
(
    ::Service::Id::HelloWorld,
    ::Service::Detail::HelloWorld
)
