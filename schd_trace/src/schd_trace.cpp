/*
 * simd_trace.cpp
 *
 *  Description:
 *    VCD trace and map
 */

#include <utility>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>
#include "schd_trace.h"
#include "schd_report.h"

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
         viewer = pref.get<std::string>("viewer");
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

   boost::optional<std::string> trace_p  = pref.get_optional<std::string>("file");

   if( trace_p.is_initialized() && ( trace_p.get().length() != 0 )) {
      // Open file and write prefix line
      std::string  fn_ext = trace_p.get() + trace_map_p.get().file_ext;

      std::ofstream os;

      os.exceptions(
            std::ofstream::badbit |
            std::ofstream::failbit );

      try {
         os.open(
               fn_ext.c_str(),
               std::ofstream::out |
               std::ofstream::app );
      }
      catch( const std::ofstream::failure &err ) {
         SCHD_REPORT_ERROR( "simd::trace" ) << err.what();
      }

      std::vector<std::string> thrd_task_proc; // Storage of all the processed instances
      boost::optional<const boost_pt::ptree&> map_list_p = pref.get_child_optional("map");

      // Cycle through the list of threads
      BOOST_FOREACH( const boost_pt::ptree::value_type& thrd, _thrd_p.get()) {
         if( !thrd.first.empty()) {
            SCHD_REPORT_ERROR( "schd::trace" ) << "Incorrect structure";
         }

         boost::optional<std::string> thrd_name_p = thrd.second.get_optional<std::string>("name");

         if( !thrd_name_p.is_initialized()) {
            SCHD_REPORT_ERROR( "schd::trace" ) << "Incorrect structure";
         }

         boost::optional<const boost_pt::ptree&> seq_p = thrd.second.get_child_optional("seq");

         if( !seq_p.is_initialized()) {
            SCHD_REPORT_ERROR( "schd::trace" ) << "Incorrect structure";
         }

         // Cycle through the instances of the thread and process tasks
         BOOST_FOREACH( const boost_pt::ptree::value_type& seq_el, seq_p.get()) {
            if( !seq_el.first.empty()) {
               SCHD_REPORT_ERROR( "schd::trace" ) << "Incorrect structure";
            }

            boost::optional<std::string> task_name_p = seq_el.second.get_optional<std::string>("task");

            if( !task_name_p.is_initialized()) {
               continue;
            }

            // Cycle through the names to process
            std::vector<std::string> name_vec = {
                  task_name_p.get(),
                  thrd_name_p.get() + "." + task_name_p.get()
            };

            BOOST_FOREACH( const std::string& name_el, name_vec ) {
               // Check if the name has been processed already
               if( std::find( thrd_task_proc.begin(),
                              thrd_task_proc.end(),
                              name_el ) != thrd_task_proc.end()) {
                  continue;
               }

               thrd_task_proc.push_back( name_el ); // Save element

               // Find map for this name
               boost::optional<const boost_pt::ptree&> map_p;

               if( map_list_p.is_initialized()) {
                  BOOST_FOREACH( const boost_pt::ptree::value_type& map_el, map_list_p.get()) {
                     boost::regex re;

                     if( !map_el.first.empty()) {
                        SCHD_REPORT_ERROR( "schd::trace" ) << "Incorrect structure";
                     }

                     try {
                        re = map_el.second.get<std::string>("name_regex"); // assign to regex
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

                     if( boost::regex_match( name_el, re )) {
                        if( !map_p.is_initialized()) {
                           map_p = map_el.second;
                        }
                        else {
                           SCHD_REPORT_ERROR( "schd::trace" ) << "Duplicate map for " << name_el;
                        }
                     }
                  } // BOOST_FOREACH( const boost_pt::ptree::value_type& map_el, map_list_p.get()) {
               } // if( map_list_p.is_initialized()) {



            } // BOOST_FOREACH( const std::string& name_el, name_vec )
         } // BOOST_FOREACH( const boost_pt::ptree::value_type& seq_el, seq_p.get()) {
      } // BOOST_FOREACH( const boost_pt::ptree::value_type& thrd, _thrd_p.get())

      // Write suffix line and close the file stream
      if( viewer == "gtkwave" ) { // GTK Wave

      }
      else if( viewer == "simvision" ) { // Cadence SimVision

      }

      os.close();
   } // if( trace_p.is_initialized() && ( trace_p.get().length() != 0 )) {
}

schd_trace_c::crgb_t schd_trace_c::rgb_split( const std::size_t rgb_c ) {
   return std::make_tuple(
      ( rgb_c >> 16 ) & 0xff,
      ( rgb_c >>  8 ) & 0xff,
      ( rgb_c       ) & 0xff );
}

std::size_t schd_trace_c::rgb_comb(  const schd_trace_c::crgb_t rgb_s ) {
   return ( std::get<0>( rgb_s ) << 16 ) |
          ( std::get<1>( rgb_s ) <<  8 ) |
          ( std::get<2>( rgb_s ));
}

std::string schd_trace_c::rgb2x11name( const std::size_t  rgb_c ) {
   crgb_t rgb_s = rgb_split( rgb_c );

   boost::optional<cmap_t> min_el_p;
   std::size_t             min_se = UINT_MAX;

   BOOST_FOREACH( const cmap_t& cel, cmap ) {
      int dist_r = std::get<0>( std::get<1>( cel )) - std::get<0>( rgb_s );
      int dist_g = std::get<1>( std::get<1>( cel )) - std::get<1>( rgb_s );
      int dist_b = std::get<2>( std::get<1>( cel )) - std::get<2>( rgb_s );

      std::size_t se = dist_r * dist_r +
                       dist_g * dist_g +
                       dist_b * dist_b;

      if( se == 0 ) {
         min_el_p = cel;
         break;
      }
      else {
         if( min_se > se ) {
            min_se = se;
            min_el_p = cel;
         }
      }
   }

   return std::get<0>( min_el_p.get() );
}

std::size_t schd_trace_c::x11name2rgb( const std::string& rgb_n ) {
   crgb_t rgb_s = std::make_tuple( 0, 0, 0 );
   bool   found = false;

   BOOST_FOREACH( const cmap_t& cel, cmap ) {
      if( std::get<0>( cel ) == rgb_n ) {
         rgb_s = std::get<1>( cel );
         found = true;
         break;
      }
   }

   if( !found ) {
      SCHD_REPORT_ERROR( "schd::trace" ) << "color name not found: " << rgb_n;
   }

   return rgb_comb( rgb_s );
}

schd_trace_c::~schd_trace_c(
      void ) {
   if( tf ) {
      sc_core::sc_close_vcd_trace_file( tf );
   }
} // schd_trace_c::~schd_trace_c(

// Trace instance
schd_trace_c simd_trace;

} // namespace schd
