/*
 * schd_dump_vec_wr.h
 *
 *  Description:
 *    Prototypes for vector writers for different data types
 */
#include <vector>
#include <string>
#include <complex>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <matio.h>

#ifndef SCHD_DUMP_INCLUDE_SCHD_DUMP_VEC_WR_H_
#define SCHD_DUMP_INCLUDE_SCHD_DUMP_VEC_WR_H_

// Short alias for the namespace
namespace boost_pt = boost::property_tree;
namespace boost_jp = boost::property_tree::json_parser;

namespace schd {

// Vector writers for basic types
matvar_t *vec_writer(
      const std::vector<int> &vec );
matvar_t *vec_writer(
      const std::vector<unsigned int> &vec );
matvar_t *vec_writer(
      const std::vector<std::size_t> &vec );
matvar_t *vec_writer(
      const std::vector<bool> &vec );
matvar_t *vec_writer(
      const std::vector<double> &vec );

// Vector writers for aggregate types
matvar_t *vec_writer(
      const std::vector<std::complex<double>> &vec );
matvar_t *vec_writer(
      const std::vector<std::string> &vec,
      const bool _copy = false );

// Vector writers for schd core types
matvar_t *vec_writer(
      const std::vector<boost_pt::ptree> &vec );

} // namespace schd

#endif /* SCHD_DUMP_INCLUDE_SCHD_DUMP_VEC_WR_H_ */
