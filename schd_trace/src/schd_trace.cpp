/*
 * simd_trace.cpp
 *
 *  Description:
 *    VCD trace and map
 */

#include <utility>
#include <boost/foreach.hpp>
#include "schd_trace.h"
#include "schd_report.h"

namespace schd {

void schd_trace_c::init(
      boost::optional<const boost_pt::ptree&> pref_p ) {

   const boost_pt::ptree& pref = pref_p.get();

   boost::optional<std::string> trace_p  = pref.get_optional<std::string>("trace_file");

   // Set vcd file name
   if(( !trace_p ) || ( trace_p.get().length() == 0 )) {
      SCHD_REPORT_INFO( "schd::trace" ) << "VCD trace is disabled";
      tf = NULL;
   }
   else {
      tf = sc_core::sc_create_vcd_trace_file( trace_p.get().c_str() );

      if( tf ) {
         SCHD_REPORT_INFO(  "schd::trace" ) << "VCD trace is set to <" << trace_p.get() << ".vcd>";
      }
      else {
         SCHD_REPORT_ERROR( "schd::trace" ) << "Unable to set VCD trace <" << trace_p.get() << ".vcd>";
      }

      std::string viewer;

      try {
       viewer = pref.get<std::string>("viewer");
      }
      catch( const boost_pt::ptree_error& err ) {
         SCHD_REPORT_ERROR( "schd::trace" ) << err.what();
      }
      catch( ... ) {
         SCHD_REPORT_ERROR( "schd::trace" ) << "Unexpected";
      }


      // Create viewer map
      if( viewer != "none" ) {

         const boost_pt::ptree &map = [&]() {
           try {
              return pref.get_child("map");
           }
           catch( const boost_pt::ptree_error& err ) {
              SCHD_REPORT_ERROR( "schd::trace" ) << err.what();
           }
           catch( const std::exception& err ) {
              SCHD_REPORT_ERROR( "schd::trace" ) << err.what();
           }
           catch( ... ) {
              SCHD_REPORT_ERROR( "schd::trace" ) << " Unexpected";
           }

           return pref.get_child("map"); // this is unreachable
         }(); // iife

         if( viewer == "gtkwave" ) { // GTK Wave
            write_map_gtkwave(
                 map,
                 trace_p.get() );
         }
         else if( viewer == "simvision" ) { // Cadence SimVision
            write_map_simvision(
                 map,
                 trace_p.get() );
         }
         else {
            SCHD_REPORT_ERROR( "schd::trace" ) << "Unsupported viewer";
         }
      }
   }
} // schd_trace_c::init(

schd_trace_c::crgb_t schd_trace_c::rgb_split( const std::size_t rgb_c ) {
   return std::make_tuple(
      ( rgb_c >> 16 ) & 0xff,
      ( rgb_c >>  8 ) & 0xff,
      ( rgb_c       ) & 0xff );
}

std::size_t schd_trace_c::rgb_comb(  const schd_trace_c::crgb_t rgb_s ) {
   return ( std::get<0>( rgb_s ) << 16 ) |
          ( std::get<1>( rgb_s ) <<  8 ) |
          ( std::get<2>( rgb_s ));
}

std::string schd_trace_c::rgb2x11name( const std::size_t  rgb_c ) {
   crgb_t rgb_s = rgb_split( rgb_c );

   boost::optional<cmap_t> min_el_p;
   std::size_t             min_se = UINT_MAX;

   BOOST_FOREACH( const cmap_t& cel, cmap ) {
      int dist_r = std::get<0>( std::get<1>( cel )) - std::get<0>( rgb_s );
      int dist_g = std::get<1>( std::get<1>( cel )) - std::get<1>( rgb_s );
      int dist_b = std::get<2>( std::get<1>( cel )) - std::get<2>( rgb_s );

      std::size_t se = dist_r * dist_r +
                       dist_g * dist_g +
                       dist_b * dist_b;

      if( se == 0 ) {
         min_el_p = cel;
         break;
      }
      else {
         if( min_se > se ) {
            min_se = se;
            min_el_p = cel;
         }
      }
   }

   return std::get<0>( min_el_p.get() );
}

std::size_t schd_trace_c::x11name2rgb( const std::string& rgb_n ) {
   crgb_t rgb_s = std::make_tuple( 0, 0, 0 );
   bool   found = false;

   BOOST_FOREACH( const cmap_t& cel, cmap ) {
      if( std::get<0>( cel ) == rgb_n ) {
         rgb_s = std::get<1>( cel );
         found = true;
         break;
      }
   }

   if( !found ) {
      SCHD_REPORT_ERROR( "schd::trace" ) << "color name not found: " << rgb_n;
   }

   return rgb_comb( rgb_s );
}

schd_trace_c::~schd_trace_c(
      void ) {
   if( tf ) {
      sc_core::sc_close_vcd_trace_file( tf );
   }
} // schd_trace_c::~schd_trace_c(

// Trace instance
schd_trace_c simd_trace;

} // namespace schd
