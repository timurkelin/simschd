/*
 * schd_planner.h
 *
 *  Description:
 *    Declaration of the planner class
 */

#ifndef SCHD_CORE_INCLUDE_SCHD_PLANNER_H_
#define SCHD_CORE_INCLUDE_SCHD_PLANNER_H_

#include <vector>
#include <string>
#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/optional.hpp>
#include <boost/regex.hpp>
#include <systemc>
#include "schd_sig_ptree.h"

// Short alias for the namespace
namespace boost_pt = boost::property_tree;

namespace schd {

   SC_MODULE( schd_planner_c ) { // declare module class

   public:
      // Module I/O Ports
      sc_core::sc_port<sc_core::sc_fifo_in_if <schd_sig_ptree_c>> core_i;
      sc_core::sc_port<sc_core::sc_fifo_out_if<schd_sig_ptree_c>> core_o;

      // Constructor declaration
      SC_CTOR( schd_planner_c );

      // Init declaration (to be used after the instantiation and before the port binding)
      void init(
            boost::optional<const boost_pt::ptree&> _thrd_p,   // Threads
            boost::optional<const boost_pt::ptree&> _task_p,   // Tasks
            boost::optional<const boost_pt::ptree&> _exec_p ); // execution units

   private:
      // Process declarations
      void exec_thrd(
            void );

      // Forward declarations
      class task_data_t;
      typedef std::map<std::string, task_data_t> task_list_t;

      class thrd_data_t;
      typedef std::map<std::string, thrd_data_t> thrd_list_t;

      class event_data_t;
      typedef std::list<event_data_t> event_reg_t;

      // List of the execution blocks
      class exec_data_t {
      public:
         boost::optional<const thrd_list_t::value_type&> thrd_p;  // Caller thread.
         boost::optional<const task_list_t::value_type&> task_p;  // Caller task. "" means the exec is free
         boost::optional<const boost_pt::ptree&>         param_p; // parameters
         sc_core::sc_time  time_start = sc_core::SC_ZERO_TIME;    // start time
         sc_core::sc_time  time_end   = sc_core::SC_ZERO_TIME;    // end   time
      };

      typedef std::map<std::string, exec_data_t> exec_list_t;

      // Task list
      class task_run_el_t {
      public:
         boost::regex        mask_exec;
         boost_pt::ptree     options;
         std::vector<double> cres_demand;
      };

      class task_data_t {
      public:
         sc_core::sc_time           run_time = sc_core::SC_ZERO_TIME;
         std::vector<task_run_el_t> run_list;
         std::vector<std::string>   cres_list;
      };

      // thread list
      class thrd_seq_el_t {
      public:
         std::string                                     name = "";
         boost::optional<const task_list_t::value_type&> task_p;        // Also used as a task/event flag
         boost_pt::ptree                                 task_param;
      };

      typedef enum {
         SEQ_STATE_IDLE    = 0,
         SEQ_STATE_WAITING = 1,
         SEQ_STATE_RUNNING = 2
      } thrd_seq_el_state_t;

      class thrd_data_t {
      public:
         double                     priority = 0.0;                   // priority of the called instance in this thread
         std::vector<boost::regex>  mask_evnt_list;                   // "and" list of start event masks
         std::vector<thrd_seq_el_t> seq_list;                         // sequence of calls
         std::size_t                seq_idx = 0;                      // index in the sequence list
         thrd_seq_el_state_t        seq_state = SEQ_STATE_IDLE;       // State of the sequence list processing

         // list of events which started this thread
         std::list<boost::optional<      event_reg_t::value_type&>> evnt_list;

         // list of the execution blocks which are processing this thread
         std::list<boost::optional<const exec_list_t::value_type&>> exec_list;
      };

      // event register
      class event_data_t {
      public:
         // event and thread name
         boost::optional<const std::string&>              name_p;
         boost::optional<const thrd_list_t::value_type&>  thrd_caller;

         // event occurrence time
         sc_core::sc_time  time = sc_core::SC_ZERO_TIME;

         // list of the threads which were started with this event
         std::list<boost::optional<const thrd_list_t::value_type&>> thrd_run_list;

         // number of the threads which were started with this event and finished
         std::size_t thrd_end_cntr = 0;

         // list of the threads which don't use this event name
         std::list<boost::optional<const thrd_list_t::value_type&>> thrd_msm_list;

         bool remove = false;
      };

      event_reg_t event_reg;
      exec_list_t exec_list;
      task_list_t task_list;
      thrd_list_t thrd_list;

      bool and_list(
            std::vector<boost::optional<const boost::regex&>>& msk,  // regex masks
            std::vector<boost::optional<const std::string&>>&  val,  // check values
            std::vector<std::size_t>& msk_val_map,                   // matching value for each mask
            std::vector<std::size_t>& val_msk_cnt );                 // Number of masks matching each value

      // processing wait list
      class wait_data_t {
      public:
         double priority;
         boost::optional<      thrd_list_t::value_type&> thrd_p;
         boost::optional<const task_list_t::value_type&> task_p;
         std::size_t mask_exec_base = 0;
         std::size_t mask_exec_size = 0;
      };

      typedef std::list<wait_data_t> wait_list_t;
   }; // SC_MODULE( schd_planner_c )
} // namespace schd

#endif /* SCHD_CORE_INCLUDE_SCHD_PLANNER_H_ */
