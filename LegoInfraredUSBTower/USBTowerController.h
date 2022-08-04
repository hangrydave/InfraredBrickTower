#pragma once

#ifndef USBTOWERCONTROLLER_H
#define USBTOWERCONTROLLER_H

#include <Windows.h>
#include <winusb.h>
#include "LegoHeaders/VendReq.h"

class USBTowerController
{
public:
	USBTowerController(const WINUSB_INTERFACE_HANDLE* handle);

	BOOL BlinkLights();
	BOOL GetVersion(LTW_REQ_GET_VERSION_REPLY& reply);
private:
	const WINUSB_INTERFACE_HANDLE* handle;

	BOOL SendVendorRequest(
		BYTE request,
		BYTE parameter,
		BYTE value,
		WORD index,
		WORD replyLength,
		BYTE* replyBuffer);

	BOOL SendVendorRequest(
		BYTE request,
		BYTE parameter,
		BYTE value,
		WORD replyLength,
		BYTE* replyBuffer);

	BOOL SendVendorRequest(
		BYTE request,
		BYTE parameter,
		BYTE value);

	BOOL SendVendorRequest(
		BYTE request,
		WORD fullValue,
		WORD index,
		WORD replyLength,
		BYTE* replyBuffer);

	BOOL SendVendorRequest(
		BYTE request,
		WORD fullValue,
		WORD replyLength,
		BYTE* replyBuffer);

	/*BOOL SendVendorRequest(
		BYTE request,
		WORD replyLength,
		BYTE* replyBuffer);*/
};

#endif USBTOWERCONTROLLER_H
