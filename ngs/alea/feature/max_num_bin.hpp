/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                 *
 * ALPS Project: Algorithms and Libraries for Physics Simulations                  *
 *                                                                                 *
 * ALPS Libraries                                                                  *
 *                                                                                 *
 * Copyright (C) 2011 - 2012 by Mario Koenz <mkoenz@ethz.ch>                       *
 *                                                                                 *
 * This software is part of the ALPS libraries, published under the ALPS           *
 * Library License; you can use, redistribute it and/or modify it under            *
 * the terms of the license, either version 1 or (at your option) any later        *
 * version.                                                                        *
 *                                                                                 *
 * You should have received a copy of the ALPS Library License along with          *
 * the ALPS Libraries; see the file LICENSE.txt. If not, the license is also       *
 * available from http://alps.comp-phys.org/.                                      *
 *                                                                                 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        *
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT       *
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE       *
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,     *
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER     *
 * DEALINGS IN THE SOFTWARE.                                                       *
 *                                                                                 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#ifndef ALPS_NGS_ALEA_DETAIL_MAX_NUM_BIN_IMPLEMENTATION_HEADER
#define ALPS_NGS_ALEA_DETAIL_MAX_NUM_BIN_IMPLEMENTATION_HEADER

#include <alps/ngs/alea/accumulator/accumulator_impl.hpp>
#include <alps/ngs/alea/features.hpp>

#include <vector>
#include <ostream>
#include <cmath>
#include <algorithm>

namespace alps
{
    namespace alea
    {
        //=================== max_num_bin proxy ===================
        template<typename value_type>
        class max_num_bin_proxy_type
        {
            typedef typename mean_type<value_type>::type mean_type;
            typedef typename std::vector<value_type>::size_type size_type;
            static std::vector<mean_type> unused;
        public:
            max_num_bin_proxy_type(): bin_(unused) {}
            
            max_num_bin_proxy_type(  std::vector<mean_type> const & bin
                                      , size_type const & bin_number):
                                                                  bin_(bin)
                                                                , bin_number_(bin_number)
            {}
            
            inline std::vector<mean_type> const & bins() const 
            {
                return bin_;
            }
            
            inline size_type const & bin_number() const
            {
                return bin_number_;
            }
            
            template<typename T>
            friend std::ostream & operator<<(std::ostream & os, max_num_bin_proxy_type<T> const & arg);
        private:
            std::vector<mean_type> const & bin_;
            size_type bin_number_;
        };

        template<typename T>
        inline std::ostream & operator<<(std::ostream & os, max_num_bin_proxy_type<T> const & arg)
        {
            os << "max_num_bin_proxy" << std::endl;
            return os;
        };
        //=================== max_num_bin trait ===================
        template <typename T>
        struct max_num_bin_type
        {
            typedef max_num_bin_proxy_type<T> type;
        };
        //=================== max_num_bin implementation ===================
        namespace detail
        {
            //set up the dependencies for the tag::max_num_binning-Implementation
            template<>
            struct Dependencies<tag::max_num_binning> 
            {
                typedef MakeList<tag::mean, tag::error>::type type;
            };

            template<typename base_type> 
            class Implementation<tag::max_num_binning, base_type> : public base_type 
            {
                typedef typename base_type::value_type value_type_loc;
                typedef typename max_num_bin_type<value_type_loc>::type num_bin_type;
                typedef typename std::vector<value_type_loc>::size_type size_type;
                typedef typename mean_type<value_type_loc>::type mean_type;
                typedef Implementation<tag::max_num_binning, base_type> ThisType;

                public:
                    Implementation<tag::max_num_binning, base_type>(ThisType const & arg): base_type(arg)
                                                                  , bin_(arg.bin_)
                                                                  , partial_(arg.partial_)
                                                                  , elements_in_bin_(arg.elements_in_bin_)
                                                                  , pos_in_partial_(arg.pos_in_partial_)
                                                                  , max_bin_num_(arg.max_bin_num_) 
                    {}
                    //TODO: set right default value 
                    
                    template<typename ArgumentPack>
                    Implementation<tag::max_num_binning, base_type>(ArgumentPack const & args
                                                , typename boost::disable_if<
                                                                              boost::is_base_of<ThisType, ArgumentPack>
                                                                            , int
                                                                            >::type = 0
                                               ): base_type(args)
                                                , partial_()
                                                , elements_in_bin_(1)
                                                , pos_in_partial_(0)
                                                , max_bin_num_(args[bin_num | 128]) //change doc if manipulated
                    {}
                    
                    inline num_bin_type const max_num_bin() const 
                    { 
                        return max_num_bin_proxy_type<value_type_loc>(bin_, max_bin_num_);
                    }
              
                    inline ThisType& operator <<(value_type_loc val)
                    {
                        using namespace alps::ngs::numeric;
                        
                        base_type::operator <<(val);
                        
                        partial_ = partial_ + val;
                        ++pos_in_partial_;
                        
                        if(pos_in_partial_ == elements_in_bin_)
                        {
                            if(bin_.size() >= max_bin_num_)
                            {
                                if(max_bin_num_ % 2 == 1)
                                {
                                    partial_ += bin_[max_bin_num_ - 1];
                                    pos_in_partial_ += elements_in_bin_;
                                }
                                
                                for(unsigned int i = 0; i < max_bin_num_ / 2; ++i) //the rounding down here is intentional
                                    bin_[i] = (bin_[2*i] + bin_[2*i + 1])/2;
                                
                                bin_.erase(bin_.begin() + max_bin_num_ / 2, bin_.end());
                                
                                elements_in_bin_ *= 2;
                                
                                if(pos_in_partial_ == elements_in_bin_)
                                {
                                    bin_.push_back(partial_ / elements_in_bin_);
                                    partial_ = value_type_loc();
                                    pos_in_partial_ = 0;
                                }
                            }
                            else
                            {
                                bin_.push_back(partial_ / elements_in_bin_);
                                partial_ = value_type_loc();
                                pos_in_partial_ = 0;
                            }
                        }
                        return *this;
                    }
              
                    template<typename Stream> 
                    inline void print(Stream & os) 
                    {
                        base_type::print(os);
                        os << "MaxBinningNumber: MaxBinNumber: " << max_bin_num_ << std::endl;
                        
                        //~ os << std::endl;
                        //~ for (unsigned int i = 0; i < bin_.size(); ++i)
                        //~ {
                            //~ os << "bin[" << i << "] = " << bin_[i] << std::endl;
                        //~ }
                    }
              
                private:
                    std::vector<mean_type> bin_;
                    value_type_loc partial_;
                    size_type elements_in_bin_;
                    size_type pos_in_partial_;
                    size_type max_bin_num_;
            };
        } // end namespace detail
    }//end alea namespace 
}//end alps namespace
#endif // ALPS_NGS_ALEA_DETAIL_MAX_NUM_BIN_IMPLEMENTATION
