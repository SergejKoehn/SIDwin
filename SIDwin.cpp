#include "stdafx.h"

#include "SID/SID.h"

using namespace System;

int main(array<System::String ^> ^args)
{
  SID sid;
  sid.writeRegister(0,0xff);
  return 0;
}
