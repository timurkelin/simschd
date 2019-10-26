/*
 * simd_conv_ptree.cpp
 *
 *  Description:
 *    Conversion functions for boost property tree
 */

#include <iostream>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/detail/file_parser_error.hpp>
#include "schd_conv_ptree.h"
#include "schd_assert.h"
#include "schd_report.h"

namespace boost_jp = boost::property_tree::json_parser;

namespace schd {

boost_pt::ptree& str2pt(
      const std::string& str_,
      boost_pt::ptree&   pt_ ) {

   std::stringstream is( str_ );

   try {
      boost_pt::read_json( is, pt_ );
   }
   catch( const boost_jp::json_parser_error& err ) {
      SCHD_REPORT_ERROR( "simd::c" ) << err.what();
   }
   catch( ... ) {
      SCHD_REPORT_ERROR( "simd::pref" ) << "Unexpected";
   }

   return pt_;
} // boost_pt::ptree& str2pt(

std::string& pt2str(
      const boost_pt::ptree& pt_,
      std::string&           str_ ) {

   std::stringstream os;

   try {
      boost_pt::write_json( os, pt_ );
   }
   catch( const boost_jp::json_parser_error& err ) {
      SCHD_REPORT_ERROR( "simd::dump" ) << err.what();
   }
   catch( ... ) {
      SCHD_REPORT_ERROR( "simd::dump" ) << "Unexpected";
   }

   str_ = os.str();

   return str_;
} // std::string& pt2str(

} // namespace simd
