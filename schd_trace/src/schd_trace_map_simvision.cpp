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
      const boost_pt::ptree& map,
      const std::string&     fname ) {

   SCHD_REPORT_ERROR( "schd::trace" ) << "Unsupported viewer";

} // schd_trace_c::write_map_simvision(

} // namespace schd
