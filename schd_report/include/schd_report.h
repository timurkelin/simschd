/*
 * schd_report.h
 *
 *  Description:
 *    Logging message handler
 */

#ifndef SCHD_REPORT_INCLUDE_SCHD_REPORT_H_
#define SCHD_REPORT_INCLUDE_SCHD_REPORT_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/optional/optional.hpp>
#include <systemc>

// Short alias for the namespace
namespace boost_pt = boost::property_tree;

namespace schd {
   void report_handler(
         const sc_core::sc_report& rep,
         const sc_core::sc_actions& actions );

   class schd_report_c {
   public:
      void init(
            boost::optional<const boost_pt::ptree&> pref_p );
   }; // class schd_report_c

   class sc_report_wrap_c {
   public:
      int state;
      std::stringstream msg;

      sc_report_wrap_c( int state_ini ) {
         state = state_ini;
      }

      sc_report_wrap_c( void ) {
         state = 0;
      }
   }; // class sc_report_wrap_c
} // namespace schd

#define SCHD_REPORT_INFO_VERB( msg_type, verbosity ) \
   for( schd::sc_report_wrap_c rep( 0 ); rep.state < 2; rep.state ++ ) \
      if( rep.state ) \
         sc_core::sc_report_handler::report( \
               sc_core::SC_INFO, \
               msg_type, \
               rep.msg.str().c_str(), \
               verbosity, \
               __FILE__ , \
               __LINE__ ); \
      else \
         rep.msg

#define SCHD_REPORT_WITH_SEVERITY( msg_type, severity ) \
   for( schd::sc_report_wrap_c rep( 0 ); rep.state < 2; rep.state ++ ) \
      if( rep.state ) \
         sc_core::sc_report_handler::report( \
               severity, \
               msg_type, \
               rep.msg.str().c_str(), \
               __FILE__ , \
               __LINE__ ); \
      else \
         rep.msg

#define SCHD_REPORT_INFO( msg_type )     SCHD_REPORT_WITH_SEVERITY( msg_type, sc_core::SC_INFO )
#define SCHD_REPORT_WARNING( msg_type )  SCHD_REPORT_WITH_SEVERITY( msg_type, sc_core::SC_WARNING )
#define SCHD_REPORT_ERROR( msg_type )    SCHD_REPORT_WITH_SEVERITY( msg_type, sc_core::SC_ERROR )
#define SCHD_REPORT_FATAL( msg_type )    SCHD_REPORT_WITH_SEVERITY( msg_type, sc_core::SC_FATAL )

#endif /* SCHD_REPORT_INCLUDE_SCHD_REPORT_H_ */
