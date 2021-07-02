/*
    This file is part of duckOS.

    duckOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    duckOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with duckOS.  If not, see <https://www.gnu.org/licenses/>.

    Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#ifndef DUCKOS_ATA_H
#define DUCKOS_ATA_H

//Commands
#define ATA_READ_PIO          0x20
#define ATA_READ_PIO_EXT      0x24
#define ATA_READ_DMA          0xC8
#define ATA_READ_DMA_EXT      0x25
#define ATA_WRITE_PIO         0x30
#define ATA_WRITE_PIO_EXT     0x34
#define ATA_WRITE_DMA         0xCA
#define ATA_WRITE_DMA_EXT     0x35
#define ATA_CACHE_FLUSH       0xE7
#define ATA_CACHE_FLUSH_EXT   0xEA
#define ATA_PACKET            0xA0
#define ATA_IDENTIFY_PACKET   0xA1
#define ATA_IDENTIFY          0xEC

//Status
#define ATA_STATUS_ERR  0b00000001u
#define ATA_STATUS_IDX  0b00000010u
#define ATA_STATUS_CORR 0b00000100u
#define ATA_STATUS_DRQ  0b00001000u
#define ATA_STATUS_SRV  0b00010000u
#define ATA_STATUS_DF   0b00100000u
#define ATA_STATUS_RDY  0b01000000u
#define ATA_STATUS_BSY  0b10000000u

//Control registers
#define ATA_DATA        0x00
#define ATA_ERROR       0x01
#define ATA_FEATURES    0x01
#define ATA_SECCNT0     0x02
#define ATA_LBA0        0x03
#define ATA_LBA1        0x04
#define ATA_LBA2        0x05
#define ATA_DRIVESEL    0x06
#define ATA_COMMAND     0x07
#define ATA_STATUS      0x07
#define ATA_SECCNT1     0x08
#define ATA_LBA3        0x09
#define ATA_LBA4        0x0A
#define ATA_LBA5        0x0B
#define ATA_CONTROL     0x0C
#define ATA_ALTSTATUS   0x0C
#define ATA_DEVADDRESS  0x0D

//Busmaster registers
#define ATA_BM_STATUS 0x2
#define ATA_BM_PRDT 0x4

//Busmaster register values and stuff
#define ATA_BM_READ 0x8

//Other
#define ATA_IDENTITY_MODEL_NUMBER_START 27 //Words
#define ATA_IDENTITY_MODEL_NUMBER_LENGTH 40 //Bytes

typedef struct __attribute__((packed)) PRDT {
public:
	uint32_t addr; //Address of memory region
	uint16_t size; //Size (in bytes) of region (0 means 64k)
	uint16_t eot;
} PRDT;

typedef struct __attribute__((packed)) ATAIdentity {
	struct {
		uint8_t : 1; //Reserved
		uint8_t : 1; //Retired
		uint8_t response_incomplete : 1;
		uint8_t : 3; //Retired
		uint8_t fixed_device : 1;
		uint8_t removable_media : 1;
		uint8_t : 7; //Retired
		uint8_t device_type : 1;
	} general_config;
	uint16_t num_cylinders;
	uint16_t specific_configuration;
	uint16_t num_heads;
	uint16_t retired_1[2];
	uint16_t sectors_per_track;
	uint16_t vendor_specific_1[3];
	uint8_t serial_number[20];
	uint16_t retired_2[2];
	uint16_t obselete_1;
	uint16_t firmware_revision[4];
	uint16_t model_number[20];
	uint8_t maximum_block_transfer;
	uint8_t vendor_specific_2;
	uint16_t trusted_computing;
	struct {
		uint8_t current_long_sector_physical_alignment : 2;
		uint8_t reserved_1 : 6;
		uint8_t dma_supported : 1;
		uint8_t lba_supported : 1;
		uint8_t iordy_disable : 1;
		uint8_t iordy_supported : 1;
		uint8_t reserved_2 : 1;
		uint8_t standby_support : 1;
		uint8_t reserved_3 : 2;
		uint16_t reserved_4;
	} capabilities;
	uint16_t obselete_2[2];
	uint16_t translation_fields_valid : 3;
	uint16_t reserved_3 : 5;
	uint8_t free_fall_control_sensitivity;
	uint16_t current_number_of_cylinders;
	uint16_t current_number_of_heads;
	uint16_t current_sectors_per_track;
	uint32_t current_sector_capacity;
	uint8_t current_multi_sector_setting;
	uint8_t multi_sector_setting_valid : 1;
	uint8_t reserved_4 : 3;
	uint8_t sanitize_supported : 1;
	uint8_t crypto_scramble_command_supported : 1;
	uint8_t overwrite_ext_command_supported : 1;
	uint8_t block_erase_ext_command_supported : 1;
	uint32_t user_addressable_sectors;
	uint16_t obselete_3;
	uint8_t multi_word_dma_support;
	uint8_t multi_word_dma_active;
	uint8_t advanced_pio_modes;
	uint8_t reserved_5;
	uint16_t minimum_mw_xfer_cycle_time;
	uint16_t recommended_mw_xfer_time;
	uint16_t minimum_pio_cycle_time;
	uint16_t minimum_pio_cycle_time_iordy;
	uint16_t additional_supported;
	uint16_t reserved_6[5];
	uint8_t queue_depth : 5;
	uint16_t sata_capabilities;
	uint16_t sata_additional_capabilities;
	uint16_t sata_features_supported;
	uint16_t sata_features_enabled;
	uint16_t major_version;
	uint16_t minor_version;
	uint16_t commands_supported[3];
	uint16_t commands_active[3];
	uint8_t udma_support;
	uint8_t udma_active;
	uint16_t security_timing[2];
	uint8_t apm_level;
	uint8_t reserved_7;
	uint16_t master_password_id;
	uint16_t hardware_reset_result;
	uint8_t acoustic_value;
	uint8_t recommended_acoustic_value;
	uint16_t stream_minimum_request_size;
	uint16_t stream_transfer_time_dma;
	uint16_t stream_access_latency_pio;
	uint32_t streaming_performance_granularity;
	uint64_t user_addressable_logical_sectors;
	uint16_t streaming_transfer_time;
	uint16_t dsm_cap;
	struct {
		uint8_t logical_sectors_per_physical_sector : 4;
		uint8_t reserved_1 : 8;
		uint8_t logical_sector_longer_than_256_words : 1;
		uint8_t multiple_logical_sectors_per_physical_sector : 1;
		uint8_t reserved_2 : 2;
	} physical_logical_sector_size;
	uint16_t inter_seek_delay;
	uint16_t world_wide_name[4];
	uint16_t world_wide_name_reserved[4];
	uint16_t technical_report_reserved;
	uint32_t words_per_logical_sector;
	uint16_t command_set_support_ext;
	uint16_t command_set_active_ext;
	uint16_t reserved_for_expanded_support_and_active[6];
	uint8_t msn_support : 2;
	uint16_t reserved_8 : 14;
	uint16_t security_status;
	uint16_t reserved_9[31];
	uint16_t cfa_power_mode;
	uint16_t reserved_for_cfa_1[7];
	uint8_t nominal_form_factor : 4;
	uint16_t reserved_10 : 12;
	struct {
		uint8_t supports_trim : 1;
		uint16_t reserved : 15;
	} data_set_management_features;
	uint16_t additional_product_id[4];
	uint16_t reserved_for_cfa_2[2];
	uint16_t current_media_serial_number[30];
	uint16_t sct_command_support;
	uint16_t reserved_11[2];
	struct {
		uint16_t alignment_of_logical_within_physical : 14;
		uint8_t word_209_supported : 1;
		uint8_t reserved : 1;
	} block_alignment;
	uint16_t write_read_verify_sector_mode_3_only[2];
	uint16_t write_read_verify_sector_mode_2_only[2];
	uint16_t nv_cache_capabilities;
	uint32_t nv_cache_size;
	uint16_t nominal_media_rotation_rate;
	uint16_t reserved_12;
	uint16_t nv_cache_options;
	uint8_t write_read_verify_sector_count_mode;
	uint8_t reserved_13;
	uint16_t reserved_14;
	struct {
		uint16_t major_version : 12;
		uint8_t transport_type : 4;
	} transport_major_version;
	uint16_t transport_minor_version;
	uint16_t reserved_15[6];
	uint64_t extended_number_of_user_addressable_sectors;
	uint16_t min_blocks_per_download_microcode_operation;
	uint16_t max_blocks_per_download_microcode_operation;
	uint16_t reserved_16[19];
	uint8_t signature;
	uint8_t checksum;
} Identity;

#endif //DUCKOS_ATA_H
