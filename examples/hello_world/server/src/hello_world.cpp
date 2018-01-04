//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     09.2016
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

// STD
#include <list>
#include <mutex>

// MIF
#include <mif/common/log.h>
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
                : public Mif::Service::Inherit<IHelloWorld>
            {
            public:
                HelloWorld()
                {
                    MIF_LOG(Info) << "HelloWorld";
                }

                ~HelloWorld()
                {
                    MIF_LOG(Info) << "~HelloWorld";
                }

            private:
                using LockType = std::mutex;
                using LockGuard = std::lock_guard<LockType>;

                mutable LockType m_lock;
                std::list<std::string> m_words;

                // IHelloWorld
                virtual void AddWord(std::string const &word) override final
                {
                    MIF_LOG(Info) << "AddWord: " << word;
                    LockGuard lock(m_lock);
                    m_words.push_back(word);
                }

                virtual std::string GetText() const override final
                {
                    std::string text;
                    {
                        LockGuard lock(m_lock);
                        MIF_LOG(Info) << "GetText. Creating text in " << m_words.size() << " words.";
                        for (auto const &word : m_words)
                        {
                            if (!text.empty())
                                text += " ";
                            text += word;
                        }
                        MIF_LOG(Info) << "GetText. Created text in " << m_words.size() << " words. Text \"" << text << "\"";
                    }
                    return text;
                }


                virtual void Clean() override final
                {
                    LockGuard lock(m_lock);
                    MIF_LOG(Info) << "Clean. Remove " << m_words.size() << " words.";
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
