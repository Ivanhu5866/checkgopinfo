/*
 *  Copyright(C) 2017 Canonical Ltd.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 *  USA.
 */


#include <efi.h>
#include <efilib.h>

EFI_STATUS
efi_main (EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *systab)
{
        EFI_STATUS status;
        EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
        EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
        EFI_LOADED_IMAGE *loaded_image = NULL;
	EFI_DEVICE_PATH_PROTOCOL *FilePath;
    	EFI_HANDLE load_image_handle = NULL;
	UINT32 i = 0;
	UINT32 maxmode = 0;
	UINTN size_info;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info = NULL;
	EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE querymode;
	EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE setmode;
	EFI_INPUT_KEY eik;
	UINT32 mode_set = 0;

        InitializeLib(image_handle, systab);

        status = uefi_call_wrapper(BS->LocateProtocol,
                                   3,
                                   &gop_guid,
                                   NULL,
                                   &gop);

	if (status == EFI_SUCCESS) {
		maxmode = gop->Mode->MaxMode;
        	Print(L"MaxMode %d\n", maxmode);
        	Print(L"Current Mode %d :\n", gop->Mode->Mode);
        	Print(L"    Version %d\n", gop->Mode->Info->Version);
        	Print(L"    HorizontalResolution %d\n", gop->Mode->Info->HorizontalResolution);
        	Print(L"    VerticalResolution %d\n", gop->Mode->Info->VerticalResolution);
        	Print(L"Framebuffer base 0x%llx\n", gop->Mode->FrameBufferBase);
        	Print(L"FrameBufferSize 0x%llx\n", gop->Mode->FrameBufferSize);
	}
	else {
		Print(L"Cannot get Locate GOP, status 0x%llx\n", status);
		return status;
	}

	querymode = (void *)(unsigned long)gop->QueryMode;
	for (i = 0; i < maxmode; i++) {
        	status = uefi_call_wrapper(querymode,
                                   	4,
                                   	gop,
                                  	i,
                                   	&size_info,
					&info);

		if (status == EFI_SUCCESS) {
			Print(L"Mode %d :\n", i);
			Print(L"    Version %d\n", info->Version);
			Print(L"    HorizontalResolution %d\n", info->HorizontalResolution);
			Print(L"    VerticalResolution %d\n", info->VerticalResolution);
		}
		else {
			Print(L"Cannot QueryMode, status 0x%llx\n", status);
			return status;
		}
	}
		
	Print(L"Pleas enter the mode you would like to set:\n");


	while (1) {
		WaitForSingleEvent(ST->ConIn->WaitForKey, 0);
		status = uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &eik);
		Print(L"ScanCode: %lxh  UnicodeChar: %lxh CallRtStatus: %lx\n",
		eik.ScanCode, eik.UnicodeChar, status);
		
		if (eik.UnicodeChar < 0x30 || eik.UnicodeChar > (0x30 + maxmode - 1)) {
			Print(L"Invalid mode number, please enter again:\n");
			continue;
		}
		mode_set = eik.UnicodeChar - 0x30;
		break;
	}

	setmode = (void *)(unsigned long)gop->SetMode;
	status = uefi_call_wrapper(setmode,
                           	2,
                           	gop,
                          	mode_set);

	if (status != EFI_SUCCESS) {
		Print(L"Cannot SetMode, status 0x%llx\n", status);
	}

	Print(L"Continue boot Ubuntu?(Y/N):\n");
	
	while (1) {
		WaitForSingleEvent(ST->ConIn->WaitForKey, 0);
		status = uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &eik);
		
		if (eik.UnicodeChar == 0x59 || eik.UnicodeChar == 0x79) {
			Print(L"Calling shimx64.efi:\n");
			status = uefi_call_wrapper(BS->HandleProtocol,
						3,
						image_handle, 
						&LoadedImageProtocol, 
						(void **) &loaded_image);
			if (EFI_ERROR(status)) {
				Print(L"HandleProtocol: %r\n", status);
				return status;
			}
			FilePath = FileDevicePath(loaded_image->DeviceHandle, L"efi\\ubuntu\\shimx64.efi");
			Print(L"file path         : %s\n", DevicePathToStr(FilePath));
			status = uefi_call_wrapper(BS->LoadImage,
						6,
						FALSE,
						image_handle,
						FilePath,
						NULL,
						0,
						&load_image_handle);
			if (EFI_ERROR(status)) {
				Print(L"Load Image Status = %x\n", status);
				return status;
			}
			status = uefi_call_wrapper(BS->StartImage,
						3,
						load_image_handle,
						NULL,
						NULL);
			if (!EFI_ERROR(status)) {
				Print(L"StartImage success\n");
			}
	
			break;
		} else if (eik.UnicodeChar == 0x4E || eik.UnicodeChar == 0x6E) {
			break;
		} else
			continue;
	}

        return status;
}
