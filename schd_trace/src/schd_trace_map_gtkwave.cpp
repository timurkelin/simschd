/*
 * simd_trace_map_gtkwave.cpp
 *
 *  Description:
 *    Trace map writer for GTKWave
 */

#include <utility>
#include <iomanip>
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/functional/hash.hpp>
#include "schd_trace_map_gtkwave.h"
#include "schd_report.h"

namespace schd {

std::string schd_trace_map_gtkwave_c::map_pref(
      void ) {
   return "";
} // schd_trace_map_gtkwave_c::map_pref(

std::string schd_trace_map_gtkwave_c::map_elem(
      const std::size_t  _hash,
      const std::string& _view,
      boost::optional<const boost_pt::ptree&> _map_p ) {

   // Process background color
   boost::optional<std::string> st_bgclr_sp;

   if( _map_p.is_initialized()) {
      boost::optional<std::size_t> st_bgclr_ip = _map_p.get().get_optional<std::size_t>( "bgcolor" );
                                   st_bgclr_sp = _map_p.get().get_optional<std::string>( "bgcolor" );

      if( st_bgclr_ip.is_initialized()) {
         st_bgclr_sp = rgb2x11name( st_bgclr_ip.get());
      }
      else if( st_bgclr_sp.is_initialized() && st_bgclr_sp.get().size() != 0 ) {
         x11name2rgb( st_bgclr_sp.get()); // check if bgcolor refers to a valid x11 colour name
      }
      else if( st_bgclr_sp.is_initialized() && st_bgclr_sp.get().size() == 0 ) {
         ;
      }
      else {
         SCHD_REPORT_ERROR( "schd::trace" ) << "Incorrect format";
      }
   }

   std::stringstream os;
   os << std::setfill('0')
      << std::setw( sizeof( _hash ) * 2 )
      << std::hex
      << _hash;

   if( st_bgclr_sp.get().size()) {
      return os.str() + " ?" + st_bgclr_sp.get() + "?" + _view + "\n";
   }
   else {
      return os.str() +  " " + _view + "\n";
   }
} // schd_trace_map_gtkwave_c::map_elem(

std::string schd_trace_map_gtkwave_c::map_suff(
      void ) {
   return "";
} // schd_trace_map_gtkwave_c::map_suff(

} // namespace schd
