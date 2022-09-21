/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "pucch_detector_test_doubles.h"
#include "pucch_processor_format1_test_data.h"
#include <gtest/gtest.h>

using namespace srsgnb;

namespace srsgnb {

std::ostream& operator<<(std::ostream& os, const pucch_processor::common_configuration& config)
{
  return os << fmt::format(
             "slot={}, bwp={}:{}, cp={}, start={}, hop={}, n_id={}, n_id_0={}, Nsr={}, Nack={}, Ncsi1={}, Ncsi2={}",
             config.slot,
             config.bwp_start_rb,
             config.bwp_size_rb,
             config.cp,
             config.starting_prb,
             config.second_hop_prb.has_value() ? std::to_string(config.second_hop_prb.value()) : "na",
             config.n_id,
             config.n_id_0,
             config.nof_sr,
             config.nof_harq_ack,
             config.nof_csi_part1,
             config.nof_csi_part2);
}

std::ostream& operator<<(std::ostream& os, const pucch_processor::format1_configuration& config)
{
  return os << config.common
            << fmt::format(", cs={}, nsymb={}, start_symb={}, occ={}",
                           config.initial_cyclic_shift,
                           config.nof_symbols,
                           config.start_symbol_index,
                           config.time_domain_occ);
}

std::ostream& operator<<(std::ostream& os, const test_case_t& tc)
{
  return os << tc.config;
}

} // namespace srsgnb

namespace {

using PucchProcessorFormat1Param = test_case_t;

class PucchProcessorFormat1Fixture : public ::testing::TestWithParam<PucchProcessorFormat1Param>
{
protected:
  static void SetUpTestSuite()
  {
    if (factory) {
      return;
    }

    std::shared_ptr<low_papr_sequence_generator_factory> lpg_factory = create_low_papr_sequence_generator_sw_factory();
    ASSERT_NE(lpg_factory, nullptr);

    std::shared_ptr<low_papr_sequence_collection_factory> lpc_factory =
        create_low_papr_sequence_collection_sw_factory(lpg_factory);
    ASSERT_NE(lpc_factory, nullptr);

    std::shared_ptr<pseudo_random_generator_factory> prg_factory = create_pseudo_random_generator_sw_factory();
    ASSERT_NE(prg_factory, nullptr);

    std::shared_ptr<dmrs_pucch_estimator_factory> dmrs_factory =
        create_dmrs_pucch_estimator_factory_sw(prg_factory, lpc_factory);
    ASSERT_NE(dmrs_factory, nullptr);

    std::shared_ptr<pucch_detector_factory> detector_factory = std::make_shared<pucch_detector_factory_spy>();
    ASSERT_NE(detector_factory, nullptr);

    channel_estimate::channel_estimate_dimensions channel_estimate_dimensions;
    channel_estimate_dimensions.nof_tx_layers = 1;
    channel_estimate_dimensions.nof_rx_ports  = 1;
    channel_estimate_dimensions.nof_symbols   = MAX_NSYMB_PER_SLOT;
    channel_estimate_dimensions.nof_prb       = MAX_RB;

    factory = create_pucch_processor_factory_sw(dmrs_factory, detector_factory, channel_estimate_dimensions);
    ASSERT_NE(factory, nullptr);
  }

  void SetUp() override
  {
    ASSERT_NE(factory, nullptr);
    processor = factory->create();
    ASSERT_NE(processor, nullptr);
  }

  static std::shared_ptr<pucch_processor_factory> factory;

  std::unique_ptr<pucch_processor> processor;
};

std::shared_ptr<pucch_processor_factory> PucchProcessorFormat1Fixture::factory;

TEST_P(PucchProcessorFormat1Fixture, FromVector)
{
  // Prepare resource grid.
  resource_grid_reader_spy grid;
  grid.write(GetParam().data_symbols.read());
  grid.write(GetParam().dmrs_symbols.read());

  const PucchProcessorFormat1Param& param = GetParam();

  pucch_processor_result result = processor->process(grid, param.config);

  ASSERT_EQ(result.csi.time_alignment, phy_time_unit::from_seconds(0));
  ASSERT_EQ(result.csi.epre_dB, 0.0);
  ASSERT_EQ(result.csi.sinr_dB, 0.0);
}

INSTANTIATE_TEST_SUITE_P(PucchProcessorFormat1,
                         PucchProcessorFormat1Fixture,
                         ::testing::ValuesIn(pucch_processor_format1_test_data));

} // namespace
