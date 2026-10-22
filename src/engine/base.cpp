#include "base.h"

// Pass the necessary parameters through to the base class
RenderObject::RenderObject(GlobalContext &context) : Object(context) {
  object_uid = ctx.currentID;
  ctx.currentID++;
}

// Temporary fix to correct VTable errors
bool RenderObject::draw() { return false; }
