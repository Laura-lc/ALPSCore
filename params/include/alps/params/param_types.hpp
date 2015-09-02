/*
 * Copyright (C) 1998-2015 ALPS Collaboration. See COPYRIGHT.TXT
 * All rights reserved. Use is subject to license terms. See LICENSE.TXT
 * For use in publications, see ACKNOWLEDGE.TXT
 */
#ifndef ALPS_PARAMS_PARAM_TYPES_INCLUDED
#define ALPS_PARAMS_PARAM_TYPES_INCLUDED

/* Using Boost preprocessor macros rather than MPL. */

#include <stdexcept>
#include <vector>
#include <iostream>

#include "boost/preprocessor/array/to_seq.hpp"
#include "boost/preprocessor/seq/transform.hpp"
#include "boost/preprocessor/seq/enum.hpp"
#include "boost/preprocessor/seq/for_each.hpp"

#include "boost/variant.hpp"
#include "boost/optional.hpp"

// FIXME: will go away with acceptance of boost::TypeIndex
#include "alps/params/typeindex.hpp"

namespace alps {
    namespace params_ns {
        namespace detail {
            
	    // Allowed basic numerical types.
	    // NOTE 1: do not forget to change "7" to the correct number if editing!
	    // NOTE 2: currently, not more than (20-2)/2 = 9 types are supported
	    //         (20 is boost::variant limit; we have std::vector<T> for each of
	    //         these basic types, plus std::string and None.)
#define	    ALPS_PARAMS_DETAIL_STYPES_VEC (7,(int, \
					      unsigned int,	\
					      double,		\
					      long int,		\
					      unsigned long int,	\
					      char,			\
					      bool))

            /// "Empty value" type
            struct None {};

            /// Output operator for the "empty value" (@throws runtime_error always)
            inline std::ostream& operator<<(std::ostream& s, const None&)
            {
                throw std::runtime_error("Attempt to print uninitialized option value");
            }


	    // BOOST-PP Sequence of numerical types types
#define     ALPS_PARAMS_DETAIL_STYPES_SEQ BOOST_PP_ARRAY_TO_SEQ(ALPS_PARAMS_DETAIL_STYPES_VEC)

	    // Macro to make derived types
#define     ALPS_PARAMS_DETAIL_MAKE_TYPE(s,atype,elem) atype< elem >

            // Sequence of std::vector<T> types (aka "vector types")
#define     ALPS_PARAMS_DETAIL_VTYPES_SEQ BOOST_PP_SEQ_TRANSFORM(ALPS_PARAMS_DETAIL_MAKE_TYPE, std::vector, ALPS_PARAMS_DETAIL_STYPES_SEQ)
	  
	    // Make a sequence of all parameter types (for boost::variant), including std::string
#define     ALPS_PARAMS_DETAIL_ALLTYPES_SEQ ALPS_PARAMS_DETAIL_STYPES_SEQ(std::string)ALPS_PARAMS_DETAIL_VTYPES_SEQ

            // FIXME: will go away with acceptance of boost::TypeIndex
            // Generate a pretty-name specialization for scalar and vector types
#define     ALPS_PARAMS_DETAIL_GEN_TYPID(s, data, elem) ALPS_PARAMS_DETAIL_TYPID_NAME(elem)
            BOOST_PP_SEQ_FOR_EACH(ALPS_PARAMS_DETAIL_GEN_TYPID,~,ALPS_PARAMS_DETAIL_ALLTYPES_SEQ);
            // Generate a few more pretty-names, for frequently-needed types
            ALPS_PARAMS_DETAIL_TYPID_NAME(char *);
            ALPS_PARAMS_DETAIL_TYPID_NAME(const char *);
            
          
            /// A variant of all types, including None (as the first one -- important!)
	    typedef boost::variant< None, BOOST_PP_SEQ_ENUM(ALPS_PARAMS_DETAIL_ALLTYPES_SEQ) > variant_all_type;

            // Sequence of `boost::optional<T>` types for scalar and vector all types (except None)
#define     ALPS_PARAMS_DETAIL_OTYPES_SEQ BOOST_PP_SEQ_TRANSFORM(ALPS_PARAMS_DETAIL_MAKE_TYPE, boost::optional, ALPS_PARAMS_DETAIL_ALLTYPES_SEQ)

            /// An output operator for optionals of any type (throws unconditionally)
            template <typename T>
            inline std::ostream& operator<<(std::ostream& , const boost::optional<T>&)
            {
                throw std::logic_error("Attempt to use undefined operator<< for boost::optional<T>");
            }

            /// Tag type to indicate "trigger" option type (FIXME: it's a hack and must be redone)
            struct trigger_tag {};

            inline std::ostream& operator<<(std::ostream&, const trigger_tag&)
            {
                throw std::logic_error("Attempt to use undefined operator<< for trigger_tag");
            }

            /// A variant of the trigger_tag and optionals of all types
            typedef boost::variant<BOOST_PP_SEQ_ENUM(ALPS_PARAMS_DETAIL_OTYPES_SEQ), trigger_tag> variant_all_optional_type;
        }

        // Elevate choosen generated types:
        using detail::variant_all_type;

	// Undefine local macros
#undef  ALPS_PARAMS_DETAIL_STYPES_VEC
#undef  ALPS_PARAMS_DETAIL_STYPES_SEQ
#undef  ALPS_PARAMS_DETAIL_MAKE_TYPE
#undef  ALPS_PARAMS_DETAIL_VTYPES_SEQ
#undef  ALPS_PARAMS_DETAIL_ALLTYPES_SEQ
#undef  ALPS_PARAMS_DETAIL_GEN_TYPID
#undef  ALPS_PARAMS_DETAIL_OTYPES_SEQ
	
    } // params_ns
}// alps

// The following is needed for serialization support
namespace boost {
    namespace serialization {
        /// Serialization function for the "empty value" (does nothing)
        template<class Archive>
        inline void serialize(Archive & ar, alps::params_ns::detail::None&, const unsigned int)
        { }

        /// Serialization function for the "trigger type" (does nothing)
        template<class Archive>
        inline void serialize(Archive & ar, alps::params_ns::detail::trigger_tag&, const unsigned int)
        { }
    } // serialization
} // boost


#endif // ALPS_PARAMS_PARAM_TYPES_INCLUDED
