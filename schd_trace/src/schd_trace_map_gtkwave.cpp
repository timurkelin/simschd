/*
 * simd_trace_map_gtkwave.cpp
 *
 *  Description:
 *    Trace map writer for GTKWave
 */

#include <utility>
#include <boost/foreach.hpp>
#include "schd_trace.h"
#include "schd_report.h"

namespace schd {

void schd_trace_c::write_map_gtkwave(
      const boost_pt::ptree& map,
      const std::string&     fname ) {

   SCHD_REPORT_ERROR( "schd::trace" ) << "Unsupported viewer";

} // schd_trace_c::write_map_gtkwave(

} // namespace schd
