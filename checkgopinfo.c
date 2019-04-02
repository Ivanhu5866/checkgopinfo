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

        InitializeLib(image_handle, systab);

        status = uefi_call_wrapper(BS->LocateProtocol,
                                   3,
                                   &gop_guid,
                                   NULL,
                                   &gop);

	if (status == EFI_SUCCESS) {
        	Print(L"MaxMode %d\n", gop->Mode->MaxMode);
        	Print(L"Current Mode %d :\n", gop->Mode->Mode);
        	Print(L"    Version %d\n", gop->Mode->Info->Version);
        	Print(L"    HorizontalResolution %d\n", gop->Mode->Info->HorizontalResolution);
        	Print(L"    VerticalResolution %d\n", gop->Mode->Info->VerticalResolution);
        	Print(L"Framebuffer base 0x%llx\n", gop->Mode->FrameBufferBase);
        	Print(L"FrameBufferSize 0x%llx\n", gop->Mode->FrameBufferSize);
	}
	else
		Print(L"Cannot get Locate GOP.\n");
        return status;
}
