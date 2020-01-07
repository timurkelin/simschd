/*
 * schd_cres.h
 *
 *  Description:
 *    Declaration of the common resource module
 */

#ifndef SCHD_CORE_INCLUDE_SCHD_CRES_H_
#define SCHD_CORE_INCLUDE_SCHD_CRES_H_

#include <list>
#include <string>
#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/optional.hpp>
#include <systemc>
#include "schd_sig_ptree.h"

// Short alias for the namespace
namespace boost_pt = boost::property_tree;

namespace schd {

   SC_MODULE( schd_cres_c ) { // declare module class

   public:
      // Module I/O Ports
      sc_core::sc_port<sc_core::sc_fifo_in_if <schd_sig_ptree_c>> exec_i;  // Resource request from exec
      sc_core::sc_port<sc_core::sc_fifo_out_if<schd_sig_ptree_c>> exec_o;  // Demand   update to exec

      // Constructor declaration
      SC_CTOR( schd_cres_c );

      // Init declaration (to be used after the instantiation and before the port binding)
      void init(
            boost::optional<const boost_pt::ptree&> _pref_p,
            boost::optional<const boost_pt::ptree&> _exec_p );

      void add_trace(
            sc_core::sc_trace_file* tf,
            const std::string& top_name );

   private:
      // Process declarations
      void exec_thrd(
            void );

      // List of the execution blocks demanding from the current common resource
      typedef struct {
         bool   connected = false;     // Common resource is connected with the exec
                                       // during the execution of the current task
         double demand    = 0;         // Demand which is requested by the exec from the planner
      } exec_data_t;

      typedef std::map<std::string, exec_data_t> exec_list_t;
      exec_list_t exec_list;

      // Common resource capacity
      double capacity = 0;
      double demand   = 0;

   }; // SC_MODULE( schd_cres_c )
} // namespace schd

#endif /* SCHD_CORE_INCLUDE_SCHD_CRES_H_ */
