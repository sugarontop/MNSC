#pragma once
typedef VARIANT(*VARIANTFUNC)( VARIANT,  VARIANT,  VARIANT,  VARIANT);

void VarFunctionInit(std::map<wstring, VARIANTFUNC>& map);