/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "lib/scheduler/support/mcs_calculator.h"
#include <gtest/gtest.h>

using namespace srsran;

TEST(test_manual_values, test)
{
  ASSERT_EQ(0, map_cqi_to_mcs(1, srsran::pdsch_mcs_table::qam64).value());
  ASSERT_EQ(0, map_cqi_to_mcs(2, srsran::pdsch_mcs_table::qam64).value());
  ASSERT_EQ(2, map_cqi_to_mcs(3, srsran::pdsch_mcs_table::qam64).value());
  ASSERT_EQ(4, map_cqi_to_mcs(4, srsran::pdsch_mcs_table::qam64).value());
  ASSERT_EQ(6, map_cqi_to_mcs(5, srsran::pdsch_mcs_table::qam64).value());
  ASSERT_EQ(8, map_cqi_to_mcs(6, srsran::pdsch_mcs_table::qam64).value());
  ASSERT_EQ(11, map_cqi_to_mcs(7, srsran::pdsch_mcs_table::qam64).value());
  ASSERT_EQ(13, map_cqi_to_mcs(8, srsran::pdsch_mcs_table::qam64).value());
  ASSERT_EQ(15, map_cqi_to_mcs(9, srsran::pdsch_mcs_table::qam64).value());
  ASSERT_EQ(18, map_cqi_to_mcs(10, srsran::pdsch_mcs_table::qam64).value());
  ASSERT_EQ(20, map_cqi_to_mcs(11, srsran::pdsch_mcs_table::qam64).value());
  ASSERT_EQ(22, map_cqi_to_mcs(12, srsran::pdsch_mcs_table::qam64).value());
  ASSERT_EQ(24, map_cqi_to_mcs(13, srsran::pdsch_mcs_table::qam64).value());
  ASSERT_EQ(26, map_cqi_to_mcs(14, srsran::pdsch_mcs_table::qam64).value());
  ASSERT_EQ(28, map_cqi_to_mcs(15, srsran::pdsch_mcs_table::qam64).value());
}

struct snr_to_ul_mcs_test_params {
  double                  snr;
  srsran::pusch_mcs_table mcs_table;
  sch_mcs_index           expected_mcs;
};

class snr_to_ul_mcs_tester : public ::testing::TestWithParam<snr_to_ul_mcs_test_params>
{};

TEST_P(snr_to_ul_mcs_tester, compare_bin_search_with_linear)
{
  ASSERT_EQ(GetParam().expected_mcs, map_snr_to_mcs_ul(GetParam().snr, GetParam().mcs_table).to_uint());
}

// Compare the binary search with linear search for a given interval.
INSTANTIATE_TEST_SUITE_P(test_snr_range,
                         snr_to_ul_mcs_tester,
                         testing::Values(snr_to_ul_mcs_test_params{-20.0, srsran::pusch_mcs_table::qam64, {0}},
                                         snr_to_ul_mcs_test_params{40.0, srsran::pusch_mcs_table::qam64, {28}},
                                         snr_to_ul_mcs_test_params{0.0133, srsran::pusch_mcs_table::qam64, {0}},
                                         snr_to_ul_mcs_test_params{13.0, srsran::pusch_mcs_table::qam64, {15}}));
