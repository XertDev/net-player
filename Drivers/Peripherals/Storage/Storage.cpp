#include "Storage/Storage.hpp"
#include "stm32f4xx_hal.h"

Storage::Storage() {
	// TODO Auto-generated constructor stub

}

Storage::~Storage() {
	// TODO Auto-generated destructor stub
}

StorageErrors Storage::init(SD_HandleTypeDef& handle) {
	assert_param(initialized_ == false);

	FRESULT res;

	res = static_cast<FRESULT>(BSP_SD_Init());
	if(res != FR_OK) {
		return CARD_INIT_ERROR;
	}
	HAL_Delay(50);
	if(	HAL_SD_ConfigWideBusOperation(&handle, SDIO_BUS_WIDE_4B) != 0) {
		return BUS_WIDTH_ERROR;
	}

	HAL_Delay(50);

	res = f_mount(&SD_fat_fs_, "0:", 1);
	if(res != FR_OK) {
		return MOUNT_ERROR;
	}

	initialized_ = true;
	return OK;
}

void Storage::deInit() {
	f_mount(0, "", 0);
	initialized_ = false;
}

StorageErrors Storage::entriesInDirectoryCount(const char* dir_name, uint8_t& count) {
	DIR dir;
	FRESULT res;
	FILINFO file_info;

	res = f_opendir(&dir, dir_name);
	if(res != FR_OK) {
		if(res == FR_NO_PATH) {
			return NO_DIRECTORY;
		}
		return UNKNOWN_ERROR;
	}

	count = 0;
	res = f_readdir(&dir, &file_info);

	while(res == FR_OK && file_info.fname[0] != 0) {
		++count;
		res = f_readdir(&dir, &file_info);
	}

	if(res != FR_OK) {
		return UNKNOWN_ERROR;
	}

	f_closedir(&dir);

	return OK;
}

const FILINFO& Storage::DirectoryScanner::fileInfo() {
	return file_info_;
}

Storage::DirectoryScanner::DirectoryScanner(const char *dir_name) {
	res_ = f_opendir(&dir_, dir_name);
	if(res_ == FR_OK) {
		res_ = f_readdir(&dir_, &file_info_);
	}
}

Storage::DirectoryScanner::~DirectoryScanner() {
	f_closedir(&dir_);

}

void Storage::DirectoryScanner::next() {
	res_ = f_readdir(&dir_, &file_info_);
}

bool Storage::DirectoryScanner::valid() {
	return res_ == FR_OK && file_info_.fname[0] != 0;
}

bool Storage::openFile(const char *name, FIL& file) {
	auto error  = f_open(&file,  name,  FA_READ);
	return error == FR_OK;
}

void Storage::closeFile(FIL& fil) {
	f_close(&fil);
}

Storage::DirectoryScanner Storage::entriesInDirectoryScanner(const char *dir_name) {
	return DirectoryScanner(dir_name);
}
