/*
 * schd_trace_map.h
 *
 *  Description:
 *    VCD trace map
 */

#ifndef SCHD_TRACE_INCLUDE_SCHD_TRACE_MAP_H_
#define SCHD_TRACE_INCLUDE_SCHD_TRACE_MAP_H_

#include <string>
#include <vector>
#include <tuple>
#include <boost/property_tree/ptree.hpp>
#include <boost/optional/optional.hpp>
#include <systemc>

// Short alias for the namespace
namespace boost_pt = boost::property_tree;

namespace schd {
   class schd_trace_map_c {
   public:
               schd_trace_map_c( void );
      virtual ~schd_trace_map_c( void ){};

      virtual std::string map_pref( void ) = 0;
      virtual std::string map_elem(
            const std::size_t  _hash,
            const std::string& _view,
            boost::optional<const boost_pt::ptree&> _map_p ) = 0;
      virtual std::string map_suff( void ) = 0;

      std::string file_ext;

protected:
      std::string  rgb2x11name( const std::size_t  rgb_c );
      std::size_t  x11name2rgb( const std::string& rgb_n );

private:
      typedef std::tuple<int, int, int> crgb_t;
      typedef std::map<std::string, schd_trace_map_c::crgb_t> cmap_t;

      crgb_t      rgb_split( const std::size_t rgb_c );
      std::size_t rgb_comb(  const schd_trace_map_c::crgb_t rgb_s );

      cmap_t x11_cmap;

   }; // class schd_trace_map_c
} // namespace schd

#endif /* SCHD_TRACE_INCLUDE_SCHD_TRACE_MAP_H_ */
