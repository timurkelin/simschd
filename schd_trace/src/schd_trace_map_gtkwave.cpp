/*
 * simd_trace_map_gtkwave.cpp
 *
 *  Description:
 *    Trace map writer for GTKWave
 */

#include <utility>
#include <fstream>
#include <boost/foreach.hpp>
#include "schd_trace.h"
#include "schd_report.h"

namespace schd {

void schd_trace_c::write_map_gtkwave(
      const boost_pt::ptree& map,
      const std::string&     fname ) {

   std::ofstream os;
   std::string   fn_ext = fname + ".trn"; // translate file

   os.exceptions(
         std::ofstream::badbit |
         std::ofstream::failbit );

   try {
      os.open(
            fn_ext.c_str(),
            std::ofstream::out |
            std::ofstream::app );
   }
   catch( const std::ofstream::failure &err ) {
      SCHD_REPORT_ERROR( "simd::trace" ) << err.what();
   }

   BOOST_FOREACH( const boost_pt::ptree::value_type& v, map ) {
      if( !v.first.empty()) {
         SCHD_REPORT_ERROR( "schd::trace" ) << "Incorrect structure";
      }

      std::string st_name_s;
      std::size_t st_name_h;

      // Read name
      try {
         st_name_s = v.second.get<std::string>( "name" );
      }
      catch( const boost_pt::ptree_error& err ) {
         SCHD_REPORT_ERROR( "schd::trace" ) << err.what();
      }
      catch( ... ) {
         SCHD_REPORT_ERROR( "schd::trace" ) << "Unexpected";
      }

      // Process bg color
      boost::optional<std::size_t> st_bgclr_ip = v.second.get_optional<std::size_t>( "bg_color" );
      boost::optional<std::string> st_bgclr_sp = v.second.get_optional<std::string>( "bg_color" );
      std::string st_bgclr_s;

      if( st_bgclr_ip.is_initialized()) {
         st_bgclr_s = rgb2x11name( st_bgclr_ip.get());
      }
      else if( st_bgclr_sp.is_initialized()) {
         x11name2rgb( st_bgclr_sp.get()); // is bg_color refers to a valid x11 color
         st_bgclr_s = st_bgclr_sp.get();
      }



   }

   os.close();

} // schd_trace_c::write_map_gtkwave(

} // namespace schd
