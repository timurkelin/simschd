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

schd_trace_c::~schd_trace_c(
      void ) {
   if( tf ) {
      sc_core::sc_close_vcd_trace_file( tf );
   }
} // schd_trace_c::~schd_trace_c(

// Trace instance
schd_trace_c simd_trace;

} // namespace schd
