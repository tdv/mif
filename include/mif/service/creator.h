//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     01.2017
//  Copyright (C) 2016-2018 tdv
//-------------------------------------------------------------------

#ifndef __MIF_SERVICE_CREATOR_H__
#define __MIF_SERVICE_CREATOR_H__

// BOOST
#include <boost/preprocessor.hpp>
#include <boost/vmd/is_empty.hpp>
#include <boost/vmd/to_seq.hpp>

// MIF
#include "mif/service/create.h"
#include "mif/service/iservice.h"
#include "mif/service/make.h"
#include "mif/service/detail/service.h"

#define MIF_GEN_SERVICE_DECL_PARAM_LIST_IMPL(r_, prm_, I, type_) \
    BOOST_PP_COMMA_IF(I) type_ BOOST_PP_CAT(prm_, I)

#define MIF_GEN_SERVICE_PARAM_LIST_IMPL(r_, prm_, I, type_) \
    BOOST_PP_COMMA_IF(I) std::forward<type_>(BOOST_PP_CAT(prm_, I))

#define MIF_SERVICE_EMPTY_LIST_IMPL(...)

#define MIF_SERVICE_CREATOR(id_, impl_, ... ) \
    namespace Mif \
    { \
        namespace Service \
        { \
            namespace Detail \
            { \
                namespace Creators \
                { \
                    template <> \
                    IServicePtr Create<id_> \
                        ( \
                            BOOST_PP_SEQ_FOR_EACH_I \
                            ( \
                                BOOST_PP_IF \
                                ( \
                                    BOOST_VMD_IS_EMPTY(__VA_ARGS__), \
                                    MIF_SERVICE_EMPTY_LIST_IMPL, \
                                    MIF_GEN_SERVICE_DECL_PARAM_LIST_IMPL \
                                ), \
                                prm_, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__) \
                            ) \
                        ) \
                    { \
                        return Make<impl_> \
                                ( \
                                    BOOST_PP_SEQ_FOR_EACH_I \
                                    ( \
                                        BOOST_PP_IF \
                                        ( \
                                            BOOST_VMD_IS_EMPTY(__VA_ARGS__), \
                                            MIF_SERVICE_EMPTY_LIST_IMPL, \
                                            MIF_GEN_SERVICE_PARAM_LIST_IMPL \
                                        ), \
                                        prm_, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__) \
                                    ) \
                                ); \
                    } \
                } \
            } \
        } \
    }

#endif  // !__MIF_SERVICE_CREATOR_H__
