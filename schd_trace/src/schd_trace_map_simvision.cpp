/*
 * simd_trace_map_simvision.cpp
 *
 *  Description:
 *    Trace map writer for Cadence SimVision
 */

#include <utility>
#include <boost/foreach.hpp>
#include "schd_trace.h"
#include "schd_report.h"

namespace schd {

void schd_trace_c::write_map_simvision(
      const boost_pt::ptree& task_list,
      const boost_pt::ptree& map,
      const std::string&     fname ) {

   std::ofstream os;
   std::string   fn_ext = fname + ".tcl"; // TCL file

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

   SCHD_REPORT_ERROR( "schd::trace" ) << "Unsupported viewer";

   os.close();

} // schd_trace_c::write_map_simvision(

} // namespace schd
