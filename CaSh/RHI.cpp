#include "RHIResources.h"

RBRHIResource* RBRHIResource::CurrentlyDeleting = nullptr;
RBThreadSafeQueue<RBRHIResource*> RBRHIResource::PendingDeletes;

//DO NOT USE THE STATIC FLINEARCOLORS TO INITIALIZE THIS STUFF.  
//Static init order is undefined and you will likely end up with bad values on some platforms.
const RBClearValueBinding RBClearValueBinding::None(EClearBinding::ENoneBound);
const RBClearValueBinding RBClearValueBinding::Black(RBColorf(0.0f, 0.0f, 0.0f, 1.0f));
const RBClearValueBinding RBClearValueBinding::White(RBColorf(1.0f, 1.0f, 1.0f, 1.0f));
const RBClearValueBinding RBClearValueBinding::Transparent(RBColorf(0.0f, 0.0f, 0.0f, 0.0f));
const RBClearValueBinding RBClearValueBinding::DepthOne(1.0f, 0);
const RBClearValueBinding RBClearValueBinding::DepthZero(0.0f, 0);
const RBClearValueBinding RBClearValueBinding::DepthNear((float)ERHIZBuffer::NearPlane, 0);
const RBClearValueBinding RBClearValueBinding::DepthFar((float)ERHIZBuffer::FarPlane, 0);