/*
 * schd_trace_map_simvision.h
 *
 *  Description:
 *    VCD trace map for SimVision viewer
 */

#ifndef SCHD_TRACE_INCLUDE_SCHD_TRACE_MAP_SIMVISION_H_
#define SCHD_TRACE_INCLUDE_SCHD_TRACE_MAP_SIMVISION_H_

#include "schd_trace_map.h"

// Short alias for the namespace
namespace boost_pt = boost::property_tree;

namespace schd {
   class schd_trace_map_simvision_c
   : public schd_trace_map_c {
   public:
      schd_trace_map_simvision_c( void )
      : schd_trace_map_c() {
         file_ext = ".tcl";
      };

      ~schd_trace_map_simvision_c( void ){};

      std::string map_pref( void );
      std::string map_elem(
            const std::size_t  _hash,
            const std::string& _view,
            boost::optional<const boost_pt::ptree&> _map_p );
      std::string map_suff( void );

   }; // class schd_trace_map_simvision_c
} // namespace schd

#endif /* SCHD_TRACE_INCLUDE_SCHD_TRACE_MAP_SIMVISION_H_ */
