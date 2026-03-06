/*
 * (C) Copyright 2020
 * Cheng Lei, Tuya Software Engineering, chenglei@tuya.com.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include "notifier.h"
#include "handler.h"

#define MAX_INSTALLER_HANDLER	64

/****************************************************************************/
extern int install_nothing(char *dev, int offs, ota_image *pimg);
extern int install_amlogic_nand_flash_image(char *dev, int offs, ota_image *pimg);
extern int install_nand_flash_image(char *dev, int dev_offs, ota_image *pimg);
extern int install_nor_flash_image(char *dev, int offs, ota_image *pimg);
extern int install_rockchip_nand_flash_image(char *dev, int offs, ota_image *pimg);


/****************************************************************************/
extern int install_nothing_wh(char *dev, ota_image *pimg, int offs, char *data, int length);
extern int install_amlogic_nand_flash_image_wh(char *dev, ota_image *pimg, int offs, char *data, int length);
extern int install_nand_flash_image_wh(char *dev, ota_image *pimg, int dev_offs, char *data, int length);
extern int install_nor_flash_image_wh(char *dev, ota_image *pimg, int offs, char *data, int length);
extern int install_rockchip_nand_flash_image_wh(char *dev, ota_image *pimg, int offs, char *data, int length);


/****************************************************************************/
extern int install_nothing_bk(ota_image *pimg, int bk_idx, int idx, int offset, int length, char *buffer);
extern int install_amlogic_nand_flash_image_bk(ota_image *pimg, int bk_idx, int idx, int offset, int length, char *buffer);
extern int install_nand_flash_image_bk(ota_image *pimg, int bk_idx, int idx, int offset, int length, char *buffer);
extern int install_nor_flash_image_bk(ota_image *pimg, int bk_idx, int idx, int offset, int length, char *buffer);
extern int install_rockchip_nand_flash_image_bk(ota_image *pimg, int bk_idx, int idx, int offset, int length, char *buffer);


static installer_handler supported_types[MAX_INSTALLER_HANDLER] = {
	{"dummy", install_nothing, install_nothing_wh, install_nothing_bk},
	{"amlogic-nand", install_amlogic_nand_flash_image, install_amlogic_nand_flash_image_wh, install_amlogic_nand_flash_image_bk},
	{"nand", install_nand_flash_image, install_nand_flash_image_wh, install_nand_flash_image_bk},
	{"nor", install_nor_flash_image, install_nor_flash_image_wh, install_nor_flash_image_bk},
	{"rockchip-nand", install_rockchip_nand_flash_image, install_rockchip_nand_flash_image_wh, install_rockchip_nand_flash_image_bk},
};

static unsigned int nr_installers = 5;

int register_handler(const char *desc, handler installer)
{
	if (nr_installers > MAX_INSTALLER_HANDLER - 1)
		return -1;

	supported_types[nr_installers].desc = (char*)calloc(1, 64);
	if (!supported_types[nr_installers].desc)
		return -1;
	strncpy(supported_types[nr_installers].desc, desc,
		      (size_t)sizeof(supported_types[nr_installers].desc));
	supported_types[nr_installers].installer = installer;
	nr_installers++;

	return 0;
}

installer_handler *find_handler(ota_image *pimg)
{
	unsigned int i;

	for (i = 0; i < nr_installers; i++) {
		if ((strlen(pimg->head.flash_type) ==
				strlen(supported_types[i].desc)) &&
				strncmp(pimg->head.flash_type,
				supported_types[i].desc, 64) == 0)
			break;
	}

	if (i >= nr_installers) {
		TRACE("no handler found for: %s",  pimg->head.flash_type);
		return NULL;
	}

	return &supported_types[i];
}

void print_registered_handlers(void)
{
	unsigned int i;

	if (!nr_installers)
		return;

	TRACE("Registered handlers:\n");

	for (i = 0; i < nr_installers; i++)
		TRACE("\t%s\n", supported_types[i].desc);
}
