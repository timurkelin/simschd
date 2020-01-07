/*
 * schd_core.h
 *
 *  Description:
 *    Declaration of the execution core
 */

#ifndef SCHD_CORE_INCLUDE_SCHD_CORE_H_
#define SCHD_CORE_INCLUDE_SCHD_CORE_H_

#include <map>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/optional.hpp>
#include <systemc>
#include "schd_sig_ptree.h"
#include "schd_ptree_xbar.h"
#include "schd_exec.h"
#include "schd_cres.h"

// Short alias for the namespace
namespace boost_pt = boost::property_tree;

namespace schd {

   SC_MODULE( schd_core_c ) { // declare module class

   public:
      // Module I/O Ports
      sc_core::sc_export<sc_core::sc_fifo_in_if <schd_sig_ptree_c>> plan_ei;
      sc_core::sc_export<sc_core::sc_fifo_out_if<schd_sig_ptree_c>> plan_eo;

      // Constructor declaration
      SC_CTOR( schd_core_c );

      // Init declaration (to be used after the instantiation and before the port binding)
      void init(
            boost::optional<const boost_pt::ptree&> _exec_p,      // exec section of the preferences
            boost::optional<const boost_pt::ptree&> _cres_p );    // cres section of the preferences

   private:
      typedef struct {
         std::size_t                             idx;    // Block index
         boost::optional<schd_exec_c&>           mod_p;  // Pointer to module
         boost::optional<const boost_pt::ptree&> pref_p; // Pointer to preferences
         boost::optional<sc_core::sc_fifo<schd_sig_ptree_c>&> chn_plan_exec_p;
         boost::optional<sc_core::sc_fifo<schd_sig_ptree_c>&> chn_exec_plan_p;
         boost::optional<sc_core::sc_fifo<schd_sig_ptree_c>&> chn_cres_exec_p;
         boost::optional<sc_core::sc_fifo<schd_sig_ptree_c>&> chn_exec_cres_p;
         static const int chn_plan_exec_size = 64;
         static const int chn_exec_plan_size = 64;
         static const int chn_cres_exec_size = 64;
         static const int chn_exec_cres_size = 64;
      } schd_exec_info_t;

      typedef std::map<std::string, schd_exec_info_t> exec_list_t;
      exec_list_t exec_list;

      typedef struct {
         std::size_t                             idx;    // Block index
         boost::optional<schd_cres_c&>           mod_p;  // Pointer to module
         boost::optional<const boost_pt::ptree&> pref_p; // Pointer to preferences
         boost::optional<sc_core::sc_fifo<schd_sig_ptree_c>&> chn_cres_exec_p;
         boost::optional<sc_core::sc_fifo<schd_sig_ptree_c>&> chn_exec_cres_p;
         static const int chn_cres_exec_size = 64;
         static const int chn_exec_cres_size = 64;
      } schd_cres_info_t;

      typedef std::map<std::string, schd_cres_info_t> cres_list_t;
      cres_list_t cres_list;

      sc_core::sc_fifo<schd_sig_ptree_c> chn_core_plan;
      sc_core::sc_fifo<schd_sig_ptree_c> chn_plan_core;

      static const int chn_core_plan_size = 64;
      static const int chn_plan_core_size = 64;

      schd_ptree_xbar_c mux_plan_exec;
      schd_ptree_xbar_c mux_exec_plan;

      schd_ptree_xbar_c mux_cres_exec;
      schd_ptree_xbar_c mux_exec_cres;

   }; // SC_MODULE( schd_core_c )
} // namespace schd

#endif /* SCHD_CORE_INCLUDE_SCHD_CORE_H_ */
