#pragma once
#include "RBMath.h"

extern uint32 GFrameNumberRenderThread;

class RBSceneRenderer
{
public:

  virtual void render();

protected:

private:

};


class RBForwardSceneRenderer : public RBSceneRenderer
{
public:
  virtual void render() override;

};


class RBDeferredSceneRenderer : public RBSceneRenderer
{
public:
  virtual void render() override;

};