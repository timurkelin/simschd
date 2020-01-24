/*
 * schd_exec.h
 *
 *  Description:
 *    Declaration of the execution unit
 */

#ifndef SCHD_CORE_INCLUDE_SCHD_EXEC_H_
#define SCHD_CORE_INCLUDE_SCHD_EXEC_H_

#include <string>
#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/optional.hpp>
#include <systemc>
#include "schd_sig_ptree.h"

// Short alias for the namespace
namespace boost_pt = boost::property_tree;

namespace schd {

   SC_MODULE( schd_exec_c ) { // declare module class

   public:
      // Module I/O Ports
      sc_core::sc_port<sc_core::sc_fifo_in_if <schd_sig_ptree_c>> plan_i;
      sc_core::sc_port<sc_core::sc_fifo_out_if<schd_sig_ptree_c>> plan_o;
      sc_core::sc_port<sc_core::sc_fifo_in_if <schd_sig_ptree_c>> cres_i;
      sc_core::sc_port<sc_core::sc_fifo_out_if<schd_sig_ptree_c>> cres_o;

      // Constructor declaration
      SC_CTOR( schd_exec_c );

      // Init declaration (to be used after the instantiation and before the port binding)
      void init(
            boost::optional<const boost_pt::ptree&> _exec_p,      // parameters for the exec module
            boost::optional<const boost_pt::ptree&> _cres_p );    // parameters for all cres blocks

      void add_trace(
            sc_core::sc_trace_file* tf,
            const std::string& top_name );

   private:
      // Process declarations
      void exec_thrd(
            void );

      sc_core::sc_time time_upd   = sc_core::SC_ZERO_TIME;  // Time of the most recent update
      sc_core::sc_time time_to_go = sc_core::SC_ZERO_TIME;  // Time to go after the last update
      double           time_ext_coe = 1.0;                  // Time extension coefficient

      std::string      thrd_name;
      std::string      task_name;
      std::string      param_id;

      typedef enum : unsigned char {
         CRES_STATE_IDLE      = 0,  // Not connected to the exec
         CRES_STATE_CONNECTED = 1,  //     Connected to the exec
         CRES_STATE_WAIT_DCN  = 2,  // Waiting to be disconnected
         CRES_STATE_WAIT_CON  = 3,  // Waiting to be    connected
      } cres_state_t;

      // List of the common resources used in the execution of the current task
      typedef struct {
         double       capacity    = 0.0;              // Common resource Capacity
         bool         connected   = false;
         cres_state_t state       = CRES_STATE_IDLE;  // Common resource is connected with the exec
                                                      // during the execution of the current task
         double       cres_demand = 0.0;              // Total demand reported by the common resource
         double       cres_load   = 0.0;              // Total load = cres_demand / capacity
         double       plan_demand = 0.0;              // Demand which is requested for the exec from the planner
         double       exec_demand = 0.0;              // Demand which is calculated by combining data from all the resources
                                                      // involved in the task execution.
      } cres_data_t;

      typedef std::map<std::string, cres_data_t> cres_list_t;
      cres_list_t cres_list;

      sc_core::sc_event exec_complete; // Complete execution

      std::size_t job_hash = 0;

   }; // SC_MODULE( schd_ptree_xbar_c )
} // namespace schd

#endif /* SCHD_CORE_INCLUDE_SCHD_EXEC_H_ */
