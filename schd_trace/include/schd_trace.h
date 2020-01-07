/*
 * schd_trace.h
 *
 *  Description:
 *    VCD trace handler and map generator
 */

#ifndef SCHD_TRACE_INCLUDE_SCHD_TRACE_H_
#define SCHD_TRACE_INCLUDE_SCHD_TRACE_H_

#include <string>
#include <vector>
#include <tuple>
#include <boost/property_tree/ptree.hpp>
#include <boost/optional/optional.hpp>
#include <systemc>
#include "schd_trace_map.h"

// Short alias for the namespace
namespace boost_pt = boost::property_tree;

namespace schd {
   class schd_trace_c {
   public:
      void init(
            boost::optional<const boost_pt::ptree&> _pref_p );

      void save_map(
            boost::optional<const boost_pt::ptree&> _thrd_p );

      std::string job_comb(
            const std::string& _thrd_str,
            const std::string& _task_str,
            const std::string& _param_str );

      ~schd_trace_c( void );

      sc_core::sc_trace_file* tf = NULL;

   private:
      boost::optional<schd_trace_map_c&>      trace_map_p;
      boost::optional<const boost_pt::ptree&> pref_p;

      typedef struct {
         boost::optional<std::string> thrd_name_p;
         boost::optional<std::string> task_name_p;
         boost::optional<std::string> task_prid_p;
         std::string comb;
         std::size_t hash;
      } tag_data_t;

      typedef std::list<tag_data_t> tag_list_t;
   }; // class schd_trace_c

   extern schd_trace_c schd_trace;
} // namespace schd

#endif /* SCHD_TRACE_INCLUDE_SCHD_TRACE_H_ */
