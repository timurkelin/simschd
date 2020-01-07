/*
 * schd_cres.cpp
 *
 *  Description:
 *    Common resource module
 */

#include <algorithm>
#include <iterator>
#include <boost/foreach.hpp>
#include "schd_cres.h"
#include "schd_dump.h"
#include "schd_assert.h"
#include "schd_report.h"

namespace schd {

SC_HAS_PROCESS( schd::schd_cres_c );
schd_cres_c::schd_cres_c(
      sc_core::sc_module_name nm )
   : sc_core::sc_module( nm )
   , exec_i( "exec_i" )
   , exec_o( "exec_o" ) {

   // Process registrations
   SC_THREAD( exec_thrd );
}

void schd_cres_c::init(
      boost::optional<const boost_pt::ptree&> _pref_p,
      boost::optional<const boost_pt::ptree&> _exec_p ) {

   try {
      capacity = _pref_p.get().get<double>("capacity");
   }
   catch( const boost_pt::ptree_error& err ) {
      SCHD_REPORT_ERROR( "schd::cres" ) << name() << " " <<  err.what();
   }
   catch( ... ) {
      SCHD_REPORT_ERROR( "schd::cres" ) << name() << " Unexpected";
   }

   if( capacity <= 0.0 ) {
      SCHD_REPORT_ERROR( "schd::cres" ) << name() << " Incorrect capacity specification";
   }

   // Create exec instances and the corresponding channels
   BOOST_FOREACH( const boost_pt::ptree::value_type& exec_el, _exec_p.get()) {
      if( !exec_el.first.empty()) {
         SCHD_REPORT_ERROR( "schd::cres" ) << name() <<  " Incorrect exec structure";
      }

      boost::optional<std::string> exec_name_p = exec_el.second.get_optional<std::string>("name");

      if( !exec_name_p.is_initialized()) {
         SCHD_REPORT_ERROR( "schd::cres" ) << name() <<  " Incorrect exec structure";
      }

      // Initialise exec data
      schd_cres_c::exec_data_t exec_data;

      exec_list.emplace( std::make_pair( exec_name_p.get(), exec_data ));
   } // BOOST_FOREACH( const boost_pt::ptree::value_type& exec_el, _exec_p.get())
} // void schd_cres_c::init(

void schd_cres_c::add_trace(
      sc_core::sc_trace_file* tf,
      const std::string& top_name ) {

   std::string mod_name = top_name + "." + name() + ".";

   // Add trace for cres
   BOOST_FOREACH( exec_list_t::value_type& exec_el, exec_list ) {
      sc_core::sc_trace(
            tf,
            &( exec_el.second.connected ),
            mod_name + exec_el.first + ".connected" );

      sc_core::sc_trace(
            tf,
            &( exec_el.second.demand ),
            mod_name + exec_el.first + ".demand" );
   }

   // Add trace for common parameters
   sc_core::sc_trace(
         tf,
         &( demand ),
         mod_name + "demand" );

   sc_core::sc_trace(
         tf,
         &( capacity ),
         mod_name + "capacity" );
} // schd_cres_c::add_trace(

void schd_cres_c::exec_thrd( void ) {
   schd_dump_buf_c<boost_pt::ptree> dump_buf_exec_i( std::string( name()) + ".exec_i" );
   schd_dump_buf_c<boost_pt::ptree> dump_buf_exec_o( std::string( name()) + ".exec_o" );

   sc_core::wait(sc_core::SC_ZERO_TIME);

   for(;;) {
      sc_core::wait( exec_i->data_written_event());

      while( exec_i->num_available()) {
         boost_pt::ptree        exec_pt_raw = exec_i->read().get();
         const boost_pt::ptree& exec_pt_inp = exec_pt_raw;

         // Extract data from the received signal
         boost::optional<std::string> exec_p = exec_pt_inp.get_optional<std::string>("src");    // exec name
         boost::optional<bool>        conn_p = exec_pt_inp.get_optional<bool>("connected");     // connected state
         boost::optional<double>      dmnd_p = exec_pt_inp.get_optional<double>("demand");      // demand

         if( !exec_p.is_initialized() ||
             !conn_p.is_initialized() ||
             !dmnd_p.is_initialized()) {
            SCHD_REPORT_ERROR( "schd::cres" ) << name() << " Incorrect data format from exec";
         }

         // Dump pt packets as they arrive to the input of the block
         dump_buf_exec_i.write( exec_pt_inp, BUF_WRITE_LAST );

         // Update the record for the exec
         auto exec_it = exec_list.find( exec_p.get());

         if( exec_it == exec_list.end()) {
            SCHD_REPORT_ERROR( "schd::cres" )
                  << name()
                  << " Unexpected name of the execution block: "
                  << exec_p.get();
         }
         else if(( !conn_p.get() && dmnd_p.get() > 0 ) || ( dmnd_p.get() < 0 )) {
            SCHD_REPORT_ERROR( "schd::cres" )
                  << name()
                  << " Data integrity failed from: "
                  << exec_p.get();
         }

         bool conn_prev = exec_it->second.connected;
         exec_it->second.connected = conn_p.get();
         exec_it->second.demand    = dmnd_p.get();

         boost_pt::ptree dst_list_pt;
         double demand_acc = 0;

         BOOST_FOREACH( const exec_list_t::value_type& exec_el, exec_list ) {
            if( exec_el.second.connected ) { // Accumulate demand
               demand_acc += exec_el.second.demand;
            }
         }

         // Broadcast updated demand to the exec blocks
         if( demand != demand_acc ) {
            demand = demand_acc;

            BOOST_FOREACH( const exec_list_t::value_type& exec_el, exec_list ) {
               if( exec_el.second.connected ) {
                  dst_list_pt.push_back( std::make_pair( "", boost_pt::ptree().put( "", exec_el.first ) ));
               }
            }
         }
         else if( conn_prev == false && exec_it->second.connected == true ) {
            dst_list_pt.push_back( std::make_pair( "", boost_pt::ptree().put( "", exec_it->first ) ));
         }

         if( !dst_list_pt.empty()) {
            // Create property tree to broadcast data to all execution units
            boost_pt::ptree cres_pt;

            cres_pt.put(      "src",       name()      );   // Name of this resource
            cres_pt.put_child("dst",       dst_list_pt );   // Update all exec units involved
            cres_pt.put(      "demand",    demand      );   // Resource demand
            cres_pt.put(      "connected", true        );   // Connected state

            schd_sig_ptree_c pt_out;

            exec_o->write( pt_out.set( cres_pt )); // Write data to the output

            // Dump pt packets as they depart from the output of the block
            dump_buf_exec_o.write( cres_pt, BUF_WRITE_LAST );
         } // if( !dst_list_pt.empty()) {

         // Just disconnected
         if( conn_prev == true && exec_it->second.connected == false ) {
            dst_list_pt.clear();

            dst_list_pt.push_back( std::make_pair( "", boost_pt::ptree().put( "", exec_it->first ) ));

            // Create property tree to broadcast data to all execution units
            boost_pt::ptree cres_pt;

            cres_pt.put(      "src",       name()      );   // Name of this resource
            cres_pt.put_child("dst",       dst_list_pt );   // Update all exec units involved
            cres_pt.put(      "demand",    demand      );   // Resource demand
            cres_pt.put(      "connected", false       );   // Connected state

            schd_sig_ptree_c pt_out;

            exec_o->write( pt_out.set( cres_pt )); // Write data to the output

            // Dump pt packets as they depart from the output of the block
            dump_buf_exec_o.write( cres_pt, BUF_WRITE_LAST );
         } // if( conn_prev == true && ...
      } // while( res_i->num_available()) {
   } // for(;;)
} // void schd_cres_c::exec_thrd(

} // namespace schd
