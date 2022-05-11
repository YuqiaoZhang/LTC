#include "generated.h"

static uint32_t _internal_ltc_vs_code[] = {
#include "generated/ltc.vert.inc"
};

uint32_t *_ltc_vs_code = _internal_ltc_vs_code;
uint32_t _ltc_vs_code_len = sizeof(_internal_ltc_vs_code);

static uint32_t _internal_ltc_fs_code[] = {
#include "generated/ltc.frag.inc"
};

uint32_t *_ltc_fs_code = _internal_ltc_fs_code;
uint32_t _ltc_fs_code_len = sizeof(_internal_ltc_fs_code);

#include "generated/ltc_mat.h"
unsigned char *_ltc_mat_asset = assets_ltc_mat_dds;
unsigned int _ltc_mat_asset_len = assets_ltc_mat_dds_len;

#include "generated/ltc_mag.h"
unsigned char *_ltc_mag_asset = assets_ltc_mag_dds;
unsigned int _ltc_mag_asset_len = assets_ltc_mag_dds_len;
