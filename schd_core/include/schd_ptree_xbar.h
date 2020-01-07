/*
 * schd_ptree_xbar.h
 *
 *  Description:
 *    Declaration of the system component: ptree cross bar
 */

#ifndef SCHD_CORE_INCLUDE_SCHD_PTREE_XBAR_H_
#define SCHD_CORE_INCLUDE_SCHD_PTREE_XBAR_H_

#include <vector>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/optional.hpp>
#include <boost/regex.hpp>
#include <systemc>
#include "schd_sig_ptree.h"

// Short alias for the namespace
namespace boost_pt = boost::property_tree;

namespace schd {

   SC_MODULE( schd_ptree_xbar_c ) { // declare module class

   public:
      // Module I/O Ports
      sc_core::sc_vector<sc_core::sc_port<sc_core::sc_fifo_in_if <schd_sig_ptree_c>>> vi;
      sc_core::sc_vector<sc_core::sc_port<sc_core::sc_fifo_out_if<schd_sig_ptree_c>>> vo;

      // Constructor declaration
      SC_CTOR( schd_ptree_xbar_c );

      // Init declaration (to be used after the instantiation and before the port binding)
      void init(
            boost::optional<const boost_pt::ptree&> _pref_p );

   private:
      // Process declarations
      void exec_thrd(
            void );

      class port_map_t {
      public:
         bool         regex = false;
         std::string  name;
         boost::regex mask;
      };

      std::vector<port_map_t> vi_map;
      std::vector<port_map_t> vo_map;

   }; // SC_MODULE( schd_ptree_xbar_c )
} // namespace schd

#endif /* SCHD_CORE_INCLUDE_SCHD_PTREE_XBAR_H_ */
