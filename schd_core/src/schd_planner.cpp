/*
 * schd_planner.cpp
 *
 *  Description:
 *    Planner
 */

#include <algorithm>
#include <iterator>
#include <limits>
#include <utility>
#include <boost/foreach.hpp>
#include "schd_planner.h"
#include "schd_ptree_time.h"
#include "schd_conv_ptree.h"
#include "schd_dump.h"
#include "schd_assert.h"
#include "schd_report.h"

namespace schd {

SC_HAS_PROCESS( schd::schd_planner_c );
schd_planner_c::schd_planner_c(
      sc_core::sc_module_name nm )
   : sc_core::sc_module( nm )
   , core_i( "core_i" )
   , core_o( "core_o" ) {

   // Process registrations
   SC_THREAD( exec_thrd );
}

void schd_planner_c::init(
      boost::optional<const boost_pt::ptree&> _thrd_p,
      boost::optional<const boost_pt::ptree&> _task_p,
      boost::optional<const boost_pt::ptree&> _exec_p ) {

   // Create a list of exec instances
   BOOST_FOREACH( const boost_pt::ptree::value_type& exec_el, _exec_p.get()) {
      if( !exec_el.first.empty()) {
         SCHD_REPORT_ERROR( "schd::plan" ) << name() <<  " Incorrect exec structure";
      }

      boost::optional<std::string> name_p = exec_el.second.get_optional<std::string>("name");

      if( !name_p.is_initialized()) {
         SCHD_REPORT_ERROR( "schd::plan" ) << name() <<  " Incorrect exec structure";
      }

      // Initialize exec data
      exec_data_t exec_data;

      exec_list.emplace( std::make_pair( name_p.get(), exec_data ));
   } // BOOST_FOREACH( const boost_pt::ptree::value_type& exec_el, _exec_p.get())

   // Task list
   BOOST_FOREACH( const boost_pt::ptree::value_type& task_el, _task_p.get()) {
      if( !task_el.first.empty()) {
         SCHD_REPORT_ERROR( "schd::plan" ) << name() <<  " Incorrect task structure";
      }

      boost::optional<std::string>            name_p = task_el.second.get_optional<std::string>("name");
      boost::optional<sc_core::sc_time>       runt_p = task_el.second.get_optional<sc_core::sc_time>("runtime");
      boost::optional<const boost_pt::ptree&> exec_p = task_el.second.get_child_optional("exec");

      if( !name_p.is_initialized() ||
          !runt_p.is_initialized() ||
          !exec_p.is_initialized()) {
         SCHD_REPORT_ERROR( "schd::plan" ) << name() <<  " Incorrect task structure";
      }

      task_data_t            task_data;
      std::list<std::string> task_cres_list;

      task_data.run_time = runt_p.get();

      BOOST_FOREACH( const boost_pt::ptree::value_type& exec_el, exec_p.get()) {
         if( !exec_el.first.empty()) {
            SCHD_REPORT_ERROR( "schd::plan" ) << name() <<  " Incorrect task structure " << name_p.get();
         }

         boost::optional<std::string>            mask_p = exec_el.second.get_optional<std::string>("run");
         boost::optional<const boost_pt::ptree&> cres_p = exec_el.second.get_child_optional("use");
         boost::optional<const boost_pt::ptree&> optn_p = exec_el.second.get_child_optional("opt");

         if( !mask_p.is_initialized() ||
             !cres_p.is_initialized() ||
             !optn_p.is_initialized()) {
            SCHD_REPORT_ERROR( "schd::plan" ) << name() <<  " Incorrect task structure " << name_p.get();
         }

         task_run_el_t task_run_el;

         try {
            task_run_el.mask_exec = mask_p.get();
         }
         catch( const boost::regex_error& err ) {
            SCHD_REPORT_ERROR( "schd::plan" ) << err.what();
         }
         catch( ... ) {
            SCHD_REPORT_ERROR( "schd::plan" ) << "Unexpected";
         }

         // Check if at least 1 exec is present for this mask
         bool exec_avail= false;
         BOOST_FOREACH( const exec_list_t::value_type& elst_el, exec_list ) {
            if( boost::regex_match( elst_el.first, task_run_el.mask_exec )) {
               exec_avail = true;
               break;
            }
         }

         if( !exec_avail ) {
            SCHD_REPORT_ERROR( "schd::plan" ) << name() <<  " exec is not found for the task " << name_p.get();
         }

         // Update the list of common resources for the task
         BOOST_FOREACH( const boost_pt::ptree::value_type& cres_el, cres_p.get()) {
            if( !cres_el.first.empty()) {
               SCHD_REPORT_ERROR( "schd::plan" ) << name() <<  " Incorrect task structure " << name_p.get();
            }

            boost::optional<std::string> crnm_p = cres_el.second.get_optional<std::string>("res");

            if( task_cres_list.end() == std::find(
                  task_cres_list.begin(),
                  task_cres_list.end(),
                  crnm_p.get())) {
               task_cres_list.push_back( crnm_p.get());
            }
         } // BOOST_FOREACH( const boost_pt::ptree::value_type& cres_el, cres_p.get())

         // Save options
         task_run_el.options = optn_p.get();

         task_data.run_list.push_back( task_run_el );
      } // BOOST_FOREACH( const boost_pt::ptree::value_type& exec_el, exec_p.get())

      // Set cres-related fields in the task list
      task_data.cres_list.resize( task_cres_list.size());

      std::size_t cres_list_idx = 0;
      BOOST_FOREACH( const std::string& cres_el, task_cres_list) {
         task_data.cres_list.at( cres_list_idx ) = std::move( cres_el );
         cres_list_idx ++;
      }

      std::size_t run_list_idx = 0;
      BOOST_FOREACH( const boost_pt::ptree::value_type& exec_el, exec_p.get()) {
         task_data.run_list.at( run_list_idx ).cres_demand.assign(
               task_data.cres_list.size(), 0.0 );

         boost::optional<const boost_pt::ptree&> cres_p = exec_el.second.get_child_optional("use");

         BOOST_FOREACH( const boost_pt::ptree::value_type& cres_el, cres_p.get()) {
            boost::optional<std::string> crnm_p = cres_el.second.get_optional<std::string>("res");
            boost::optional<double>      dmnd_p = cres_el.second.get_optional<double>("demand");

            if( !crnm_p.is_initialized() ||
                !dmnd_p.is_initialized()) {
               SCHD_REPORT_ERROR( "schd::plan" ) << name() <<  " Incorrect task structure " << name_p.get();
            }

            task_data.run_list.at( run_list_idx ).cres_demand.at(
                  std::distance(
                        task_data.cres_list.begin(),
                        std::find( task_data.cres_list.begin(),
                                   task_data.cres_list.end(),
                                   crnm_p.get() ))) = dmnd_p.get();

         } // BOOST_FOREACH( const boost_pt::ptree::value_type& cres_el, cres_p.get()) {

         run_list_idx ++;
      } // BOOST_FOREACH( const boost_pt::ptree::value_type& exec_el, exec_p.get()) {

      task_list.emplace( std::make_pair( name_p.get(), task_data ));
   } // BOOST_FOREACH( const boost_pt::ptree::value_type& task_el, _task_p.get())

   // Thread list
   std::list<std::string> evt_check_list = {"__start__"};

   BOOST_FOREACH( const boost_pt::ptree::value_type& thrd_el, _thrd_p.get()) {
      if( !thrd_el.first.empty()) {
         SCHD_REPORT_ERROR( "schd::plan" ) << name() <<  " Incorrect task structure";
      }

      boost::optional<std::string>            name_p = thrd_el.second.get_optional<std::string>("name");
      boost::optional<double>                 prio_p = thrd_el.second.get_optional<double>("priority");
      boost::optional<const boost_pt::ptree&> seqn_p = thrd_el.second.get_child_optional("sequence");
      boost::optional<const boost_pt::ptree&> evnt_p = thrd_el.second.get_child_optional("start");

      if( !name_p.is_initialized() ||
          !prio_p.is_initialized() ||
          !seqn_p.is_initialized() ||
          !evnt_p.is_initialized()) {
         SCHD_REPORT_ERROR( "schd::plan" ) << name() <<  " Incorrect thread structure";
      }

      thrd_data_t thrd_data;

      thrd_data.priority = prio_p.get();

      // Create list of the ignition events
      BOOST_FOREACH( const boost_pt::ptree::value_type& evnt_el, evnt_p.get()) {
         if( !evnt_el.first.empty()) {
            SCHD_REPORT_ERROR( "schd::plan" ) << name() <<  " Incorrect task structure " << name_p.get();
         }

         boost::optional<std::string> mask_p = evnt_el.second.get_value_optional<std::string>();

         if( !mask_p.is_initialized() ) {
            SCHD_REPORT_ERROR( "schd::plan" ) << name() <<  " Incorrect task structure " << name_p.get();
         }

         boost::regex evt_mask;

         try {
            evt_mask = mask_p.get();
         }
         catch( const boost::regex_error& err ) {
            SCHD_REPORT_ERROR( "schd::plan" ) << err.what();
         }
         catch( ... ) {
            SCHD_REPORT_ERROR( "schd::plan" ) << "Unexpected";
         }

         thrd_data.mask_evnt_list.push_back( evt_mask );
      } // BOOST_FOREACH( const boost_pt::ptree::value_type& evnt_el, evnt_p.get())

      BOOST_FOREACH( const boost_pt::ptree::value_type& seqn_el, seqn_p.get()) {
         if( !seqn_el.first.empty()) {
            SCHD_REPORT_ERROR( "schd::plan" ) << name() <<  " Incorrect task structure " << name_p.get();
         }

         boost::optional<std::string>            evnt_p = seqn_el.second.get_optional<std::string>("event");
         boost::optional<const boost_pt::ptree&> task_p = seqn_el.second.get_child_optional("task");

         thrd_seq_el_t thrd_seq_el;

         if( evnt_p.is_initialized() &&
             task_p.is_initialized()) {
            SCHD_REPORT_ERROR( "schd::plan" ) << name() <<  " Incorrect thread structure " << name_p.get();
         }
         else if( evnt_p.is_initialized()) {
            if( thrd_data.seq_list.size() == 0 ||
               !thrd_data.seq_list.at( thrd_data.seq_list.size() - 1 ).task_p.is_initialized()) {
               // Either the first one or 2 events in a row
               SCHD_REPORT_ERROR( "schd::plan" ) << name() <<  " Incorrect event sequence " << name_p.get();
            }

            thrd_seq_el.name = evnt_p.get();

            // save the event name for later static analysis
            evt_check_list.push_back( thrd_seq_el.name );
         }
         else if( task_p.is_initialized()) {
            boost::optional<std::string>            tsnm_p = task_p.get().get_optional<std::string>("run");
            boost::optional<const boost_pt::ptree&> tspr_p = task_p.get().get_child_optional("param");

            if( !tsnm_p.is_initialized() ||
                !tspr_p.is_initialized()) {
               SCHD_REPORT_ERROR( "schd::plan" ) << name() << " Incorrect thread structure " << name_p.get();
            }

            thrd_seq_el.task_param = tspr_p.get();
            thrd_seq_el.name       = tsnm_p.get();
            thrd_seq_el.task_p     =  boost::optional<const task_list_t::value_type&>(
                  *task_list.find( tsnm_p.get()));

            if( thrd_seq_el.task_p.get_ptr() == &( *task_list.end())) {
               SCHD_REPORT_ERROR( "schd::plan" ) << name() << " Unresolved task name in " << name_p.get();
            }
         }
         else {
            SCHD_REPORT_ERROR( "schd::plan" ) << name() << " Incorrect thread structure " << name_p.get();
         }

         thrd_data.seq_list.push_back( thrd_seq_el );
      } // BOOST_FOREACH( const boost_pt::ptree::value_type& seqn_el, seqn_p.get())

      thrd_list.emplace( std::make_pair( name_p.get(), thrd_data ));
   } // BOOST_FOREACH( const boost_pt::ptree::value_type& thrd_el, _thrd_p.get())

   // Check if there is an event potentially available for the ignition of each thread
   BOOST_FOREACH( const thrd_list_t::value_type& thrd_el, thrd_list ) {
      BOOST_FOREACH( const boost::regex& mask_el, thrd_el.second.mask_evnt_list ) {

         if( evt_check_list.end() == std::find_if(
               evt_check_list.begin(),
               evt_check_list.end(),
               [mask_el]( const std::string& evt_el )->bool {
                     return boost::regex_match( evt_el, mask_el ); } )) {
            SCHD_REPORT_ERROR( "schd::plan" )
                  << name()
                  <<  " igniting event is not found for the thread "
                  << thrd_el.first
                  << " mask "
                  << mask_el;
         }
      } // BOOST_FOREACH( const thrd_run_el_t& run_el, thrd_el.second.run_list )
   } // BOOST_FOREACH( const thrd_list_t::value_type& thrd_el, thrd_list )

} // schd_planner_c::init(

void schd_planner_c::exec_thrd( void ) {
   sc_core::wait(sc_core::SC_ZERO_TIME);

   schd_dump_buf_c<boost_pt::ptree> dump_buf_core_i( std::string( name()) + ".core_i" );
   schd_dump_buf_c<boost_pt::ptree> dump_buf_core_o( std::string( name()) + ".core_o" );

   // Register start event at #0
   std::string  start_evnt( "__start__" );
   event_data_t event_data;

   event_data.name_p     = boost::optional<const std::string&>( start_evnt );
   event_data.time       = sc_core::sc_time_stamp();
   event_reg.push_back( event_data );

   for(;;) {
      wait_list_t wait_list;
      std::size_t mask_exec_size = 0;

      // Scan the list of threads to find if any of them can start
      BOOST_FOREACH( thrd_list_t::value_type& thrd_el, thrd_list ) {
         // Create a vector of event masks
         std::vector<boost::optional<const boost::regex&>> mask_evnt_v( thrd_el.second.mask_evnt_list.size() );

         for( std::size_t run_idx = 0; run_idx < thrd_el.second.mask_evnt_list.size(); run_idx ++ ) {
            mask_evnt_v.at( run_idx ) =
                  boost::optional<const boost::regex&>( thrd_el.second.mask_evnt_list.at( run_idx ));
         } // BOOST_FOREACH( const thrd_run_el_t& run_el, thrd_el.second.run_list )

         // Create a vector of pointers to the events in the register
         std::list<boost::optional<event_reg_t::value_type&>> evnt_data_v;
         boost::optional<const thrd_list_t::value_type&> thrd_el_ptr =
               boost::optional<const thrd_list_t::value_type&>( thrd_el );

         BOOST_FOREACH( event_data_t& evnt_el, event_reg ) {
            if(( evnt_el.thrd_caller.get_ptr() != thrd_el_ptr.get_ptr()) &&    // This thread didn't issue the event
               ( evnt_el.thrd_run_list.end() == std::find_if(     // This thread is not in the "run" list of the event
                     evnt_el.thrd_run_list.begin(),
                     evnt_el.thrd_run_list.end(),
                     [thrd_el_ptr]( const boost::optional<const thrd_list_t::value_type&> &el )->bool {
                        return el.get_ptr() == thrd_el_ptr.get_ptr(); } )) &&
               ( evnt_el.thrd_msm_list.end() == std::find_if(     // This thread is not in the "mismatch" list of the event
                     evnt_el.thrd_msm_list.begin(),
                     evnt_el.thrd_msm_list.end(),
                     [thrd_el_ptr]( const boost::optional<const thrd_list_t::value_type&> &el )->bool {
                        return el.get_ptr() == thrd_el_ptr.get_ptr(); } ))) {
               evnt_data_v.push_back(
                     boost::optional<event_reg_t::value_type&>( evnt_el ));
            }
         } // BOOST_FOREACH( const event_reg_t::value_type& evnt_el, event_reg )

         // Create vector of the pointers to the event names
         std::vector<boost::optional<const std::string&>> evnt_name_v( evnt_data_v.size());

         std::size_t evnt_idx = 0;
         BOOST_FOREACH( boost::optional<event_reg_t::value_type&> evnt_el, evnt_data_v ) {
            evnt_name_v.at( evnt_idx ) = evnt_el.get().name_p;
            evnt_idx ++;
         }

         std::vector<std::size_t> mask_evnt_map_v;
         std::vector<std::size_t> evnt_mask_cnt_v;

         if( and_list(
               mask_evnt_v,
               evnt_name_v,
               mask_evnt_map_v,
               evnt_mask_cnt_v )) {
            if( thrd_el.second.seq_state == SEQ_STATE_IDLE ) {
               // Thread is waiting for the task at index 0 to get started
               thrd_el.second.seq_idx   = 0;
               thrd_el.second.seq_state = SEQ_STATE_WAITING;

               // Update run list for the event and event list in the thread
               if( !thrd_el.second.evnt_list.empty() ) {
                  SCHD_REPORT_ERROR( "schd::plan" )
                                       << name()
                                       << " Non-empty evnt_list in the thread: "
                                       << thrd_el.first;
               }

               std::size_t evnt_idx = 0;

               BOOST_FOREACH( boost::optional<event_reg_t::value_type&> evnt_el, evnt_data_v ) {
                  if( evnt_mask_cnt_v.at( evnt_idx ) != 0 ) {
                     evnt_el.get().thrd_run_list.push_back(
                           boost::optional<const thrd_list_t::value_type&>( thrd_el ));

                     thrd_el.second.evnt_list.push_back(
                           boost::optional<event_reg_t::value_type&>( evnt_el ));
                  }

                  evnt_idx ++;
               }
            }
            else {
               SCHD_REPORT_ERROR( "schd::plan" )
                     << name()
                     << " Re-start condition for running thread "
                     << thrd_el.first;
            }
         } // if( and_list( ...
         else {
            // Update mismatch list for the event
            std::size_t evnt_idx = 0;

            BOOST_FOREACH( boost::optional<event_reg_t::value_type&> evnt_el, evnt_data_v ) {
               if( evnt_mask_cnt_v.at( evnt_idx ) == 0 ) {
                  evnt_el.get().thrd_msm_list.push_back(
                        boost::optional<const thrd_list_t::value_type&>( thrd_el ));
               }

               evnt_idx ++;
            }
         } // if( and_list( ... else ...

         // Update waiting list for the execution
         if( thrd_el.second.seq_state == SEQ_STATE_WAITING ) {
            wait_data_t wait_data;

            wait_data.priority = thrd_el.second.priority; // To simplify access when sorting
            wait_data.thrd_p   = boost::optional<      thrd_list_t::value_type&>( thrd_el );
            wait_data.task_p   = boost::optional<const task_list_t::value_type&>(
                  thrd_el.second.seq_list.at( thrd_el.second.seq_idx ).task_p );

            if( !wait_data.task_p.is_initialized()) {
               SCHD_REPORT_ERROR( "schd::plan" )
                     << name()
                     << " Incorrect sequence element "
                     << thrd_el.second.seq_list.at( thrd_el.second.seq_idx ).name
                     << " while processing thread "
                     << thrd_el.first;
            }

            wait_list.push_back( wait_data );

            mask_exec_size += wait_data.task_p.get().second.run_list.size();
         } // if( thrd_el.second.seq_state == SEQ_STATE_WAITING )
      } // BOOST_FOREACH( const thrd_list_t::value_type& thrd_el, thrd_list )

      // Sort wait list in the order of descending priority
      wait_list.sort(
            []( const wait_list_t::value_type &lhs, wait_list_t::value_type &rhs )->bool {
               return lhs.priority > rhs.priority; } );

      // Create vector of masks and update indexes in the wait list
      std::vector<boost::optional<const boost::regex&>> mask_exec_v( mask_exec_size );

      std::size_t mask_exec_idx = 0;

      BOOST_FOREACH( wait_list_t::value_type& wait_el, wait_list ) {
         wait_el.mask_exec_base = mask_exec_idx;
         wait_el.mask_exec_size = wait_el.task_p.get().second.run_list.size();

         for( std::size_t run_idx = 0; run_idx < wait_el.mask_exec_size; run_idx ++ ) {
            mask_exec_v.at( mask_exec_idx + run_idx ) = boost::optional<const boost::regex&>(
                  wait_el.task_p.get().second.run_list.at( run_idx ).mask_exec );
         }

         mask_exec_idx += wait_el.task_p.get().second.run_list.size();
      }

      // Find available execution blocks
      std::vector<boost::optional<exec_list_t::value_type&>> exec_data_v;

      BOOST_FOREACH( exec_list_t::value_type& exec_el, exec_list ) {
         if( !exec_el.second.task_p.is_initialized()) {
            exec_data_v.push_back(
                  boost::optional<exec_list_t::value_type&>( exec_el ));
         }
      }

      // Allocate execution blocks
      std::vector<boost::optional<const std::string&>> exec_name_v( exec_data_v.size());
      std::vector<std::size_t> mask_exec_map_v( mask_exec_v.size());
      std::vector<std::size_t> exec_mask_cnt_v( exec_data_v.size());

      // Vector of exec names
      for( std::size_t exec_idx = 0; exec_idx < exec_data_v.size(); exec_idx ++ ) {
         exec_name_v.at( exec_idx ) = boost::optional<const std::string&>(
               exec_data_v.at( exec_idx ).get().first );  //
      }

      while( wait_list.size() != 0 && mask_exec_v.size() != 0 ) {
         if( and_list(
               mask_exec_v,
               exec_name_v,
               mask_exec_map_v,
               exec_mask_cnt_v )) { // All requests are mapped to the available exec blocks. Exit.
            break;
         }
         else { // Remove data which corresponds to the lowest priority
            std::size_t low_prio_base = wait_list.back().mask_exec_base;

            wait_list.resize( wait_list.size() - 1 );

            if( wait_list.size() == 0 ) {
               break;
            }

            mask_exec_v.resize(     low_prio_base - 1 );
            mask_exec_map_v.resize( low_prio_base - 1 );
         }
      }

      BOOST_FOREACH( const wait_list_t::value_type& wait_el, wait_list ) {
         if( !wait_el.thrd_p.get().second.exec_list.empty() ) {
            SCHD_REPORT_ERROR( "schd::plan" )
                                 << name()
                                 << " Non-empty exec_list in the thread: "
                                 << wait_el.thrd_p.get().first;
         }

         boost_pt::ptree  plan_pt;
         schd_sig_ptree_c pt_out;

         plan_pt.put("src",     name()                               );       // Planner
         plan_pt.put("thread",  wait_el.thrd_p.get().first           );       // Thread name
         plan_pt.put("task",    wait_el.task_p.get().first           );       // Task   name
         plan_pt.put("runtime", wait_el.task_p.get().second.run_time );       // Runtime
         plan_pt.put_child("param",   wait_el.thrd_p.get().second.seq_list.at(
               wait_el.thrd_p.get().second.seq_idx ).task_param      );       // Parameters

         for( std::size_t exec_idx = 0; exec_idx < wait_el.mask_exec_size; exec_idx ++ ) {
            exec_list_t::value_type& exec_data_r =
                  exec_data_v.at( mask_exec_map_v.at( wait_el.mask_exec_base + exec_idx )).get();

            exec_data_r.second.time_start = sc_core::sc_time_stamp();
            exec_data_r.second.task_p     = wait_el.task_p;
            exec_data_r.second.thrd_p     = wait_el.thrd_p;
            exec_data_r.second.param_p    = boost::optional<const boost_pt::ptree&>(
                  wait_el.thrd_p.get().second.seq_list.at( wait_el.thrd_p.get().second.seq_idx ).task_param );

            wait_el.thrd_p.get().second.exec_list.push_back(
                  boost::optional<const exec_list_t::value_type&>( exec_data_r ));

            // Construct data to be sent to the exec blocks
            boost_pt::ptree  cres_list_pt;
            boost_pt::ptree  exec_list_pt;

            exec_list_pt.push_back( std::make_pair( "", boost_pt::ptree().put( "", exec_data_r.first ) ));

            for( std::size_t cres_idx = 0; cres_idx < exec_data_r.second.task_p.get().second.cres_list.size(); cres_idx ++ ) {
               boost_pt::ptree  cres_pt;

               cres_pt.put( "res",
                     exec_data_r.second.task_p.get().second.cres_list.at( cres_idx ));
               cres_pt.put( "demand",
                     exec_data_r.second.task_p.get().second.run_list.at( exec_idx ).cres_demand.at( cres_idx ));

               cres_list_pt.push_back( std::make_pair( "", cres_pt ));
            }

            plan_pt.put_child("dst",     exec_list_pt ); // Name of the exec block to update
            plan_pt.put_child("common",  cres_list_pt ); // Common resources
            plan_pt.put_child("options",                 // exec options
                  exec_data_r.second.task_p.get().second.run_list.at( exec_idx ).options );

            core_o->write( pt_out.set( plan_pt )); // Write data to the output

            // Dump pt packets as they depart from the output of the block
            dump_buf_core_o.write( plan_pt, BUF_WRITE_LAST );

         } // for( std::size_t exec_idx = 0; exec_idx < wait_el.mask_exec_size; exec_idx ++ )
         wait_el.thrd_p.get().second.seq_state  = SEQ_STATE_RUNNING;
      } // BOOST_FOREACH( wait_list_t::value_type& wait_el, wait_list )

      wait_list.clear();

      sc_core::wait( core_i->data_written_event());

      while( core_i->num_available()) {
         // Read from port
         boost_pt::ptree        core_pt_raw = core_i->read().get();
         const boost_pt::ptree& core_pt_inp = core_pt_raw;

         // Dump pt packets as they arrive to the input of the block
         dump_buf_core_i.write( core_pt_inp, BUF_WRITE_LAST );

         // Update exec statuses and thread sequences
         boost::optional<std::string>            src_p = core_pt_inp.get_optional<std::string>("src");        // Tread name
         boost::optional<const boost_pt::ptree&> dst_p = core_pt_inp.get_child_optional("dst");          // task name

         if( !src_p.is_initialized() ||
             !dst_p.is_initialized()) {
            std::string core_pt_str;

            SCHD_REPORT_ERROR( "schd::plan" ) << name()
                                              << " Incorrect data format from exec: "
                                              << pt2str( core_pt_inp, core_pt_str );
         }

         // Resolve the execution block name
         boost::optional<exec_list_t::value_type&> exec_p =
               boost::optional<exec_list_t::value_type&>( *exec_list.find( src_p.get()) );

         if( exec_p.get_ptr() ==
               boost::optional<exec_list_t::value_type&>( *exec_list.end()).get_ptr()) {
            SCHD_REPORT_ERROR( "schd::plan" ) << name()
                                              << " Can't resolve exec name: "
                                              << exec_p.get().first;
         }

         // Casting to non-const to modify
         boost::optional<thrd_list_t::value_type&> thrd_p =
               boost::optional<thrd_list_t::value_type&>(
                     const_cast<thrd_list_t::value_type&>( exec_p.get().second.thrd_p.get()) );

         // Free exec
         exec_p.get().second.time_end = sc_core::sc_time_stamp();
         exec_p.get().second.thrd_p.reset();
         exec_p.get().second.task_p.reset();
         exec_p.get().second.param_p.reset();

         // Update thread information
         auto exec_list_it = std::find_if(
               thrd_p.get().second.exec_list.begin(),
               thrd_p.get().second.exec_list.end(),
               [exec_p]( const boost::optional<const exec_list_t::value_type&> &el )->bool {
                  return el.get_ptr() == exec_p.get_ptr(); } );

         if( exec_list_it == thrd_p.get().second.exec_list.end()) {
            SCHD_REPORT_ERROR( "schd::plan" ) << name()
                                              << " Can't find exec name: "
                                              << exec_p.get().first
                                              << " in the exec_list of the thread "
                                              << thrd_p.get().first;
         }

         thrd_p.get().second.exec_list.erase( exec_list_it );

         while( thrd_p.get().second.exec_list.empty()) {
            thrd_p.get().second.seq_idx ++;

            if( thrd_p.get().second.seq_idx >= thrd_p.get().second.seq_list.size()) {
               // End of thread
               BOOST_FOREACH( boost::optional<event_reg_t::value_type&> evnt_el,
                     thrd_p.get().second.evnt_list ) {
                  evnt_el.get().thrd_end_cntr ++;
               }

               thrd_p.get().second.seq_state = SEQ_STATE_IDLE;
               thrd_p.get().second.evnt_list.clear();
               break;
            }
            else if( thrd_p.get().second.seq_list.at(
                  thrd_p.get().second.seq_idx ).task_p.is_initialized() ) {
               // Proceed with the new task from the sequence
               thrd_p.get().second.seq_state = SEQ_STATE_WAITING;
               break;
            }
            else {
               // Register new event
               event_data.name_p = boost::optional<const std::string&>(
                     thrd_p.get().second.seq_list.at( thrd_p.get().second.seq_idx ).name );
               event_data.time   = sc_core::sc_time_stamp();
               event_reg.push_back( event_data );
            }
         } // while( thrd_p.get().second.exec_list.empty())

         // Cleanup event register
         if( thrd_p.get().second.seq_state == SEQ_STATE_IDLE ) {
            BOOST_FOREACH( event_reg_t::value_type& evnt_el, event_reg ) {
               if( evnt_el.thrd_end_cntr == evnt_el.thrd_run_list.size() &&
                   evnt_el.thrd_end_cntr + evnt_el.thrd_run_list.size() + 1 >=
                   thrd_list.size()) {
                  evnt_el.remove = true;
               }
            }

            event_reg.remove_if( []( event_reg_t::value_type& el )->bool {
               return el.remove; } );
         } // if( thrd_p.get().second.seq_state == SEQ_STATE_IDLE )

      } // while( core_i->num_available())
   } // for(;;)
} // schd_planner_c::exec_thrd(

bool schd_planner_c::and_list(
      std::vector<boost::optional<const boost::regex&>>& msk, // regex masks
      std::vector<boost::optional<const std::string&>>&  val, // check values
      std::vector<std::size_t>& msk_val_map,                  // matching value for each mask
      std::vector<std::size_t>& val_msk_cnt ) {               // Number of masks matching each value

   // Create and fill the matrix of matches
   std::vector<std::vector<std::size_t>> mat( msk.size(), std::vector<std::size_t>( val.size(), 0 ));
   std::vector<std::size_t> mv_cnt( msk.size(), 0 );
   std::vector<std::size_t> vm_cnt( val.size(), std::numeric_limits<std::size_t>::max() );
   val_msk_cnt.assign( val.size(), 0 );
   msk_val_map.assign( msk.size(), std::numeric_limits<std::size_t>::max() );

   for( std::size_t msk_idx = 0; msk_idx < msk.size(); msk_idx ++ ) {
      for( std::size_t val_idx = 0; val_idx < val.size(); val_idx ++ ) {
         if( boost::regex_match( val.at( val_idx ).get(), msk.at(msk_idx ).get() )) {
            mat.at( msk_idx ).at( val_idx ) = 1;
            mv_cnt.at(  msk_idx ) ++;
            vm_cnt.at(  val_idx ) =
                  ( vm_cnt.at( val_idx ) > msk.size() ) ? 1
                                                        : ( vm_cnt.at( val_idx ) + 1 );

            // We need to fill this vector so we run till the end
            val_msk_cnt.at( val_idx ) ++;
         }
      }
   }

   // exit if we have at least 1 mask without a matching value
   if( std::find( mv_cnt.begin(), mv_cnt.end(), 0 ) != mv_cnt.end() ) {
      return false;
   }

   while( true ) {
      // Find mask with min number of matching values
      std::size_t min_mv_cnt_idx = std::distance( mv_cnt.begin(),
                                                  std::min_element( mv_cnt.begin(), mv_cnt.end()));

      if( mv_cnt.at( min_mv_cnt_idx ) <= val.size() ) {
         // Find value with min number of matching masks
         std::size_t min_vm_cnt_idx = std::numeric_limits<std::size_t>::max(),
                     min_vm_cnt_val = std::numeric_limits<std::size_t>::max();

         for( std::size_t val_idx = 0; val_idx < val.size(); val_idx ++ ) {
            if( mat.at( min_mv_cnt_idx ).at( val_idx ) &&
                min_vm_cnt_val > vm_cnt.at( val_idx )) {
                min_vm_cnt_val = vm_cnt.at( val_idx );
                min_vm_cnt_idx = val_idx;
            }
         }

         // Update map metrics
         if( min_vm_cnt_idx == std::numeric_limits<std::size_t>::max()) {
        	 SCHD_REPORT_ERROR( "schd::plan" ) << name() << " Incorrect index.";
         }

         msk_val_map.at( min_mv_cnt_idx ) = min_vm_cnt_idx;

         mv_cnt.at( min_mv_cnt_idx ) = std::numeric_limits<std::size_t>::max();
         vm_cnt.at( min_vm_cnt_idx ) = std::numeric_limits<std::size_t>::max();

         for( std::size_t msk_idx = 0; msk_idx < msk.size(); msk_idx ++ ) {
            mv_cnt.at( msk_idx ) -= mat.at( msk_idx ).at( min_vm_cnt_idx );

            if( mv_cnt.at( msk_idx ) == 0 ) {
                mv_cnt.at( msk_idx ) = std::numeric_limits<std::size_t>::max();
            }
         }

         for( std::size_t val_idx = 0; val_idx < val.size(); val_idx ++ ) {
            vm_cnt.at( val_idx ) -= mat.at( min_mv_cnt_idx ).at( val_idx );

            if( vm_cnt.at( val_idx ) == 0 ) {
                vm_cnt.at( val_idx ) = std::numeric_limits<std::size_t>::max();
            }
         }
      }
      else {
         break;
      }
   }

   return msk_val_map.end() == std::find(
         msk_val_map.begin(),
         msk_val_map.end(),
         std::numeric_limits<std::size_t>::max() );

} // and_list(

} // namespace schd
