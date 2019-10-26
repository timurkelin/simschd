/*
 * schd_conv_ptree.h
 *
 *  Description:
 *    Declaration of the conversion functions for boost property tree
 */

#ifndef SCHD_COMMON_INCLUDE_SCHD_CONV_PTREE_H_
#define SCHD_COMMON_INCLUDE_SCHD_CONV_PTREE_H_

#include <string>
#include <boost/property_tree/ptree.hpp>

// Short alias for the namespace
namespace boost_pt = boost::property_tree;

namespace schd {

boost_pt::ptree& str2pt(
      const std::string& str_,
      boost_pt::ptree&   pt_ );

std::string& pt2str(
      const boost_pt::ptree& pt_,
      std::string&           str_ );

} // namespace schd

#endif /* SCHD_COMMON_INCLUDE_SCHD_CONV_PTREE_H_ */
