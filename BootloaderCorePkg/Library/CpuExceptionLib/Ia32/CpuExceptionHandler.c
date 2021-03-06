/** @file
  IA32 CPU Exception Handler functons.

  Copyright (c) 2012 - 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <CpuExceptionCommon.h>

CONST UINT32 mErrorCodeFlag = 0x00027d00;

/**
  Common exception handler.

  It will print out the location where exception occured and then halt the system.
  This function will never return.

  @param[in] Stack          Current stack address pointer.
  @param[in] ExceptionType  Exception type code.
**/
VOID
EFIAPI
CommonExceptionHandler (
  IN UINT32        *Stack,
  IN UINT8          ExceptionType
  )
{
  UINT32  *Ptr;

  // Skip the ExceptionType on the stack
  Ptr = Stack + 1;

  // Skip the ErrorCode on the stack
  if (ExceptionType < CPU_EXCEPTION_NUM) {
    if (mErrorCodeFlag & (1 << ExceptionType)) {
      Ptr++;
    }
  }

  DEBUG ((DEBUG_ERROR, "Exception #%d from 0x%04X:0x%08X !!!\n", ExceptionType, Ptr[1], Ptr[0]));
  CpuHalt (NULL);
}

/**
  Update exception handler in IDT table .

  This function is used to update the IDT exception handler with current stage.

  @param[in]  IdtBase   If non-zero, it is the IDT base address.
                        if it is 0,  the IDT base will be retrieved from IDT base register.

**/
VOID
UpdateExceptionHandler (
  IN UINT32        IdtBase
)
{
  UINT64                     *IdtTable;
  IA32_IDT_GATE_DESCRIPTOR    IdtGateDescriptor;
  UINT32                      Index;
  UINT32                      Address;
  IA32_DESCRIPTOR             Idtr;
  EXCEPTION_HANDLER_TEMPLATE_MAP  TemplateMap;

  if (IdtBase == 0) {
    AsmReadIdtr (&Idtr);
    IdtTable = (UINT64 *)Idtr.Base;
  } else {
    IdtTable = (UINT64 *)IdtBase;
  }
  IdtGateDescriptor.Uint64          = 0;
  IdtGateDescriptor.Bits.Selector   = AsmReadCs ();
  IdtGateDescriptor.Bits.GateType   = IA32_IDT_GATE_TYPE_INTERRUPT_32;

  AsmGetTemplateAddressMap (&TemplateMap);
  for (Index = 0; Index < CPU_EXCEPTION_NUM; Index ++) {
    Address = TemplateMap.ExceptionStart + TemplateMap.ExceptionStubHeaderSize * Index;
    IdtGateDescriptor.Bits.OffsetHigh = (UINT16)(Address >> 16);
    IdtGateDescriptor.Bits.OffsetLow  = (UINT16)Address;
    IdtTable[Index] = IdtGateDescriptor.Uint64;
  }
}