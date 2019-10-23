/*
 * schd_trace.h
 *
 *  Description:
 *    VCD trace handler and map generator
 */

#ifndef SCHD_REPORT_INCLUDE_SCHD_TRACE_H_
#define SCHD_REPORT_INCLUDE_SCHD_TRACE_H_

#include <boost/property_tree/ptree.hpp>
#include <boost/optional/optional.hpp>
#include <systemc>

// Short alias for the namespace
namespace boost_pt = boost::property_tree;

namespace schd {
   class schd_trace_c {
   public:
      void init(
            boost::optional<const boost_pt::ptree&> pref_p );

      ~schd_trace_c( void );

      sc_core::sc_trace_file* tf = NULL;

   private:
      void write_map_gtkwave(
            const boost_pt::ptree& map,
            const std::string&     fname );

      void write_map_simvision(
            const boost_pt::ptree& map,
            const std::string&     fname );

   }; // class schd_trace_c

   extern schd_trace_c schd_trace;
} // namespace schd

#endif /* SCHD_REPORT_INCLUDE_SCHD_TRACE_H_ */
