/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "pdsch_modulator_test_data.h"
#include "srsran/phy/support/support_factories.h"
#include "srsran/phy/upper/channel_processors/pdsch/factories.h"
#include "srsran/ran/pdsch/pdsch_constants.h"
#include "srsran/srsvec/bit.h"

using namespace srsran;

int main()
{
  std::shared_ptr<modulation_mapper_factory> modulator_factory = create_modulation_mapper_factory();
  TESTASSERT(modulator_factory);

  std::shared_ptr<pseudo_random_generator_factory> prg_factory = create_pseudo_random_generator_sw_factory();
  TESTASSERT(prg_factory);

  std::shared_ptr<channel_precoder_factory> precoding_factory = create_channel_precoder_factory("auto");
  TESTASSERT(precoding_factory);

  std::shared_ptr<resource_grid_mapper_factory> rg_mapper_factory =
      create_resource_grid_mapper_factory(precoding_factory);
  TESTASSERT(precoding_factory);

  std::shared_ptr<pdsch_modulator_factory> pdsch_factory =
      create_pdsch_modulator_factory_sw(modulator_factory, prg_factory, rg_mapper_factory);
  TESTASSERT(modulator_factory);

  std::unique_ptr<pdsch_modulator> pdsch = pdsch_factory->create();
  TESTASSERT(pdsch);

  for (const test_case_t& test_case : pdsch_modulator_test_data) {
    bounded_bitset<MAX_RB> prb_mask =
        test_case.config.freq_allocation.get_prb_mask(test_case.config.bwp_start_rb, test_case.config.bwp_size_rb);
    int prb_idx_high = prb_mask.find_highest();
    TESTASSERT(prb_idx_high > 1);
    unsigned max_prb   = static_cast<unsigned>(prb_idx_high + 1);
    unsigned max_symb  = get_nsymb_per_slot(cyclic_prefix::NORMAL);
    unsigned max_ports = test_case.config.precoding.get_nof_ports();

    // Prepare resource grid spy.
    resource_grid_writer_spy grid(max_ports, max_symb, max_prb);

    // Read codeword.
    std::vector<uint8_t> data = test_case.data.read();

    // Pack data.
    dynamic_bit_buffer packed_data(data.size());
    srsvec::bit_pack(packed_data, data);

    // Prepare codewords.
    static_vector<bit_buffer, pdsch_constants::MAX_NOF_CODEWORDS> codewords;
    codewords.emplace_back(packed_data);

    // Modulate.
    pdsch->modulate(grid, codewords, test_case.config);

    // Read resource grid data.
    std::vector<resource_grid_writer_spy::expected_entry_t> rg_entries = test_case.symbols.read();

    // Assert resource grid entries.
    grid.assert_entries(rg_entries, std::sqrt(max_ports));
  }

  return 0;
}
