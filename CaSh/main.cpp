#include "App.h"
#include "TypeHash.h"
#include <iostream>

int main(int argc,char** argv)
{

  uint32 DirtyBits = 0x4c10;
  uint32 res = (DirtyBits)& (~DirtyBits+1);
  

  AppD3D11 app;
  //客户端区域尺寸（除菜单栏边界等的区域）
  app.init_windows("Alpha Window", 1920*0.8, 1080*0.8);
  app.run();
  //app.~AppD3D11();

  return 0;
}