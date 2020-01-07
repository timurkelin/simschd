/*
 * schd_exec.cpp
 *
 *  Description:
 *    Methods of the Execution unit
 */

#include <algorithm>
#include <iterator>
#include <boost/foreach.hpp>
#include "schd_exec.h"
#include "schd_ptree_time.h"
#include "schd_conv_ptree.h"
#include "schd_dump.h"
#include "schd_trace.h"
#include "schd_assert.h"
#include "schd_report.h"

namespace schd {

SC_HAS_PROCESS( schd::schd_exec_c );
schd_exec_c::schd_exec_c(
      sc_core::sc_module_name nm )
   : sc_core::sc_module( nm )
   , plan_i( "plan_i" )
   , plan_o( "plan_o" )
   , cres_i( "cres_i" )
   , cres_o( "cres_o" )
   , exec_complete( "exec_complete" ) {

   // Process registrations
   SC_THREAD( exec_thrd );
}

void schd_exec_c::init(
      boost::optional<const boost_pt::ptree&> _exec_p,
      boost::optional<const boost_pt::ptree&> _cres_p ) {

   // Create cres instances and the corresponding channels
   BOOST_FOREACH( const boost_pt::ptree::value_type& cres_el, _cres_p.get()) {
      if( !cres_el.first.empty()) {
         SCHD_REPORT_ERROR( "schd::exec" ) << name() <<  " Incorrect cres structure";
      }

      boost::optional<std::string> cres_name_p = cres_el.second.get_optional<std::string>("name");
      boost::optional<double>      cres_cap_p  = cres_el.second.get_optional<double>("capacity");

      if( !cres_name_p.is_initialized() ||
          !cres_cap_p.is_initialized()) {
         SCHD_REPORT_ERROR( "schd::exec" ) << name() <<  " Incorrect cres structure";
      }

      // Initialise cres data
      schd_exec_c::cres_data_t cres_data;
      cres_data.capacity     = cres_cap_p.get();

      if( cres_data.capacity <= 0.0 ) {
         SCHD_REPORT_ERROR( "schd::exec" ) << name() << " Incorrect capacity specification";
      }

      cres_list.emplace( std::make_pair( cres_name_p.get(), cres_data ));
   } // BOOST_FOREACH( const boost_pt::ptree::value_type& cres_el, _cres_p.get())

} // schd_exec_c::init(

void schd_exec_c::add_trace(
      sc_core::sc_trace_file* tf,
      const std::string& top_name ) {

   std::string mod_name = top_name + "." + name() + ".";

   // Add trace for cres
   BOOST_FOREACH( cres_list_t::value_type& cres_el, cres_list ) {
      sc_core::sc_trace(
            tf,
            &( cres_el.second.capacity ),
            mod_name + cres_el.first + ".capacity" );

      sc_core::sc_trace(
            tf,
            &( cres_el.second.connected ),
            mod_name + cres_el.first + ".connected" );

      sc_core::sc_trace(
            tf,
            &( cres_el.second.cres_demand ),
            mod_name + cres_el.first + ".cres_demand" );

      sc_core::sc_trace(
            tf,
            &( cres_el.second.cres_load ),
            mod_name + cres_el.first + ".cres_load" );

      sc_core::sc_trace(
            tf,
            &( cres_el.second.plan_demand ),
            mod_name + cres_el.first + ".plan_demand" );

      sc_core::sc_trace(
            tf,
            &( cres_el.second.exec_demand ),
            mod_name + cres_el.first + ".exec_demand" );
   }

   // Add trace for job hash
   sc_core::sc_trace(
         tf,
         &( job_hash ),
         mod_name + "job_hash" );

} // schd_exec_c::add_trace(

void schd_exec_c::exec_thrd( void ) {
   schd_dump_buf_c<boost_pt::ptree> dump_buf_plan_i( std::string( name()) + ".plan_i" );
   schd_dump_buf_c<boost_pt::ptree> dump_buf_plan_o( std::string( name()) + ".plan_o" );
   schd_dump_buf_c<boost_pt::ptree> dump_buf_cres_i( std::string( name()) + ".cres_i" );
   schd_dump_buf_c<boost_pt::ptree> dump_buf_cres_o( std::string( name()) + ".cres_o" );

   sc_core::wait(sc_core::SC_ZERO_TIME);

   for(;;) {
      sc_core::sc_event_or_list ports_run_or_list;

      ports_run_or_list |= plan_i->data_written_event();
      ports_run_or_list |= cres_i->data_written_event();

      if( time_to_go != sc_core::SC_ZERO_TIME ) {
         ports_run_or_list |= exec_complete;
      }

      sc_core::wait( ports_run_or_list ); // Wait for any event from the list to trigger

      // Check if there is new data from the planner
      while( plan_i->num_available()) {
         boost_pt::ptree        plan_pt_raw = plan_i->read().get();
         const boost_pt::ptree& plan_pt_inp = plan_pt_raw;

         // Extract data from the received signal
         boost::optional<std::string>            thrd_p = plan_pt_inp.get_optional<std::string>("thread");        // Tread name
         boost::optional<std::string>            task_p = plan_pt_inp.get_optional<std::string>("task");          // task name
         boost::optional<const boost_pt::ptree&> para_p = plan_pt_inp.get_child_optional("param");                // parameters ptree

         boost::optional<sc_core::sc_time>       time_p = plan_pt_inp.get_optional<sc_core::sc_time>("runtime");  // execution time
         boost::optional<const boost_pt::ptree&> cres_p = plan_pt_inp.get_child_optional("common");               // Common resources

         if( !thrd_p.is_initialized() ||
             !task_p.is_initialized() ||
             !para_p.is_initialized() ||
             !time_p.is_initialized() ||
             !cres_p.is_initialized()) {
            std::string plan_pt_str;

            SCHD_REPORT_ERROR( "schd::exec" ) << name()
                                              << " Incorrect data format from planner: "
                                              << pt2str( plan_pt_inp, plan_pt_str );
         }

         boost::optional<std::string> prid_p = para_p.get().get_optional<std::string>("id");

         if( !prid_p.is_initialized() ) {
            std::string plan_pt_str;

            SCHD_REPORT_ERROR( "schd::exec" ) << name()
                                              << " Incorrect data format from planner: "
                                              << pt2str( plan_pt_inp, plan_pt_str );
         }

         // Dump pt packets as they arrive to the input of the block
         dump_buf_plan_i.write( plan_pt_inp, BUF_WRITE_LAST );

         // Save task name
         if( job_hash == 0 ) {
            thrd_name = thrd_p.get();
            task_name = task_p.get();
             param_id = prid_p.get();
         }
         else {
            SCHD_REPORT_ERROR( "simd::exec" ) << " Unexpected request from the planner: "
                                              << schd_trace.job_comb(
                                                    thrd_p.get(),
                                                    task_p.get(),
                                                    prid_p.get())
                                              << " while running: "
                                              << schd_trace.job_comb(
                                                    thrd_name,
                                                    task_name,
                                                    param_id );
         }

         // Update the list of the common resources and notify common resources
         BOOST_FOREACH( const boost_pt::ptree::value_type& cres_el, cres_p.get()) {
            if( !cres_el.first.empty()) {
               SCHD_REPORT_ERROR( "schd::exec" ) << name() <<  " Incorrect structure";
            }

            boost::optional<std::string> name_p = cres_el.second.get_optional<std::string>("res");
            boost::optional<double>      dmnd_p = cres_el.second.get_optional<double>("demand");

            // Check availability of the fields
            if( !name_p.is_initialized() ||
                !dmnd_p.is_initialized() ||
                 dmnd_p.get() < 0 ) {
               SCHD_REPORT_ERROR( "schd::exec" ) << name() << " Incorrect data format from planner";
            } // if( !name_p.is_initialized() || ...

            auto cres_list_it = cres_list.find( name_p.get());

            if( cres_list_it == cres_list.end()) {
               SCHD_REPORT_ERROR( "schd::exec" )
                     << name()
                     << " Unexpected resorce name: "
                     << name_p.get();
            }
            else if( cres_list_it->second.state != CRES_STATE_IDLE ) {
               SCHD_REPORT_ERROR( "schd::exec" )
                     << name()
                     << " Duplicate connection for: "
                     << name_p.get();
            }

            // Initialise cres data
            cres_list_it->second.state       = CRES_STATE_WAIT_CON;
            cres_list_it->second.cres_demand = 0.0;
            cres_list_it->second.cres_load   = 0.0;
            cres_list_it->second.plan_demand = dmnd_p.get();
            cres_list_it->second.exec_demand = dmnd_p.get();

            // Notify common resource on the new demand
            boost_pt::ptree  dst_list_pt;
            boost_pt::ptree  exec_pt;
            schd_sig_ptree_c pt_out;

            dst_list_pt.push_back( std::make_pair( "", boost_pt::ptree().put( "", name_p.get() ) ));

            // Notify common resources from the list on the new demand
            exec_pt.put(      "src",       name()       ); // Name of this resource
            exec_pt.put_child("dst",       dst_list_pt  ); // Common resource to update
            exec_pt.put(      "connected", true         ); // Connected
            exec_pt.put(      "demand",    dmnd_p.get() ); // Resource demand from this execution block

            cres_o->write( pt_out.set( exec_pt )); // Write data to the output

            // Dump pt packets as they depart from the output of the block
            dump_buf_cres_o.write( exec_pt, BUF_WRITE_LAST );
         } // BOOST_FOREACH( const boost_pt::ptree::value_type& cres_el, cres_p.get())

         // Update time and run the execution
         time_to_go = time_p.get();
         time_upd   = sc_core::sc_time_stamp();
         time_ext_coe = 1.0;

         std::string job_tag = schd_trace.job_comb(
               thrd_p.get(),
               task_p.get(),
               prid_p.get());

         job_hash = 0;
         boost::hash_combine(
               job_hash,
               job_tag  );

         exec_complete.notify( time_to_go );

      } // while( plan_i->num_available())

      // Check if there is new data from common resources
      while( cres_i->num_available()) {
         boost_pt::ptree        cres_pt_raw = cres_i->read().get();
         const boost_pt::ptree& cres_pt_inp = cres_pt_raw;

         // Extract data from the received signal
         boost::optional<std::string> cres_p = cres_pt_inp.get_optional<std::string>("src");    // cres name
         boost::optional<double>      dmnd_p = cres_pt_inp.get_optional<double>("demand");      // demand
         boost::optional<bool>        stat_p = cres_pt_inp.get_optional<bool>("connected");      // connection state

         if( !cres_p.is_initialized() ||
             !dmnd_p.is_initialized() ||
             !stat_p.is_initialized()) {
            std::string cres_pt_str;

            SCHD_REPORT_ERROR( "schd::exec" ) << name()
                                              << " Incorrect data format from cres: "
                                              << pt2str( cres_pt_inp, cres_pt_str );
         }

         // Dump pt packets as they arrive to the input of the block
         dump_buf_cres_i.write( cres_pt_inp, BUF_WRITE_LAST );

         auto cres_list_it = cres_list.find( cres_p.get());

         if( cres_list_it == cres_list.end()) {
            SCHD_REPORT_ERROR( "schd::exec" )
                  << name()
                  << " Unexpected resource name: "
                  << cres_p.get();
         }
         else if( cres_list_it->second.state == CRES_STATE_IDLE ) {
            SCHD_REPORT_ERROR( "schd::exec" )
                  << name()
                  << " Unexpected update from: "
                  << cres_p.get();
         }

         // Notify common resource on the new demand
         boost_pt::ptree  dst_list_pt;
         boost_pt::ptree  exec_pt;
         schd_sig_ptree_c pt_out;

         if( stat_p.get() && cres_list_it->second.state != CRES_STATE_IDLE &&
                             cres_list_it->second.state != CRES_STATE_WAIT_DCN ) {
            cres_list_it->second.state       = CRES_STATE_CONNECTED;
            cres_list_it->second.connected   = true;
            cres_list_it->second.cres_demand = dmnd_p.get();
            cres_list_it->second.cres_load   = dmnd_p.get() / cres_list_it->second.capacity;

            double cres_load_max = 0;

            // Find common resource which is loaded at most
            BOOST_FOREACH( const cres_list_t::value_type& cres_el, cres_list ) {
               if( cres_el.second.state == CRES_STATE_CONNECTED ) {
                  cres_load_max = std::max( cres_load_max, cres_el.second.cres_load );
               }
            }

            // Update demand for the common resource if there are changes
            BOOST_FOREACH( cres_list_t::value_type& cres_el, cres_list ) {
               if( cres_el.second.state != CRES_STATE_CONNECTED ) {
                  continue;
               }

               double exec_demand_new;

               if(( cres_load_max <= 1.0 ) ||
                  ( cres_load_max == cres_el.second.cres_load )) {
                  // Don't change the demand if the resource is not demanded above the capacity
                  // Don't change the demand for the resource with the max load
                  exec_demand_new = cres_el.second.plan_demand;
               }
               else {
                  // Reduce the demand for the resources other than one with max load.
                  // This models the demand while execution block is stalled by the shortage of supply
                  exec_demand_new = cres_el.second.plan_demand / cres_load_max;
               }

               // Check if the reported demand has changed
               if( cres_el.second.exec_demand != exec_demand_new ) {
                  cres_el.second.exec_demand = exec_demand_new;

                  // Notify common resource from the list on the new demand
                  dst_list_pt.clear();

                  dst_list_pt.push_back( std::make_pair( "", boost_pt::ptree().put( "", cres_el.first ) ));

                  exec_pt.put(      "src",       name()          ); // Name of this resource
                  exec_pt.put_child("dst",       dst_list_pt     ); // Common resource to update
                  exec_pt.put(      "connected", true            ); // Connected
                  exec_pt.put(      "demand",    exec_demand_new ); // Demand

                  cres_o->write( pt_out.set( exec_pt )); // Write data to the output

                  // Dump pt packets as they depart from the output of the block
                  dump_buf_cres_o.write( exec_pt, BUF_WRITE_LAST );
               } // if( cres_el.second.exec_demand != exec_demand_new )  ...
            } // BOOST_FOREACH( const cres_list_t::value_type& cres_el, cres_list )

            // Update time to go
            double time_ext_coe_new = std::max( cres_load_max, 1.0 );

            if( time_ext_coe != time_ext_coe_new ) {
               time_to_go = ( time_to_go - ( sc_core::sc_time_stamp() - time_upd )) / time_ext_coe * time_ext_coe_new;
               time_upd   = sc_core::sc_time_stamp();

               time_ext_coe = time_ext_coe_new;

               exec_complete.cancel();
               exec_complete.notify( time_to_go );
            } // if( time_ext_coe != time_ext_coe_new )
         } // if( stat_p.get() )
         else if ( !stat_p.get() && cres_list_it->second.state == CRES_STATE_WAIT_DCN ) {
            cres_list_it->second.state     = CRES_STATE_IDLE;
            cres_list_it->second.connected = false;

            if( cres_list.end() == std::find_if(
                  cres_list.begin(),
                  cres_list.end(),
                  []( const cres_list_t::value_type& el )->bool {
                     return el.second.state != CRES_STATE_IDLE; } )) {
               // Notify planner that the execution is complete
               dst_list_pt.push_back( std::make_pair( "", boost_pt::ptree().put( "", "planner" ) ));

               exec_pt.put(       "src",  name()      );  // Name of this executor
               exec_pt.put_child( "dst",  dst_list_pt );  // Destination: planner

               plan_o->write( pt_out.set( exec_pt )); // Write data to the output

               // Dump pt packets as they depart from the output of the block
               dump_buf_plan_o.write( exec_pt, BUF_WRITE_LAST );
            }
         } // if( stat_p.get() ) ... else ...
         else if( stat_p.get() && cres_list_it->second.state == CRES_STATE_WAIT_DCN ) {
            ;
         }
         else {
            SCHD_REPORT_ERROR( "schd::exec" ) << name() << " Unexpected state transition";
         }
      } // while( cres_i->num_available())

      // Check if the execution is complete
      if( exec_complete.triggered()) {
         time_to_go   = sc_core::SC_ZERO_TIME;
         time_ext_coe = 1.0;

         boost_pt::ptree  dst_list_pt;
         boost_pt::ptree  exec_pt;
         schd_sig_ptree_c pt_out;

         BOOST_FOREACH( cres_list_t::value_type& cres_el, cres_list ) {
            if( cres_el.second.state != CRES_STATE_IDLE ) {
               dst_list_pt.push_back( std::make_pair( "", boost_pt::ptree().put( "", cres_el.first ) ));

               cres_el.second.state       = CRES_STATE_WAIT_DCN;
               cres_el.second.cres_demand = 0.0;
               cres_el.second.cres_load   = 0.0;
               cres_el.second.plan_demand = 0.0;
               cres_el.second.exec_demand = 0.0;
            }
         }

         // Notify common resources from the list that the executor now has zero demand
         if( !dst_list_pt.empty()) {
            exec_pt.put(      "src",       name()      ); // Name of this resource
            exec_pt.put_child("dst",       dst_list_pt ); // Update all common resources involved
            exec_pt.put(      "connected", false       ); // No longer connected
            exec_pt.put(      "demand",    0.0         ); // Zero demand

            cres_o->write( pt_out.set( exec_pt )); // Write data to the output

            // Dump pt packets as they depart from the output of the block
            dump_buf_cres_o.write( exec_pt, BUF_WRITE_LAST );
         }
         else {
            // Notify planner that the execution is complete
            dst_list_pt.push_back( std::make_pair( "", boost_pt::ptree().put( "", "planner" ) ));

            exec_pt.put(       "src",  name()      );  // Name of this executor
            exec_pt.put_child( "dst",  dst_list_pt );  // Destination: planner

            plan_o->write( pt_out.set( exec_pt )); // Write data to the output

            // Dump pt packets as they depart from the output of the block
            dump_buf_plan_o.write( exec_pt, BUF_WRITE_LAST );
         }

         job_hash = 0;
      } // if( exec_complete.triggered())
   } // for(;;)
}

} // namespace schd
