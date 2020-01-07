/*
 * schd_trace.cpp
 *
 *  Description:
 *    VCD trace and map
 */

#include <utility>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <boost/functional/hash.hpp>
#include "schd_trace.h"
#include "schd_report.h"

#include "schd_trace_map_gtkwave.h"
#include "schd_trace_map_simvision.h"

namespace schd {

void schd_trace_c::init(
      boost::optional<const boost_pt::ptree&> _pref_p ) {

   pref_p = _pref_p;

   const boost_pt::ptree& pref = pref_p.get();

   boost::optional<std::string> trace_p = pref.get_optional<std::string>("file");

   // Set vcd file name
   if( trace_p.is_initialized() && ( trace_p.get().length() != 0 )) {
      tf = sc_core::sc_create_vcd_trace_file( trace_p.get().c_str() );

      if( tf ) {
         SCHD_REPORT_INFO(  "schd::trace" ) << "VCD trace is set to <" << trace_p.get() << ".vcd>";
      }
      else {
         SCHD_REPORT_ERROR( "schd::trace" ) << "Unable to set VCD trace <" << trace_p.get() << ".vcd>";
      }

      std::string viewer;

      try {
         viewer      = pref.get<std::string>("viewer");
      }
      catch( const boost_pt::ptree_error& err ) {
         SCHD_REPORT_ERROR( "schd::trace" ) << err.what();
      }
      catch( ... ) {
         SCHD_REPORT_ERROR( "schd::trace" ) << "Unexpected";
      }

      schd_trace_map_c *_ptr = NULL;

      if(      viewer == "gtkwave"   ) _ptr = new schd_trace_map_gtkwave_c();    // GTK Wave
      else if( viewer == "simvision" ) _ptr = new schd_trace_map_simvision_c();  // Cadence SimVision
      else {
         SCHD_REPORT_ERROR( "schd::trace" ) << "Unsupported viewer";
      }

      trace_map_p = boost::optional<schd_trace_map_c&>( *_ptr );
   }
   else {
      SCHD_REPORT_INFO( "schd::trace" ) << "VCD trace is disabled";
      tf = NULL;
   }
} // schd_trace_c::init(

void schd_trace_c::save_map(
      boost::optional<const boost_pt::ptree&> _thrd_p ) {

   const boost_pt::ptree& pref = pref_p.get();

   boost::optional<std::string> trace_p = pref.get_optional<std::string>("file");

   if( !trace_p.is_initialized() || ( trace_p.get().length() == 0 )) {
      return;
   }

   tag_list_t tag_list; // Storage of all the processed instances
   tag_data_t tag_data;

   // Create entry for idle
   tag_data.comb = "";
   tag_data.hash = 0;
   tag_list.push_back( tag_data );
   tag_list.begin()->thrd_name_p =
         boost::optional<std::string>( tag_list.begin()->comb );
   tag_list.begin()->task_name_p =
         boost::optional<std::string>( tag_list.begin()->comb );
   tag_list.begin()->task_prid_p =
         boost::optional<std::string>( tag_list.begin()->comb );

   // Cycle through the list of threads
   BOOST_FOREACH( const boost_pt::ptree::value_type& thrd, _thrd_p.get()) {
      if( !thrd.first.empty()) {
         SCHD_REPORT_ERROR( "schd::trace" ) << "Incorrect structure";
      }

      tag_data.thrd_name_p = thrd.second.get_optional<std::string>("name");

      if( !tag_data.thrd_name_p.is_initialized()) {
         SCHD_REPORT_ERROR( "schd::trace" ) << "Incorrect structure";
      }

      boost::optional<const boost_pt::ptree&> seq_p = thrd.second.get_child_optional("sequence");

      if( !seq_p.is_initialized()) {
         SCHD_REPORT_ERROR( "schd::trace" )
               << "Incorrect structure for task "
               << tag_data.thrd_name_p.get();
      }

      // Cycle through the list of tasks and skip events
      BOOST_FOREACH( const boost_pt::ptree::value_type& seq_el, seq_p.get()) {
         if( !seq_el.first.empty()) {
            SCHD_REPORT_ERROR( "schd::trace" )
                  << "Incorrect structure for task "
                  << tag_data.thrd_name_p.get();
         }

         boost::optional<const boost_pt::ptree&> task_child_p = seq_el.second.get_child_optional("task");

         // Only tasks are visualised. Skip events
         if( !task_child_p.is_initialized()) {
            continue;
         }

         tag_data.task_name_p = task_child_p.get().get_optional<std::string>("run");

         boost::optional<const boost_pt::ptree&> task_param_p =
               task_child_p.get().get_child_optional("param");

         if( !tag_data.task_name_p.is_initialized() ||
             !task_param_p.is_initialized()) {
            SCHD_REPORT_ERROR( "schd::trace" )
                  << "Incorrect structure for task "
                  << tag_data.thrd_name_p.get();
         }

         tag_data.task_prid_p = task_param_p.get().get_optional<std::string>("id");

         if( !tag_data.task_prid_p.is_initialized()) {
            SCHD_REPORT_ERROR( "schd::trace" )
                  << "Incorrect structure for task "
                  << tag_data.thrd_name_p.get();
         }

         // Create tag which is visualised in the waveform viewer
         tag_data.comb = job_comb(
               tag_data.thrd_name_p.get(),
               tag_data.task_name_p.get(),
               tag_data.task_prid_p.get());

         tag_data.hash = 0;
         boost::hash_combine(
               tag_data.hash,
               tag_data.comb );

         // Check if the name has been processed already
         if( tag_list.end() == std::find_if(
               tag_list.begin(),
               tag_list.end(),
               [tag_data]( const tag_list_t::value_type& el )->bool {
                  return tag_data.hash == el.hash; } )) {
            tag_list.push_back( tag_data );
         }
      } // BOOST_FOREACH( const boost_pt::ptree::value_type& seq_el, seq_p.get())
   } // BOOST_FOREACH( const boost_pt::ptree::value_type& thrd, _thrd_p.get())

   // Open file and write prefix line
   std::string  fn_ext = trace_p.get() + trace_map_p.get().file_ext;
   std::ofstream os;

   os.exceptions(
         std::ofstream::badbit |
         std::ofstream::failbit );
   try {
      os.open(
            fn_ext.c_str(),
            std::ofstream::out );
   }
   catch( const std::ofstream::failure &err ) {
      SCHD_REPORT_ERROR( "simd::trace" ) << err.what();
   }

   os << trace_map_p.get().map_pref();

   boost::optional<const boost_pt::ptree&> map_list_p = pref.get_child_optional("map");

   if( !map_list_p.is_initialized() ) {
      SCHD_REPORT_ERROR( "schd::trace" ) << "View map is not initialised";
   }

   // Process the list of the exec states
   BOOST_FOREACH( const tag_list_t::value_type& tag_el, tag_list ) {
      // Find a map for this name
      boost::optional<const boost_pt::ptree&> map_p;
      std::stringstream                       tag_view_os;

      BOOST_FOREACH( const boost_pt::ptree::value_type& map_el, map_list_p.get()) {
         if( !map_el.first.empty()) {
            SCHD_REPORT_ERROR( "schd::trace" ) << "Incorrect structure";
         }

         boost::regex re;
         std::string  fmt;

         try {
            re  = map_el.second.get<std::string>("mask"); // assign to regex
            fmt = map_el.second.get<std::string>("format");
         }
         catch( const boost_pt::ptree_error& err ) {
            SCHD_REPORT_ERROR( "schd::trace" ) << err.what();
         }
         catch( const boost::regex_error& err ) {
            SCHD_REPORT_ERROR( "schd::trace" ) << err.what();
         }
         catch( ... ) {
            SCHD_REPORT_ERROR( "schd::trace" ) << "Unexpected";
         }

         if( boost::regex_match( tag_el.comb, re )) {
            try {
               tag_view_os << boost::format( fmt ) %
                     tag_el.thrd_name_p.get() %
                     tag_el.task_name_p.get() %
                     tag_el.task_prid_p.get();
            }
            catch (boost::io::format_error& err) {
               SCHD_REPORT_ERROR( "schd::trace" ) << err.what();
            }
            catch( ... ) {
               SCHD_REPORT_ERROR( "schd::trace" ) << "Unexpected";
            }

            map_p = boost::optional<const boost_pt::ptree&>( map_el.second );
            break;
         }
      } // BOOST_FOREACH( const boost_pt::ptree::value_type& map_el, map_list_p.get()) {

      // Write name mapping.
      os << trace_map_p.get().map_elem( tag_el.hash, tag_view_os.str(), map_p );
   } // BOOST_FOREACH( const tag_list_t::value_type& tag_el, tag_list )

   // Write suffix line and close the file stream
   os << trace_map_p.get().map_suff();

   os.close();
}

schd_trace_c::~schd_trace_c(
      void ) {
   if( tf ) {
      sc_core::sc_close_vcd_trace_file( tf );
   }
}

std::string schd_trace_c::job_comb(
      const std::string& _thrd_str,
      const std::string& _task_str,
      const std::string& _param_str ) {
   return _thrd_str + "#" +
          _task_str + "#" +
          _param_str;
} // std::size_t  schd_trace_c::comb_hash(

// Trace instance
schd_trace_c schd_trace;

} // namespace schd
