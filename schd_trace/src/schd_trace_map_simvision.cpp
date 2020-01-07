/*
 * simd_trace_map_simvision.cpp
 *
 *  Description:
 *    Trace map writer for Cadence SimVision
 */

#include <utility>
#include <iomanip>
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/functional/hash.hpp>
#include "schd_trace_map_simvision.h"
#include "schd_report.h"

namespace schd {

std::string schd_trace_map_simvision_c::map_pref(
      void ) {
   return "mmap new -reuse -name schd -radix %x -contents {\n";
} // schd_trace_map_simvision_c::map_pref(

std::string schd_trace_map_simvision_c::map_elem(
      const std::size_t  _hash,
      const std::string& _view,
      boost::optional<const boost_pt::ptree&> _map_p ) {

   // Process background color and other options
   boost::optional<std::size_t> st_bgclr_ip;
   boost::optional<std::string> st_opt_sp;

   if( _map_p.is_initialized()) {
                                   st_bgclr_ip = _map_p.get().get_optional<std::size_t>( "bgcolor" );
      boost::optional<std::string> st_bgclr_sp = _map_p.get().get_optional<std::string>( "bgcolor" );

      if(  st_bgclr_ip.is_initialized() ||
         ( st_bgclr_sp.is_initialized() && st_bgclr_sp.get().size() == 0 )) {
         ;
      }
      else if( st_bgclr_sp.is_initialized() && st_bgclr_sp.get().size() != 0 ) {
         st_bgclr_ip = x11name2rgb( st_bgclr_sp.get()); // bg_color refers to a valid x11 color name
      }
      else {
         SCHD_REPORT_ERROR( "schd::trace" ) << "Incorrect format";
      }

      st_opt_sp = _map_p.get().get_optional<std::string>( "opt" );
   }

   std::stringstream os;

   os << "{%x="
      << std::setfill('0')
      << std::setw( sizeof( _hash ) * 2 )
      << std::hex
      << _hash
      << " -label {"
      << _view
      << "}";

   if( st_bgclr_ip.is_initialized()) {
      os << " -bgcolor #"
         << std::setfill('0')
         << std::setw( 6 )
         << std::hex
         << ( st_bgclr_ip.get() & 0xffffff );
   }

   if( st_opt_sp.is_initialized() && ( st_opt_sp.get().size() != 0 )) {
      os << " "
         << st_opt_sp.get();
   }

   os << "}"
      << std::endl;

   return os.str();
} // schd_trace_map_simvision_c::map_elem(

std::string schd_trace_map_simvision_c::map_suff(
      void ) {
   return "}\n";
} // schd_trace_map_simvision_c::map_suff(

} // namespace schd
