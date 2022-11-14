#ifndef INTERLOCK_BRIDGE_CONFIG
#define INTERLOCK_BRIDGE_CONFIG

#include <stdint.h>
#include"EnvironmentProvider.h"

// These structures are derived

typedef struct {
  uint16_t rim1_dout;
  uint16_t ib_dout;
} pd_out_t; // 10-200245-00_sqa_self_test_v1

typedef struct {
  uint16_t rim1_dout_rb;
  uint16_t rim1_din;
  uint16_t ib_dout_rb;
  uint16_t ib_din;
} pd_in_t; // 10-200245-00_sqa_self_test_v1


class InterlockBridge : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*SkeletonTest*"; }
    virtual string GetDescription() { return "InterlockBridge"; }
};

#endif

